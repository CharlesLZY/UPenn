#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

//uniform mat4 u_Model;
//uniform mat4 u_ViewProj;

in vec4 vs_Pos;
in vec2 vs_UV;

out vec2 fs_UV;

void main()
{
    fs_UV = vs_UV;
    gl_Position = vs_Pos;
}
