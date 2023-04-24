#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

uniform int u_Time;

in vec4 vs_UV;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_UV;

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

void main()
{
    fs_Pos = u_Model * vs_Pos;
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_UV = vs_Col.xy;

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.
//    fs_Nor = vs_Nor;
    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    if (vs_Nor.w == 0.5){
        fs_Nor = vs_Nor;
    }else{
        //WATER
        float tx = modelposition.x * 0.1 + u_Time / 5.f;
        float tz = modelposition.z * 0.1 + u_Time / 5.f;
        float hx = (sin(0.5f * tx) + sin(0.6 * tx + 6.f) +
                sin(0.3 * tx + 1.f) +
                    sin(0.7 * tx + 9.f)) / 4.f;
        float hz = (sin(0.5 * tz) + sin(0.6 * tz + 6.f) +
                sin(0.3 * tz + 1.f) +
                    sin(0.7 * tz + 9.f)) / 4.f;
        float h = 0.25f * (hz - 1.f + hx - 1.f);
        modelposition.y += h;
    }
    if (fs_UV.x >= 15.f * 0.0625 && fs_UV.x < 16.f * 0.0625
                        && fs_UV.y >= 14.f * 0.0625 && fs_UV.y < 15.f * 0.0625){
        float tx = modelposition.x * 0.1 + u_Time / 10.f;
        float tz = modelposition.z * 0.1 + u_Time / 10.f;
        float hx = (sin(1.5f * tx) + sin(2.6 * tx + 6.f) +
                sin(3.3 * tx + 1.f) +
                    sin(4.7 * tx + 9.f)) / 4.f;
        float hz = (sin(1.5 * tz) + sin(3.6 * tz + 6.f) +
                sin(2.3 * tz + 1.f) +
                    sin(4.7 * tz + 9.f)) / 4.f;
        float h = 0.25f * (hz - 1.f + hx - 1.f);
        modelposition.x += h;
        modelposition.z -= h;
    }


    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
