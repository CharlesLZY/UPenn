#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;
out vec3 color;

uniform sampler2D u_RenderedTexture;

const vec3 a = vec3(0.9, 0.9, 0.9);
const vec3 b = vec3(0.5, 0.5, 0.5);
const vec3 c = vec3(1.0, 1.0, 1.0);
const vec3 d = vec3(0.0, 0.1, 0.2);

const float PI = 3.14159265;

vec2 random2(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3))))*43758.5453);
}

vec3 Gradient(float t)
{
    return a + b * cos(6.2831 * (c * t + d));
}

vec2 smoothF(vec2 uv)
{
    return uv*uv*(3.-2.*uv);
}

float noise(in vec2 uv)
{
    const float k = 257.;
    vec4 l = vec4(floor(uv), fract(uv));
    float u = l.x + l.y * k;
    vec4 v = vec4(u, u+1, u+k, u+k+1.);
    v = fract(fract(1.23456789*v)*v/.987654321);
    l.zw = smoothF(l.zw);
    l.x = mix(v.x, v.y, v.z);
    l.y = mix(v.z, v.w, l.z);
    return mix(l.x, l.y, l.w);
}

float fbm(const in vec2 uv)
{
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;
    float freq = 2.f;
    float amp = 0.5f;
    for (int i=1; i<=octaves; i++){
        freq *= 2.f;
        amp *= persistence;
        total += noise(uv) * amp;
    }
    return total;
}




