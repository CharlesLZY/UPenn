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


//#define RIDGE
//#define MOUNTAIN
#define PLAIN

//#define TEST


float random1(vec2 p) {
    return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);
}

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

vec2 rotate(vec2 p, float deg) {
    float rad = deg * 3.14159 / 180.0;
    return vec2(cos(rad) * p.x - sin(rad) * p.y,
                sin(rad) * p.x + cos(rad) * p.y);
}

float WorleyNoise(vec2 uv) {
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0;
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = random2(uvInt + neighbor);
            point = 0.5 + 0.5 * sin(52768 * 0.01 + 6.2831 * point);
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float surflet(vec2 P, vec2 gridPoint) {
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.0) + 15 * pow(distX, 4.0) - 10 * pow(distX, 3.0);
    float tY = 1 - 6 * pow(distY, 5.0) + 15 * pow(distY, 4.0) - 10 * pow(distY, 3.0);
    vec2 gradient = random2(gridPoint);
    vec2 diff = P - gridPoint;
    float height = dot(diff, gradient);
    return height * tX * tY;
}

float PerlinNoise(vec2 uv)
{
    vec2 uvXLYL = floor(uv);
    vec2 uvXHYL = uvXLYL + vec2(1,0);
    vec2 uvXHYH = uvXLYL + vec2(1,1);
    vec2 uvXLYH = uvXLYL + vec2(0,1);
    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
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

float bilerpNoise(vec2 uv) {
    vec2 uvFract = fract(uv);
    float ll = random1(floor(uv));
    float lr = random1(floor(uv) + vec2(1,0));
    float ul = random1(floor(uv) + vec2(0,1));
    float ur = random1(floor(uv) + vec2(1,1));
    float lerpXL = mix(ll, lr, uvFract.x);
    float lerpXU = mix(ul, ur, uvFract.x);
    return mix(lerpXL, lerpXU, uvFract.y);
}

float fbm(vec2 uv) {
    int octave = 8;
    float persistence = 0.5;
    float freq = 1.0;
    float sum = 0.0;
    for(int i = 0; i < octave; i++) {
        float amp = pow(persistence, i);
        sum += bilerpNoise(uv * freq) * amp;
        freq *= 2.0;
    }
    return sum;
}

float ridge(vec2 uv) {
    float WARP_FREQUENCY = 8.0;
    float WARP_MAGNITUDE = 2.0;
    float NOISE_FREQUENCY = 4.0;
    float SCALE = 1000.0;
    uv = uv / SCALE;
    float nx = PerlinNoise(uv * WARP_FREQUENCY);
    float ny = PerlinNoise(rotate(uv * WARP_FREQUENCY, 60.0));
    vec2 warp = vec2(nx, ny) * WARP_MAGNITUDE;
    uv = uv * NOISE_FREQUENCY + warp;
    // Worley Noise
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0;
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = random2(uvInt + neighbor);
            point = 0.5 + 0.5 * sin(52768 * 0.01 + 6.2831 * point);
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    float res = minDist < 0.5 ? minDist * minDist + 0.25 : minDist;
    res = smoothstep(0.1, 1, minDist);
    return res * 0.5 + 0.5;
}

float mountain(vec2 uv) {
    float SCALE = 250.0;
    uv = uv / SCALE;
    vec2 coord = uv;
    float h = SimplexNoise(coord);
    mat2 m = mat2( 1.3, 2.2, -1.4, 1.2 );
    h  = 0.5000*SimplexNoise( coord );
    coord = m*coord;
    h += 0.2500*SimplexNoise( coord );
    return h;
}

float plain(vec2 uv) {
    float SCALE = 250.0;
    uv = uv / SCALE;
    vec2 coord = uv;
    return 1 - fbm(uv);
}


void main()
{
    //// gl_FragCoord.xy : 1440 x 1080
    vec2 uv = gl_FragCoord.xy;
    float h = 0;

    #ifdef RIDGE
    h = ridge(uv);
    #endif

    #ifdef MOUNTAIN
    h = mountain(uv);
    #endif

    #ifdef PLAIN
    h = plain(uv);
    #endif

    #ifdef TEST

    #endif

    color = vec3(h);

}
