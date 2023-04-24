#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

out vec4 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

const float TIME_OFFSET = 1000.f; // when does the game start
const float SUN_VELOCITY = 1/ 1000.f;


// The palette is a color spectrum with some bars with different colors
// The color is decided by linearly interpolating two adjacent bars

// Daytime palette
const vec3 daytime[5] = vec3[](vec3(185, 220, 230) / 255.0,
                               vec3(170, 215, 245) / 255.0,
                               vec3(150, 205, 240) / 255.0,
                               vec3(100, 180, 235) / 255.0,
                               vec3( 80, 170, 230) / 255.0);

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                              vec3(254, 192,  81) / 255.0,
                              vec3(255, 137, 103) / 255.0,
                              vec3(253,  96,  81) / 255.0,
                              vec3( 57,  32,  51) / 255.0);

// Dusk palette
const vec3 dusk[5] = vec3[](vec3(114, 66, 114) / 255.0,
                            vec3( 71, 47, 95) / 255.0,
                            vec3( 47, 23, 95) / 255.0,
                            vec3( 28, 4,  76) / 255.0,
                            vec3(  0, 4,  52) / 255.0);

// Sun palette
const vec3 sun[3] = vec3[](vec3(255, 255, 190 ) / 255.0, // daytime
                           vec3(255, 140, 100) / 255.0, // sun rise
                           vec3(100,  70, 130) / 255.0); // sunset


// Map 3d point from a unit sphere to uv coordinate
vec2 sphereToUV(vec3 p) {
    // polar coordinates
    float phi = atan(p.z, p.x); // 0 <= phi < 2*Pi
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}


// Map 2d uv coordinate to sunset palette
vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

// Map 2d uv coordinate to dusk palette
vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.5) {
        return dusk[0];
    }
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    return dusk[4];
}

// Map 2d uv coordinate to daytime palette
vec3 uvToDaytime(vec2 uv) {
    if(uv.y < 0.5) {
        return daytime[0];
    }
    else if(uv.y < 0.55) {
        return mix(daytime[0], daytime[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(daytime[1], daytime[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(daytime[2], daytime[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(daytime[3], daytime[4], (uv.y - 0.65) / 0.1);
    }
    return daytime[4];
}

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.02 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.02 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}



void main()
{
    // From pixel space to world space
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    // ray direction to the camera
    vec3 rayDir = normalize(p.xyz - u_Eye);

    // get the uv
    vec2 uv = sphereToUV(rayDir);

    // Generate a disturbance based on time to simulate an effect of moving clouds
    vec2 offset = vec2(0.0);
    // Get a noise value in the range [-1, 1]
    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);


    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 daytimeColor = uvToDaytime(uv + offset * 0.1);
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);
    vec3 duskColor = uvToDusk(uv /* + offset * 0.1 */); // do not paint cloud at night
    vec3 skyColor;
    vec3 sunColor;

    // time loop for a day
    // -1 < t < -0.3 night
    // -0.3 < t < 0.3 sunset / sunrise
    // 0.3 < t < 1 daytime
    float time = sin(u_Time * SUN_VELOCITY + TIME_OFFSET); // negative for night and positive for daytime

    if (time > 0.3) { // daytime
        skyColor = daytimeColor;
        sunColor = sun[0];
    }
    else if (time < -0.3) { // night
        skyColor = duskColor;
        sunColor = vec3(0); // no sun
    }
    else {
        skyColor = mix(duskColor, daytimeColor, (time + 0.3) / (0.3 * 2.f));
        if (time > 0) { // daytime
            sunColor = mix(sun[1], sun[0], smoothstep(0.f, 1.f, abs(time) / 0.3));
        }
        else { // night
            sunColor = mix(sun[1], sun[2], smoothstep(0.f, 1.f, abs(time) / 0.3));
        }
    }

    // paint some stars
    if (time < -0.3) { // night
        if (rayDir.y > 0.2 && random1(rayDir) * 1000 < 1 && random1(rayDir) * 1000 > 0.9){
            skyColor = vec3(1);
        }
    }

    // Add a glowing sun in the sky
    vec3 sunDir = normalize(vec3(cos(u_Time * SUN_VELOCITY + TIME_OFFSET), sin(u_Time * SUN_VELOCITY + TIME_OFFSET), 0.f)); // move the sun
    float sunSize = 30;
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun
    if(angle < sunSize) {
        // Full center of sun
        if(angle < 7.5) {
            skyColor = sunColor;
        }
        // Corona of sun, mix with sky color
        else {
            skyColor = mix(sunColor, skyColor, (angle - 7.5) / 22.5);

        }
    }
    // Otherwise our ray is looking into just the sky
    else {
        if (-0.3 < time && time < 0.3) {
            float SUNSET_THRESHOLD = 0.75;
            float DUSK_THRESHOLD = -0.1;

            float raySunDot = dot(rayDir, sunDir);
            if(raySunDot > SUNSET_THRESHOLD) {
                // Do nothing, sky is already correct color
            }
            // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and dusk color
            else if(raySunDot > DUSK_THRESHOLD) {
                float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
                skyColor = mix(skyColor, duskColor, t);
            }
            // Any dot product <= -0.1 are pure dusk color
            else {
                skyColor = duskColor;
            }

            vec3 temp;
            if (time < 0) {
                temp = duskColor;
            } else {
                temp = daytimeColor;
            }

            // smooth the sky color transfer
            skyColor = mix(temp, skyColor, smoothstep(0.f, 1.f, 1.f - abs(time) / 0.3));
        }
    }



    outColor = vec4(skyColor, 1); // set alpha channel
}