float WorleyNoise(vec2 uv)
{
    uv *= 7; // now the space is 10x10 instead
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0;

    // search all neighboring cells and this cell for their point
    for (int y = -1; y <= 1; ++y)
    {
        for(int x = -1; x <= 1; ++x)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
             point = 0.5 + 0.5 * sin(u_Time * 0.004 + 6.2831 * point); // 0 to 1

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float WorleyNoiseCos(vec2 uv)
{
    float minDist = WorleyNoise(uv);
    return cos(minDist * PI * 0.5);
}

vec2 PixelToGrid(vec2 pixel, float gridSize)
{
    vec2 uv = pixel.xy / u_Dimensions.xy;
    // Acount for aspect ratio
    uv.x = uv.x * float(u_Dimensions.x) / float(u_Dimensions.y);
    // Determine number of cells (NxN)
    uv *= gridSize;

    return uv;
}

vec3 rgb2HSV(vec3 inValue)
{
    vec3 outValue;
    float minValue, max, delta;

    minValue = inValue.r < inValue.g ? inValue.r : inValue.g;
    minValue = minValue  < inValue.b ? minValue  : inValue.b;

    max = inValue.r > inValue.g ? inValue.r : inValue.g;
    max = max > inValue.b ? max : inValue.b;

    outValue.z = max;
    delta = max - minValue;
    if (delta < 0.00001)
    {
        outValue.y = 0;
        outValue.x = 0; // undefinValueed, maybe nan?
        return outValue;
    }
    if (max > 0.0) { // NOTE: if Max == 0, this divide would cause a crash
        outValue.y = (delta / max);
    } else{
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefinValueed
        outValue.y = 0.0;
        outValue.x = 0.0;
        return outValue;
    }
    if (inValue.r >= max)
        outValue.x = (inValue.g - inValue.b) / delta;
    else
        if (inValue.g >= max)
            outValue.x = 2.0 + (inValue.b - inValue.r) / delta;
        else
            outValue.x = 4.0 + (inValue.r - inValue.g) / delta;

    outValue.x *= 60.0;

    if (outValue.x < 0.0)
        outValue.x += 360.0;

    return outValue;
}

vec3 hsv2rgb(vec3 inValue)
{
    float hh, p, q, t, ff;
    int i;
    vec3 outValue;

    if (inValue.y <= 0.0) { // < is bogus, just shuts up warninValuegs
        outValue.r = inValue.z;
        outValue.g = inValue.z;
        outValue.b = inValue.z;
        return outValue;
    }
    hh = inValue.x;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = int(hh);
    ff = hh - i;
    p = inValue.z * (1.0 - inValue.y);
    q = inValue.z * (1.0 - (inValue.y * ff));
    t = inValue.z * (1.0 - (inValue.y * (1.0 - ff)));

    if (i == 0) {
        outValue.r = inValue.z;
        outValue.g = t;
        outValue.b = p;
    }
    else if (i == 1) {
        outValue.r = q;
        outValue.g = inValue.z;
        outValue.b = p;
    }
    else if (i == 2) {
        outValue.r = p;
        outValue.g = inValue.z;
        outValue.b = t;
    }
    else if (i == 3) {
        outValue.r = p;
        outValue.g = q;
        outValue.b = inValue.z;
    }
    else if (i == 4) {
        outValue.r = t;
        outValue.g = p;
        outValue.b = inValue.z;
    }
    else {
        outValue.r = inValue.z;
        outValue.g = p;
        outValue.b = q;
    }
    return outValue;
}

vec2 rotate(vec2 v, float angle)
{
    float rad = angle * PI / 180.0;
    return vec2(cos(rad) * v.x - sin(rad) * v.y, sin(rad) * v.x + cos(rad)*v.y);
}



void main()
{

    vec2 UV = fs_UV;

//    UV.x += sin(-fs_UV.y*5 + (u_Time / 5) % 100 * PI / 50 ) / 60
//          + sin(fs_UV.x*15 + (u_Time / 10) % 100 * PI / 50 ) / 200
//          + cos(fs_UV.x*10 + (u_Time / 40) % 100 * PI / 50 ) / 30;
//    UV.y += cos(fs_UV.x*10 + (u_Time / 20) % 100 * PI / 50 ) / 50
//          - sin(-fs_UV.y*5 + (u_Time / 10) % 100 * PI / 50 ) / 50;

//    color = texture(u_RenderedTexture, UV).rgb;

//    float dx = fbm(UV + vec2(1.0 / u_Dimensions.x, 0)) - fbm(UV - vec2(1.0 / u_Dimensions.x, 0));
//    float dy = fbm(UV + vec2(0, 1.0 / u_Dimensions.y)) - fbm(UV - vec2(0, 1.0 / u_Dimensions.y));

    float dx = 0.25 * fbm(UV) * (cos(3.14  * u_Time / 300 + 5) + 4*sin(3.14  * u_Time / 600 - 15));
    float dy = 0.5 * fbm(UV) * (sin(3.14  * u_Time / 300 - 5) + cos(3.14  * u_Time / 600 + 15));

    color = texture(u_RenderedTexture, UV+vec2(dx, dy) * 0.5).rgb;



}




//uniform ivec2 u_Dimensions;
//uniform int u_Time;

//in vec2 fs_UV;

//out vec3 color;

//uniform sampler2D u_RenderedTexture;

//int row = 5;
//int col = 5;


//vec2 random2( vec2 p) {
//    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
//}

//float BlinnPhoneReflection(vec4 eye, vec4 light, vec4 norn, float shininess) {
//    vec4 h_vec = normalize(eye) + normalize(light);
////    h_vec /= 2;
//    h_vec = normalize(h_vec);
//    float S = max(pow(dot(h_vec, norn), shininess), 0.0f);
//    return S;
//}

//vec2 noiseCenter(vec2 FragCoord, int index) {
//    float cell_width = u_Dimensions.x * 2 / col;
//    float cell_height = u_Dimensions.y * 2 / row;

//    int x = index % col;
//    int y = index / col;
//    float noiseCenter_x = cell_width * (x+random2(vec2(x,y)).x);
//    float noiseCenter_y = cell_height * (y+random2(vec2(x,y)).y);

//    return vec2(noiseCenter_x, noiseCenter_y);
//}

//float worley(vec2 FragCoord) {

//    float cell_width = u_Dimensions.x * 2 / col;
//    float cell_height = u_Dimensions.y * 2 / row;

//    int cell_x = int (FragCoord.x / cell_width);
//    int cell_y = int (FragCoord.y / cell_height);

//    // compare with near 8 cells
//    float distance = 10000;

//    for (int i = 0; i < 9; i++) {
//        int x = i % 3;
//        int y = i / 3;

//        int x_offset = x - 1;
//        int y_offset = y - 1;
//        if (((cell_x + x_offset) + (cell_y + y_offset) * col) < row * col &&
//                ((cell_x + x_offset) + (cell_y + y_offset) * col) >= 0 ){
//            ;
//            float cur_distance = sqrt(pow((FragCoord.x - noiseCenter(FragCoord, (cell_x + x_offset) + (cell_y + y_offset) * col).x)/u_Dimensions.x,2) +
//                    pow((FragCoord.y - noiseCenter(FragCoord, (cell_x + x_offset) + (cell_y + y_offset) * col).y)/u_Dimensions.y,2));

//            if (cur_distance < distance) {
//                distance = cur_distance;
//            }
//        }
//    }
//    return distance;
//}

//void main()
//{
//    // TODO Homework 5

//    float cell_width = u_Dimensions.x * 2 / col;
//    float cell_height = u_Dimensions.y * 2 / row;



//    float grad_x = worley(vec2(gl_FragCoord.x+1, gl_FragCoord.y)) - worley(vec2(gl_FragCoord.x-1, gl_FragCoord.y));
//    float grad_y = worley(vec2(gl_FragCoord.x, gl_FragCoord.y+1)) - worley(vec2(gl_FragCoord.x, gl_FragCoord.y-1));

//    float distance = worley(vec2(gl_FragCoord));
////    vec3 shaderColor = vec3(0.3, 0.9, 0.9) * distance * 20 ;
//    vec3 shaderColor = vec3(distance * 5);

////    color = shaderColor;

////    color = shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(1.1 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100 - 2.8),
////                                                                       0.7 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100))));


//    color = vec3(texture(u_RenderedTexture, fs_UV + vec2(1.1 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100 - 2.8),
//                                                         0.7 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100))));


//    //    color = Blinn *( shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(grad_x, grad_y) * u_Dimensions.x / col / 5)));
////    color = ( shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(grad_x, grad_y) * u_Dimensions.x / col / 5)));

//    //    float distance = worley(vec2(gl_FragCoord));
////    color = vec3(distance);
//}
