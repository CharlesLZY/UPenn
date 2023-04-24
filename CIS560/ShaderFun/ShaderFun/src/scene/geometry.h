#pragma once
#include <shaderprogram.h>

class Geometry : public ShaderProgram::Drawable
{
protected:
    int count;
    QOpenGLBuffer bufIdx;
    QOpenGLBuffer bufPos;
    QOpenGLBuffer bufNor;
    QOpenGLBuffer bufCol;//Can be used to pass per-vertex color information to the shader, but is currently unused.

public:
    Geometry();

    virtual void create() = 0;
    void destroy();

    virtual GLenum drawMode();
    virtual int elemCount();
    virtual bool bindIdx();
    virtual bool bindPos();
    virtual bool bindNor();
    virtual bool bindCol();
};
