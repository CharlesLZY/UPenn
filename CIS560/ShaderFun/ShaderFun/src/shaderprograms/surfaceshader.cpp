#include "surfaceshader.h"


SurfaceShader::SurfaceShader(OpenGLContext *context)
    : ShaderProgram(context),
      attrPos(-1), attrNor(-1), attrUV(-1),
      unifModel(-1), unifModelInvTr(-1), unifView(-1), unifProj(-1)
{}

SurfaceShader::~SurfaceShader()
{}

void SurfaceShader::setupMemberVars()
{
    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
    attrNor = context->glGetAttribLocation(prog, "vs_Nor");
    attrUV  = context->glGetAttribLocation(prog, "vs_UV");

    unifModel      = context->glGetUniformLocation(prog, "u_Model");
    unifModelInvTr = context->glGetUniformLocation(prog, "u_ModelInvTr");
    unifView       = context->glGetUniformLocation(prog, "u_View");
    unifProj       = context->glGetUniformLocation(prog, "u_Proj");

    unifSampler2D  = context->glGetUniformLocation(prog, "u_Texture");
    unifTime = context->glGetUniformLocation(prog, "u_Time");

    unifCam = context->glGetUniformLocation(prog, "u_Cam");

    context->printGLErrorLog();
}

//This function, as its name implies, uses the passed in GL widget
void SurfaceShader::draw(Drawable &d, int textureSlot = 0)
{
    useMe();

    if(unifSampler2D != -1)
    {
        context->glUniform1i(unifSampler2D, /*GL_TEXTURE*/textureSlot);
    }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    if (attrPos != -1 && d.bindPos()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrNor != -1 && d.bindNor()) {
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 0, NULL);
    }

    if (attrUV != -1 && d.bindUV()) {
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL);
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);

    context->printGLErrorLog();
}


void SurfaceShader::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &model[0][0]);

    }

    if (unifModelInvTr != -1) {
        glm::mat3 modelinvtr = glm::inverse(glm::transpose(glm::mat3(model)));
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix3fv(unifModelInvTr,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &modelinvtr[0][0]);
    }
}

void SurfaceShader::setViewProjMatrix(const glm::mat4 &v, const glm::mat4 &p)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifView != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix4fv(unifView,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &v[0][0]);
    }

    if(unifProj != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix4fv(unifProj,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &p[0][0]);
    }
}

// refer to the style of SurfaceShasder::setModelMatrix
void SurfaceShader::setCamPos(const glm::vec3 &cam)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if (unifCam != -1) {
        // Pass a vec3 into a uniform variable in our shader
                        // Handle to the vec variable on the GPU
        context->glUniform3fv(unifCam,
                        // How many vectors to pass
                           1,
                        // Pointer to the first element of the vec
                           &cam[0]);
    }
}
