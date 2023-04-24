#version 150

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;
uniform ivec2 u_Dimensions;

// Sobel Kernel
const float SobelX[9] = float[9]( 3, 0,  -3,
                                 10, 0,  -10,
                                  3, 0,  -3);

const float SobelY[9] = float[9]( 3, 10,   3,
                                  0,  0,   0,
                                 -3, -10, -3);


void main()
{
    vec3 dx = vec3(0);
    vec3 dy = vec3(0);

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 delta = vec2(float(i) / u_Dimensions.x, float(j) / u_Dimensions.y);
            vec3 Color = texture(u_RenderedTexture, fs_UV + delta).rgb;
            dx += SobelX[(j + 1) + (i + 1) * 3] * Color;
            dy += SobelY[(j + 1) + (i + 1) * 3] * Color;
        }
    }
    color = vec3(dx * dx + dy * dy);
}
