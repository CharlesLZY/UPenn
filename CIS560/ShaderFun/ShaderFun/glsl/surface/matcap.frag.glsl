#version 330

uniform sampler2D u_Texture; // The texture to be read from by this shader

in vec2 fs_UV;

layout(location = 0) out vec3 out_Col;

void main()
{
    // TODO Homework 4
    // Matcap shading is most commonly used to give 3D models the appearance of a complex material
    // with dynamic lighting without having to perform expensive lighting calculations.

    vec4 Color = texture(u_Texture, fs_UV);
    out_Col = Color.rgb;
}
