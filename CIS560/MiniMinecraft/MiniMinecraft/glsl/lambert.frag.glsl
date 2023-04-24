
#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_texture;
uniform sampler2D u_normTexture;
uniform sampler2D u_textureBetter;
uniform int u_Time;
uniform vec4 u_CamPos;


uniform vec3 u_Eye;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec2 fs_UV;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

const float SUN_VELOCITY = 1 / 1000.f;
const float TIME_OFFSET = 1000.f; // when does the game start

// Sun palette
const vec3 sun[3] = vec3[](vec3(255, 255, 245) / 255.0,
                           vec3(255, 140, 100) / 255.0,
                           vec3(100,  70, 130) / 255.0);


const vec4 fogColor = vec4(0.8, 0.9, 1, 1);

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}


float vec2Fbm(vec2 uv){
    float amp = 0.5;
    float freq = 8.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(vec3(uv, 0) * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

float waterHeight(vec2 uv){
    return vec2Fbm((uv + vec2(u_Time)) * 0.01);
}

vec3 waterNormalDisplacement(vec2 uv){
    vec2 dx = vec2(0.1, 0);
    vec2 dy = vec2(0, 0.1);
    vec2 grad = vec2(waterHeight(uv + dx) - waterHeight(uv - dx),
                     waterHeight(uv + dy) - waterHeight(uv - dy));
    grad = grad * 20.f;
    float z = sqrt(1.f - grad.x * grad.x - grad.y * grad.y);
    return vec3(grad.xy, z);
}

void coordinateSystem(in vec3 nor, out vec3 tan, out vec3 bit){
    if (abs(nor.x) > abs(nor.y)){
        tan = vec3(-nor.z, 0, nor.x);
    }else{
        tan = vec3(0, nor.z, -nor.y);
    }
    bit = cross(nor, tan);
}
//https://vicrucann.github.io/tutorials/osg-shader-fog/
//float getFogFactor(float d){
//    float fogMax = 20.f;
//    float fogMin = 10.f;

//    if (d >= fogMax)
//        return 1;
//    if (d <= fogMin)
//        return 0;
//    return 1 - (fogMax - d) / (fogMax - fogMin);
//}


void main()
{
    // time loop for a day
    // -1 < t < -0.3 night
    // -0.3 < t < 0.3 sunset / sunrise
    // 0.3 < t < 1 daytime

    float time = sin(u_Time * SUN_VELOCITY + TIME_OFFSET); // negative for night and positive for daytime


    // Direction of sun light
    vec3 sunDir = normalize(vec3(cos(u_Time * SUN_VELOCITY + TIME_OFFSET), sin(u_Time * SUN_VELOCITY + TIME_OFFSET), 0.f));

    if (fs_Nor.w != 0.5 && !(fs_UV.x >= 14.f * 0.0625 && fs_UV.x < 15.f * 0.0625
                             && fs_UV.y >= 2.f * 0.0625 && fs_UV.y < 3.f * 0.0625)){
        time = sin(TIME_OFFSET);
        sunDir = normalize(vec3(cos(0 * SUN_VELOCITY + TIME_OFFSET), sin(0 * SUN_VELOCITY + TIME_OFFSET), 0.f));
    }
    // Color of sun light
    vec3 sunColor;
    if (time > 0.3) {
        sunColor = sun[0];
    }
    else if (time < -0.3) {
        sunColor = sun[2];
    }
    else {
        if (time > 0) {
            float smooth_t = smoothstep(0.f, 1.f, abs(time) / 0.3);
            sunColor = mix(sun[1], sun[0], smooth_t);
        }
        else {
            float smooth_t = smoothstep(0.f, 1.f, abs(time) / 0.3);
            sunColor = mix(sun[1], sun[2], smooth_t);
        }
    }

    // Material base color (before shading)
    vec4 diffuseColor = vec4(0);
    vec4 Nor = fs_Nor;

    float diffuseTerm = 0;
    //fs_Col.z == 0.2, use betterTexture, fs_Nor.w == 0.5 no
    //animation
    if (fs_Nor.w == 0.5){
        if (fs_Col.z == 0.2){
            diffuseColor = texture(u_textureBetter, fs_UV);
            Nor = texture(u_normTexture, vec2(3 * 0.0625, 15 * 0.0625));
        }
        else{
            diffuseColor = texture(u_texture, fs_UV);
            diffuseColor.xyz = diffuseColor.xyz * (0.5 * fbm(fs_Pos.xyz) + 0.5);
            Nor = texture(u_normTexture, fs_UV);
        }

//        vec4 my_Nor = texture(u_normTexture, fs_UV);
        diffuseTerm = dot(normalize(vec3(fs_Nor) + vec3(Nor)), sunDir);

    } else{
        if (fs_Col.z == 0.2){

            diffuseColor = texture(u_textureBetter, vec2(fs_UV.x + (u_Time % 10) * 0.01 * 0.0625, fs_UV.y));
            Nor = texture(u_normTexture, vec2(3 * 0.0625, 15 * 0.0625));
//            diffuseColor.xyz = diffuseColor.xyz * (0.5 * fbm(fs_Pos.xyz) + 0.5);
        }
        else{
            if (fs_UV.x >= 14.f * 0.0625 && fs_UV.x < 15.f * 0.0625
                    && fs_UV.y >= 2.f * 0.0625 && fs_UV.y < 3.f * 0.0625){
                vec3 tan, bit;
                coordinateSystem(normalize(fs_Nor.xyz), tan, bit);
                mat3 tanWorld = mat3(tan, bit, normalize(fs_Nor.xyz));
                vec3 shadingNormal = waterNormalDisplacement(ivec2(fs_Pos.xz * 4));
                shadingNormal = tanWorld * shadingNormal;
                Nor = vec4(shadingNormal.x, shadingNormal.y, shadingNormal.z,  0.f);
//                diffuseColor = texture(u_texture, vec2(floor(fs_UV.x * 4.f) / 4.f, floor(fs_UV.y * 4.f) / 4.f));
//                diffuseColor = vec4(0);
                diffuseColor = texture(u_texture, vec2(fs_UV.x + (u_Time % 20) * 0.05 * 0.0625, fs_UV.y));
            }else{
                diffuseColor = texture(u_texture, vec2(fs_UV.x + (u_Time % 20) * 0.05 * 0.0625, fs_UV.y));
                diffuseColor.rgb = diffuseColor.rgb * (0.5 * fbm(fs_Pos.xyz) + 0.5);
                Nor = texture(u_normTexture, vec2(3 * 0.0625, 15 * 0.0625));
            }
//            diffuseColor.xyz = diffuseColor.xyz * (0.5 * fbm(fs_Pos.xyz) + 0.5);
        }


        diffuseTerm = dot(normalize(vec3(Nor)), sunDir);
    }
    if (diffuseColor.a < 0.1){
        discard;
    }
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    // Blinn-Phong
    float shininess = 50;
    // Calculate the specularTerm
    vec3 view_dir = u_Eye - vec3(fs_Pos);
    vec3 H = normalize((normalize(view_dir) + normalize(sunDir)));
    float specularTerm = pow(dot(normalize(vec3(Nor)), H), shininess);
    specularTerm = clamp(specularTerm, 0.f, 1.f);

    if (time < -0.3) {
        diffuseTerm = 0.f;
        specularTerm = 0.f;
    }
    else if (time >= -0.3 && time < 0.3) {
        diffuseTerm = mix(0.f, diffuseTerm, (time + 0.3) / (2.f * 0.3));
        specularTerm = mix(0.f, specularTerm, (time + 0.3) / (2.f * 0.3));
    }



//    diffuseColor.rgb = diffuseColor.rgb * (0.5 * fbm(fs_Pos.xyz) + 0.5); // make each block different

    float ambientTerm = 0.2;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                                   //to simulate ambient lighting. This ensures that faces that are not
                                                                   //lit by our point light are not completely black.

    // Compute final shaded color
    float d = distance(u_CamPos, fs_Pos);
    float fogAlpha = smoothstep(30.f, 70.f, d);

//    if (fs_Col.z == 0.2){
       out_Col = mix(vec4(diffuseColor.rgb * lightIntensity * sunColor + vec3(specularTerm), diffuseColor.a + specularTerm), vec4(sunColor, 1.f), fogAlpha * 0.6);
//    }else{
//        out_Col = mix(diffuseColor, fogColor, fogAlpha * 0.7);
//    }

}
