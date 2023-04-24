#version 150

uniform ivec2 u_Dimensions;
uniform int u_Time;

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;


void main()
{

    int strideX = 20;
    int strideY = 10;
    if (u_Time % 100 < 20) {
        strideX = 60;
        strideY = 5;
    }


    int x = int(fs_UV.x * u_Dimensions.x);
    int y = int(fs_UV.y * u_Dimensions.y);

    float R = texture(u_RenderedTexture, vec2(x-strideX, y-strideY)/u_Dimensions).r;
    float G = texture(u_RenderedTexture, vec2(x, y)/u_Dimensions).g;
    float B = texture(u_RenderedTexture, vec2(x+strideX, y+strideY)/u_Dimensions).b;
    vec3 shiftColor = vec3(R, G, B);

    vec3 Color = texture(u_RenderedTexture, vec2(x, y)/u_Dimensions).rgb;
    color = Color * 0.5 + shiftColor * 0.5;

}
