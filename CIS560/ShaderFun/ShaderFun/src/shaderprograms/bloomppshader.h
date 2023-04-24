#pragma once

#include "postprocessshader.h"

class BloomShader : public PostProcessShader
{
public:

    int unifBrightTextureSampler; // A handle to the "uniform" sampler2D that will
                                   // read from the output of the render pass that
                                   // isolated the bright parts of the scene.

public:
    BloomShader(GLWidget277* context, int numRenderPasses);
    virtual ~BloomShader();

    // Sets up shader-specific handles
    virtual void setupMemberVars() override;
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d, int textureSlot) override;
};
