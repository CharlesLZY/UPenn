#version 150
// ^ Change this to version 130 if you have compatibility issues

#define MAX_JOINTS 100

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

uniform mat4 u_BindMatrixs[MAX_JOINTS];
uniform mat4 u_Transformations[MAX_JOINTS];

in vec4 vs_Pos;
in vec4 vs_Col;

in ivec2 vs_Joint;
in vec2 vs_JointWeight;

out vec4 fs_Col;


void main()
{
    fs_Col = vs_Col;

    vec4 pos1 = u_Transformations[vs_Joint[0]] * u_BindMatrixs[vs_Joint[0]] * vs_Pos;
    vec4 pos2 = u_Transformations[vs_Joint[1]] * u_BindMatrixs[vs_Joint[1]] * vs_Pos;

    vec4 modelposition = u_Model * vs_JointWeight[0] * pos1 + vs_JointWeight[1] * pos2;

    gl_Position = u_ViewProj * modelposition;
}

