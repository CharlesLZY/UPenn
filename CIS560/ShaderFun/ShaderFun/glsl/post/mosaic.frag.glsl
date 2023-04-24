#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

// reference from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
// VoroNoise
// Also provided in the slides 'Noise Function'
vec2 hash(vec2 p){ // p is used as the random seed
    p = vec2( dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3)));
    return fract(sin(p)*43758.5453); // fract(x) = x - floor(x)
}


const int size = 15;

vec2 closestCenter(int x, int y) {
    int i = x / size;
    int j = y / size;

    vec2 p = vec2(x,y);

    vec2 center = hash(vec2(i,j))*size + vec2(i*size, j*size);
    // center of neighbour blocks
    vec2 lu_center = hash(vec2(i-1,j-1))*size + vec2(i*size - size, j*size - size);
    vec2 u_center  = hash(vec2(i  ,j-1))*size + vec2(i*size, j*size - size);
    vec2 ru_center = hash(vec2(i+1,j-1))*size + vec2(i*size + size, j*size - size);

    vec2 l_center = hash(vec2(i-1,j))*size + vec2(i*size - size, j*size);
    vec2 r_center = hash(vec2(i+1,j))*size + vec2(i*size + size, j*size);

    vec2 ld_center = hash(vec2(i-1,j+1))*size + vec2(i*size - size, j*size + size);
    vec2 d_center  = hash(vec2(i  ,j+1))*size + vec2(i*size, j*size + size);
    vec2 rd_center = hash(vec2(i+1,j+1))*size + vec2(i*size + size, j*size + size);

    vec2 centers[9] = vec2[9](center, lu_center, u_center, ru_center, l_center, r_center, ld_center, d_center, rd_center);

    vec2 closest = center;
    float minDist = distance(p, center);

    for (int n=1; n<9; n++) {
        if (distance(p, centers[n]) < minDist) {
            closest = centers[n];
            minDist = distance(p, centers[n]);
        }
    }

    return closest;


}

void main()
{


    int x = int(fs_UV.x * u_Dimensions.x);
    int y = int(fs_UV.y * u_Dimensions.y);


    vec2 center = closestCenter(x, y);


    color = texture(u_RenderedTexture, center/u_Dimensions).rgb;

    color += clamp(vec3(distance(center, vec2(x,y))) / (10*size), 0.f, 0.5f);


}
