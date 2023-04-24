#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

// reference from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
// Also provided in the slides 'Noise Function'
vec2 hash(vec2 p){ // p is used as the random seed
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
    return fract(sin(p)*43758.5453); // fract(x) = x - floor(x)
}

const float PI = 3.14159265;
const int size = 180;

vec2 closestCenter(int x, int y) {
    int i = x / size;
    int j = y / size;
    vec2 rotate = vec2(10*sin(fs_UV.y*15 + (u_Time / 3) % 80 * PI / 40 ) +  10*cos(-fs_UV.y*5 + (u_Time / 3) % 80 * PI / 40 ),
                       10*cos(-fs_UV.x*10 + (u_Time / 5) % 80 * PI / 40 ) + 10*sin(-fs_UV.x*10 + (u_Time / 3) % 80 * PI / 40 ));

    vec2 p = vec2(x,y);

    vec2 center = hash(vec2(i,j))*size + vec2(i*size, j*size) + rotate;
    // center of neighbour blocks
    vec2 lu_center = hash(vec2(i-1,j-1))*size + vec2(i*size - size, j*size - size) + rotate;
    vec2 u_center  = hash(vec2(i  ,j-1))*size + vec2(i*size, j*size - size) + rotate;
    vec2 ru_center = hash(vec2(i+1,j-1))*size + vec2(i*size + size, j*size - size) + rotate;

    vec2 l_center = hash(vec2(i-1,j))*size + vec2(i*size - size, j*size) + rotate;
    vec2 r_center = hash(vec2(i+1,j))*size + vec2(i*size + size, j*size) + rotate;

    vec2 ld_center = hash(vec2(i-1,j+1))*size + vec2(i*size - size, j*size + size) + rotate;
    vec2 d_center  = hash(vec2(i  ,j+1))*size + vec2(i*size, j*size + size) + rotate;
    vec2 rd_center = hash(vec2(i+1,j+1))*size + vec2(i*size + size, j*size + size) + rotate;

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

vec2 closestRealCenter(int x, int y) {
    int i = x / size;
    int j = y / size;
    vec2 rotate = vec2(0);

    vec2 p = vec2(x,y);

    vec2 center = hash(vec2(i,j))*size + vec2(i*size, j*size) + rotate;
    // center of neighbour blocks
    vec2 lu_center = hash(vec2(i-1,j-1))*size + vec2(i*size - size, j*size - size) + rotate;
    vec2 u_center  = hash(vec2(i  ,j-1))*size + vec2(i*size, j*size - size) + rotate;
    vec2 ru_center = hash(vec2(i+1,j-1))*size + vec2(i*size + size, j*size - size) + rotate;

    vec2 l_center = hash(vec2(i-1,j))*size + vec2(i*size - size, j*size) + rotate;
    vec2 r_center = hash(vec2(i+1,j))*size + vec2(i*size + size, j*size) + rotate;

    vec2 ld_center = hash(vec2(i-1,j+1))*size + vec2(i*size - size, j*size + size) + rotate;
    vec2 d_center  = hash(vec2(i  ,j+1))*size + vec2(i*size, j*size + size) + rotate;
    vec2 rd_center = hash(vec2(i+1,j+1))*size + vec2(i*size + size, j*size + size) + rotate;

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
    vec2 centerUV = center / u_Dimensions;

    vec2 uv = fs_UV - centerUV;

    float angle = atan(uv.y, uv.x) * 0.5 + acos(uv.y) * 2 + asin(uv.x) * 1.5 - uv.y * 5 + uv.x * 10 + 2 * PI;

    color = texture(u_RenderedTexture, vec2(sin(angle), cos(angle))/60 + fs_UV).rgb;


    float dist = distance(center+vec2(0,-80.f), vec2(x,y)) / (2*size);


    float r = 1.2 - 0.3 * pow(dist, 2);


    color *= r;

    color = clamp(vec3(1.2*color-dist), 0.f, 1.f);



}
