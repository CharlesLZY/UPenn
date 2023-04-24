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



//#define TEST

#define WARP_FREQUENCY 8.0
#define WARP_MAGNITUDE 2.0
#define NOISE_FREQUENCY 4.0
#define SCALE 1000

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
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 1.0;

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor); // 0 to 1 range

            point = 0.5 + 0.5 * sin(52768 * 0.01 + 6.2831 * point); // 0 to 1 range

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

vec2 NoiseVectorPerlin(vec2 uv) {
    float x = PerlinNoise(uv);
    float y = PerlinNoise(rotate(uv, 60.0));
    return vec2(x, y);
}




void main()
{
    //// gl_FragCoord.xy : 1440 x 1080
    vec2 uv = gl_FragCoord.xy / SCALE;

    vec2 warp = vec2(0);
    warp = NoiseVectorPerlin(uv * WARP_FREQUENCY) * WARP_MAGNITUDE;

    float h = 0;

    h = WorleyNoise(uv * NOISE_FREQUENCY + warp);




#ifdef TEST

#endif

    color = vec3(1-h);

}
