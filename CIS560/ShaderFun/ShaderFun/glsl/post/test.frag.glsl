#version 330
// noOp.vert.glsl:
// A fragment shader used for post-processing that simply reads the
// image produced in the first render pass by the surface shader
// and outputs it to the frame buffer

uniform ivec2 u_Dimensions;
uniform int u_Time;

uniform int u_GridSize;


in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;


//#define WARP_FBM
//#define WARP_WORLEY
//#define WARP_PERLIN

#define FBM
//#define WORLEY
//#define PERLIN
//#define P_BASIC
//#define P_SUMMED


//#define SIMPLEX
//#define S_BASIC
//#define S_SUMMED

//#define TEST

#define WARP_FREQUENCY 8.0
#define WARP_MAGNITUDE 2.0
#define NOISE_FREQUENCY 4.0
#define FBM_OCTAVES 8



float random1(vec2 p) {
    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);
}

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float mySmootherStep(float a, float b, float t) {
    t = t*t*t*(t*(t*6.0 - 15.0) + 10.0);
    return mix(a, b, t);
}

vec2 rotate(vec2 p, float deg) {
    float rad = deg * 3.14159 / 180.0;
    return vec2(cos(rad) * p.x - sin(rad) * p.y,
                sin(rad) * p.x + cos(rad) * p.y);
}

float bilerpNoise(vec2 uv) {
    vec2 uvFract = fract(uv);
    float ll = random1(floor(uv));
    float lr = random1(floor(uv) + vec2(1,0));
    float ul = random1(floor(uv) + vec2(0,1));
    float ur = random1(floor(uv) + vec2(1,1));

    float lerpXL = mySmootherStep(ll, lr, uvFract.x);
    float lerpXU = mySmootherStep(ul, ur, uvFract.x);

    return mySmootherStep(lerpXL, lerpXU, uvFract.y);
}


float fbm(vec2 uv) {
//    uv = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * uv);
    float persistence = 0.5;
    float freq = 1.0;
    float sum = 0.0;
    for(int i = 0; i < FBM_OCTAVES; i++) {
        float amp = pow(persistence, i);
        sum += bilerpNoise(uv * freq) * amp;
        freq *= 2.0;
    }
    return sum;
}


float WorleyNoise(vec2 uv) {
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
//            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float surflet(vec2 P, vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);

    // Get the random vector for the grid point
    vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}


vec2 PixelToGrid(vec2 pixel, float size) {
    vec2 uv = pixel.xy / u_Dimensions.xy;
    uv.x = uv.x * float(u_Dimensions.x) / float(u_Dimensions.y);
    uv *= size;
    return uv;
}


float PerlinNoise(vec2 uv)
{

    vec2 uvXLYL = floor(uv);
    vec2 uvXHYL = uvXLYL + vec2(1,0);
    vec2 uvXHYH = uvXLYL + vec2(1,1);
    vec2 uvXLYH = uvXLYL + vec2(0,1);

    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}

vec2 NoiseVectorPerlin(vec2 uv) {
    float x = PerlinNoise(uv);
    float y = PerlinNoise(rotate(uv, 60.0));
    return vec2(x, y);
}

vec2 NoiseVectorWorley(vec2 uv) {
    float x = WorleyNoise(uv) * 2.0 - 1.0;
    float y = WorleyNoise(rotate(uv, 60.0)) * 2.0 - 1.0;
    return vec2(x, y);
}

vec2 NoiseVectorFBM(vec2 uv) {
    float x = fbm(uv) * 2.0 - 1.0;
    float y = fbm(rotate(uv, 60.0)) * 2.0 - 1.0;
    return vec2(x, y);
}



vec2 hash( vec2 p ) // replace this by something better
{
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(cos(p)*53758.5453123);
}

float SimplexNoise( vec2 p )
{
    float K1 = 0.366025404; // (sqrt(3)-1)/2;
    float K2 = 0.211324865; // (3-sqrt(3))/6;

    vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x);
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
    vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
    vec3  n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
    return dot( n, vec3(70.0) );
}

void main()
{
    vec2 uv = gl_FragCoord.xy / 1000;

    vec2 warp = vec2(0);
    #ifdef WARP_FBM
    warp = NoiseVectorFBM(uv * WARP_FREQUENCY) * WARP_MAGNITUDE;
    #endif
    #ifdef WARP_WORLEY
    warp = NoiseVectorWorley(uv * WARP_FREQUENCY) * WARP_MAGNITUDE;
    #endif
    #ifdef WARP_PERLIN
    warp = NoiseVectorPerlin(uv * WARP_FREQUENCY) * WARP_MAGNITUDE;
    #endif

    float h = 0;
    #ifdef FBM
    h = fbm(uv * NOISE_FREQUENCY + warp);
    #endif
    #ifdef WORLEY
    h = WorleyNoise(uv * NOISE_FREQUENCY + warp);
    #endif


    #ifdef PERLIN

    #ifdef P_BASIC
    vec2 coord = PixelToGrid(gl_FragCoord.xy, 8.0);
    float perlin = PerlinNoise(coord);
    h = (perlin + 1) * 0.5;
    #endif

    #ifdef PSUMMED
    float summedNoise = 0.0;
    float amplitude = 0.5;
    for (int i = 2; i <= 32; i*=2) {
        vec2 coord = PixelToGrid(gl_FragCoord.xy, float(i));
        coord = vec2(cos(3.14159/3.0 * i) * coord.x - sin(3.14159/3.0 * i) * coord.y, sin(3.14159/3.0 * i) * coord.x + cos(3.14159/3.0 * i) * coord.y);

        float perlin = 1 - abs(PerlinNoise(coord));
        perlin = smoothstep(0.6, 1, perlin);
        summedNoise += perlin * amplitude;
        amplitude *= 0.5;
    }
    h = 1 - summedNoise * summedNoise;
    #endif


    #endif


    #ifdef SIMPLEX

    #ifdef S_BASIC
    vec2 coord = uv * NOISE_FREQUENCY + warp;
    h = SimplexNoise(coord);
    #endif

    #ifdef S_SUMMED
    mat2 m1 = mat2( 1.3, 2.2, -1.4, 1.2 );
    mat2 m2 = mat2( 2.3, 1.2, -0.7, 1.5 );
    h  = 0.5000*SimplexNoise( coord );
    coord = m1*coord;
    h += 0.2500*SimplexNoise( coord );
    coord = m1*coord;
    h += 0.1250*SimplexNoise( coord );
    coord = m2*coord;
    h += 0.0625*SimplexNoise( coord );
    coord = m2*coord;


    #endif
    #endif

#ifdef TEST
#endif

    color = vec3(1-h);

}
