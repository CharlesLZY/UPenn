#pragma once

#include "shaderprogram.h"

class PostProcessShader : public ShaderProgram
{
public:

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrUV; // A handle for the "in" vec2 representing the UV coordinates in the vertex shader

    int unifDimensions; // A handle to the "uniform" ivec2 that stores the width and height of the texture being rendered

public:
    PostProcessShader(OpenGLContext* context);
    virtual ~PostProcessShader();

    // Sets up shader-specific handles
    virtual void setupMemberVars() override;
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d, int textureSlot) override;

    void setDimensions(glm::ivec2 dims);
};
