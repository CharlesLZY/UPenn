#version 150
// ^ Change this to version 130 if you have compatibility issues

uniform mat3 u_Model;
uniform mat3 u_View;

in vec3 vs_Pos;
in vec3 vs_Col;

out vec3 fs_Col;

void main()
{
    fs_Col = vs_Col;

    //built-in things to pass down the pipeline
    vec3 finalPos = u_View * u_Model * vs_Pos;
    gl_Position = vec4(finalPos.xy, finalPos.z - 0.001, 1);

}
