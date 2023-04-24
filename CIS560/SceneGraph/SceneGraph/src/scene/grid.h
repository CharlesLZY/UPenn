#pragma once
#include "drawable.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Grid : public Drawable
{
public:
    Grid(OpenGLContext* context);
    void create() override;

    GLenum drawMode() override;
};
