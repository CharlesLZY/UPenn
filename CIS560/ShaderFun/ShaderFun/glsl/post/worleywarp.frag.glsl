#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

int row = 30;
int col = 30;


vec2 random2( vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float BlinnPhoneReflection(vec4 eye, vec4 light, vec4 norn, float shininess) {
    vec4 h_vec = normalize(eye) + normalize(light);
//    h_vec /= 2;
    h_vec = normalize(h_vec);
    float S = max(pow(dot(h_vec, norn), shininess), 0.0f);
    return S;
}

vec2 noiseCenter(vec2 FragCoord, int index) {
    float cell_width = u_Dimensions.x * 2 / col;
    float cell_height = u_Dimensions.y * 2 / row;

    int x = index % col;
    int y = index / col;
    float noiseCenter_x = cell_width * (x+random2(vec2(x,y)).x);
    float noiseCenter_y = cell_height * (y+random2(vec2(x,y)).y);

    return vec2(noiseCenter_x, noiseCenter_y);
}

float worley(vec2 FragCoord) {

    float cell_width = u_Dimensions.x * 2 / col;
    float cell_height = u_Dimensions.y * 2 / row;

    int cell_x = int (FragCoord.x / cell_width);
    int cell_y = int (FragCoord.y / cell_height);

    // compare with near 8 cells
    float distance = 10000;

    for (int i = 0; i < 9; i++) {
        int x = i % 3;
        int y = i / 3;

        int x_offset = x - 1;
        int y_offset = y - 1;
        if (((cell_x + x_offset) + (cell_y + y_offset) * col) < row * col &&
                ((cell_x + x_offset) + (cell_y + y_offset) * col) >= 0 ){
            ;
            float cur_distance = sqrt(pow((FragCoord.x - noiseCenter(FragCoord, (cell_x + x_offset) + (cell_y + y_offset) * col).x)/u_Dimensions.x,2) +
                    pow((FragCoord.y - noiseCenter(FragCoord, (cell_x + x_offset) + (cell_y + y_offset) * col).y)/u_Dimensions.y,2));

            if (cur_distance < distance) {
                distance = cur_distance;
            }
        }
    }
    return distance;
}

void main()
{
    // TODO Homework 5

    float cell_width = u_Dimensions.x * 2 / col;
    float cell_height = u_Dimensions.y * 2 / row;



    float grad_x = worley(vec2(gl_FragCoord.x+1, gl_FragCoord.y)) - worley(vec2(gl_FragCoord.x-1, gl_FragCoord.y));
    float grad_y = worley(vec2(gl_FragCoord.x, gl_FragCoord.y+1)) - worley(vec2(gl_FragCoord.x, gl_FragCoord.y-1));

    float distance = worley(vec2(gl_FragCoord));
//    vec3 shaderColor = vec3(0.3, 0.9, 0.9) * distance * 20 ;
    vec3 shaderColor = vec3(distance * 5);

//    color = shaderColor;

    color = shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(1.1 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100 - 2.8),
                                                                       0.7 * worley(vec2(gl_FragCoord.x, gl_FragCoord.y)) * cos(3.14  * u_Time / 100))));
//    color = Blinn *( shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(grad_x, grad_y) * u_Dimensions.x / col / 5)));
//    color = ( shaderColor + vec3(texture(u_RenderedTexture, fs_UV + vec2(grad_x, grad_y) * u_Dimensions.x / col / 5)));

    //    float distance = worley(vec2(gl_FragCoord));
//    color = vec3(distance);
}
