#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <QListWidget>

#include <mesh.h>
#include <skeleton.h>
#include <vertexdisplay.h>
#include <facedisplay.h>
#include <halfedgedisplay.h>


class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram m_progSkeleton; // A shader program for skeleton

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    // Copied from example code on hw5 website
    // Have an instance of VertexDisplay that is
    // drawn in paintGL, and has VBO data representing
    // the position of the currently selected Vertex so
    // it can be drawn as a GL_POINTS
    VertexDisplay vertToDisplay;
    FaceDisplay faceToDisplay;
    HalfEdgeDisplay edgeToDisplay;

    Vertex *selectedVert;
    Face *selectedFace;
    HalfEdge *selectedEdge;

    Mesh m_mesh;
    Skeleton m_skeleton;

    Joint *setJoint(QJsonObject root);

    void updateRot();

protected:
    void keyPressEvent(QKeyEvent *e);

public slots:
    // refer to hw2
    void slot_setSelectedVertex(QListWidgetItem *vert);
    void slot_setSelectedFace(QListWidgetItem *face);
    void slot_setSelectedHalfEdge(QListWidgetItem *edge);

    void slot_setPosX(double x);
    void slot_setPosY(double y);
    void slot_setPosZ(double z);

    void slot_setColorR(double R);
    void slot_setColorG(double G);
    void slot_setColorB(double B);

    void slot_splitEdge();
    void slot_triangulate();

    void slot_subdivide();
    void slot_loadObj();

    void slot_setJointPosX(double);
    void slot_setJointPosY(double);
    void slot_setJointPosZ(double);

    void slot_setJointRotX();
    void slot_setJointRotY();
    void slot_setJointRotZ();

    void slot_setJointRotXInv();
    void slot_setJointRotYInv();
    void slot_setJointRotZInv();

    void slot_loadJson();
    void slot_bindMesh();

    void slot_setSelectedJoint(QTreeWidgetItem *joint);

signals:
    void sig_updateMesh(Mesh*);

    void sig_updateVertPosX(double);
    void sig_updateVertPosY(double);
    void sig_updateVertPosZ(double);

    void sig_updateVertColorR(double);
    void sig_updateVertColorG(double);
    void sig_updateVertColorB(double);

    void sig_updateJointPosX(double);
    void sig_updateJointPosY(double);
    void sig_updateJointPosZ(double);

    void sig_updateJointRot(QString);

    void sig_updateSkeleton(Skeleton *skeleton);

};


#endif // MYGL_H
