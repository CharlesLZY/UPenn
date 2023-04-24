#include "bloomppshader.h"

BloomShader::BloomShader(GLWidget277 *context, int numRenderPasses)
    : PostProcessShader(context, numRenderPasses)
{}

BloomShader::~BloomShader()
{}

void BloomShader::setupMemberVars()
{
    PostProcessShader::setupMemberVars();

    unifBrightTextureSampler = context->glGetUniformLocation(prog, "u_BrightTexture");
}

void BloomShader::draw(Drawable &d, int textureSlot)
{
    useMe();

    // Set our "renderedTexture" sampler to user Texture Unit 0
    context->glUniform1i(unifSampler2D, textureSlot);
    // Set our sampler used to read the blurred texture to read from texture unit 1.
    context->glUniform1i(unifBrightTextureSampler, 1);

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    if (attrPos != -1 && d.bindPos()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
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
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);

    context->printGLErrorLog();
}
