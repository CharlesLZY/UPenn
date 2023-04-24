#version 330

uniform sampler2D u_Texture; // The texture to be read from by this shader

in vec3 fs_Nor;
in vec3 fs_LightVec;

layout(location = 0) out vec3 out_Col;

void main()
{
    // TODO Homework 4
    float t = clamp(dot(fs_Nor, normalize(fs_LightVec)), 0.f, 1.f);
    float PI = 3.14159265;
    float R = 0.5 + 0.5 * cos(2 * PI * (t + 0.0));
    float G = 0.5 + 0.5 * cos(2 * PI * (t + 0.25));
    float B = 0.5 + 0.5 * cos(2 * PI * (t + 0.5));
    out_Col = vec3(R, G, B);
}
