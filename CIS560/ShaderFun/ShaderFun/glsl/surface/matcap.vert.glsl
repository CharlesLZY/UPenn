#version 150

uniform mat4 u_Model;
uniform mat3 u_ModelInvTr;
uniform mat4 u_View;
uniform mat4 u_Proj;

in vec4 vs_Pos;
in vec4 vs_Nor;
in vec2 vs_UV;

out vec2 fs_UV;

void main()
{
    // TODO Homework 4
    vec3 fs_Nor = normalize(u_ModelInvTr * vec3(vs_Nor));
    fs_Nor = mat3(u_View) * fs_Nor;

    // helpful resourse: https://gamedev.net/forums/topic/678693-theory-behind-spherical-reflection-mapping-aka-matcap/5292021/
    // refer to https://www.clicktorelease.com/blog/creating-spherical-environment-mapping-shader/
    // Eye is the vector that goes from the camera
    // (a point in space at the origin) to the fragment position
//    vec3 e = vec3(normalize(u_View * vs_Pos));
//    // reflected vector
//    vec3 r = reflect(e, fs_Nor);

//    float m = 2. * sqrt(pow( r.x, 2. ) + pow( r.y, 2. ) + pow( r.z + 1., 2. ));
//    fs_UV = r.xy / m + .5;

    // simplified version
    // just map the normal to the texture circle
    // texture is resized to (0,0) (0,1) (1,1) （1，0） square
    // the circle will be (x-0.5)^2 + (y-0.5)^2 = 0.5^2
    // the norm can be considered as 极坐标
    fs_UV = vec2(fs_Nor/2 + 0.5);

    vec4 modelposition = u_Model * vs_Pos;
    gl_Position = u_Proj * u_View * modelposition;
}
