#version 150

uniform mat4 u_Model;
uniform mat3 u_ModelInvTr;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform vec3 u_Cam;

uniform int u_Time;

in vec4 vs_Pos;
in vec4 vs_Nor;

out vec3 fs_Pos;
out vec3 fs_Nor;

out vec3 fs_LightVec;

void main()
{
    // TODO Homework 4
    fs_Nor = normalize(u_ModelInvTr * vec3(vs_Nor));

    float radius = 2.5; // sphere radius
    // map the original vertex to sphere surface
    vec4 newPos = vec4(normalize(vec3(vs_Pos)) * radius, 1);
    vec4 modelposition = u_Model * (vs_Pos + (newPos - vs_Pos) * cos(u_Time*0.008) *cos(u_Time*0.008));


    fs_Pos = vec3(modelposition);
    gl_Position = u_Proj * u_View * modelposition;

    fs_LightVec = u_Cam - vec3(modelposition);
}
