#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_texture;
uniform int u_PostType;

uniform int u_Time;

void main()
{
    vec3 diffuse_color = texture(u_texture, fs_UV).rgb;

    if (u_PostType == 1) {
        vec2 uv = fs_UV;
        float X = uv.x * 10.0 + u_Time * 3.14159 / 400.f;
        float Y = uv.y * 10.0 + u_Time * 3.14159 / 400.f;
        uv.y += cos(X + Y) * 0.01 * cos(Y);
        uv.x += sin(X + Y) * 0.01 * sin(Y);
        diffuse_color = texture(u_texture, uv).rgb;
        diffuse_color += vec3(0, 0, 0.3);
    }

    else if (u_PostType == 2) {
        diffuse_color = vec3(1.f, 0, 0);
    }

    out_Col = vec4(diffuse_color, 1);
}
