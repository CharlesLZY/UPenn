#version 150

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    vec4 Color = texture(u_RenderedTexture, fs_UV);
    // the green channel has the highest contribution of the three by a large margin
    // this is because the human eye is most sensitive to green light
    float grey = 0.21 * Color.r + 0.72 * Color.g + 0.07 * Color.b;

    vec2 center = vec2(0.5, 0.5);   // center

    float dist = distance(fs_UV, center);
    // vignetting effect
    color = clamp(vec3(grey - dist), 0.f, 1.f);

}
