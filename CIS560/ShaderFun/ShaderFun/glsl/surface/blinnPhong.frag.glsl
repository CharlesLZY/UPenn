#version 330

//This is a fragment shader. If you've opened this file first, please open and read lambert.vert.glsl before reading on.
//Unlike the vertex shader, the fragment shader actually does compute the shading of geometry.
//For every pixel in your program's output screen, the fragment shader is run for every bit of geometry that particular pixel overlaps.
//By implicitly interpolating the position data passed into the fragment shader by the vertex shader, the fragment shader
//can compute what color to apply to its pixel based on things like vertex position, light position, and vertex color.

uniform sampler2D u_Texture; // The texture to be read from by this shader

//These are the interpolated values out of the rasterizer, so you can't know
//their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec2 fs_UV;

in vec4 fs_CameraPos;
in vec4 fs_Pos;

layout(location = 0) out vec3 out_Col;//This is the final output color that you will see on your screen for the pixel that is currently being processed.

void main()
{
    // TODO Homework 4
    // Blinn-Phong reflection is a method of emulating the specular highlights
    // one sees on materials like plastic

    // refer to the slide "Texturing, Shading, and Lighting" P25
    // V
    vec4 viewDir = fs_CameraPos - fs_Pos;
    // H
    vec4 halfDir = normalize((viewDir + fs_LightVec) / 2);
    // L
    vec4 lightDir = normalize(fs_LightVec);

    float ambient = 0.3;
    float diffusion = dot(fs_Nor, lightDir); // lambertian shading

    // shininess controls how diffuse the highlight is,
    // with smaller values being more diffuse
    float shininess = 32;
    float specular = max(pow(dot(halfDir, fs_Nor), shininess), 0);

    vec4 Color = texture(u_Texture, fs_UV);
    out_Col = Color.rgb * (ambient + diffusion + specular);




}
