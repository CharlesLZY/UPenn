#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <scene/terrain.h>
#include <scene/quad.h>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    Cube* mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    WorldAxes* mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram* mp_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram* mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)

    // Sky demo
    ShaderProgram* mp_progSky; // A screen-space shader for creating the sky background
    Quad* mp_geomQuad;
    float time;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera* mp_camera;
    Terrain* mp_terrain;

    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.


public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
