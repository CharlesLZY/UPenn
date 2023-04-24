#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      prog_flat(this),
      m_geomGrid(this), m_geomSquare(this, {glm::vec3(0.5f, 0.5f, 1.f),
                                            glm::vec3(-0.5f, 0.5f, 1.f),
                                            glm::vec3(-0.5f, -0.5f, 1.f),
                                            glm::vec3(0.5f, -0.5f, 1.f)}),
      m_showGrid(true),
      mp_selectedNode(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();

    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
    m_geomGrid.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the scene geometry
    m_geomGrid.create();
    m_geomSquare.create();

    // Create and set up the flat lighting shader
    prog_flat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);


    // TODO: Call your scene graph construction function here
    this->root = constructSceneGraph();

    emit sig_sendRootNode(this->root.get());
}

uPtr<Node> MyGL::constructSceneGraph() {
    uPtr<Node> root = mkU<TranslateNode>("root", nullptr, 0, 0);

    // A torso that serves as the "root" of all body transformations
    Node &torso_translate = root->addNewChild(mkU<TranslateNode>("torso_translate", nullptr, 0, 0));
    Node &torso_rotate = torso_translate.addNewChild(mkU<RotateNode>("torso_rotate", nullptr, 0));
    Node &FIXED_torso_size = torso_rotate.addNewChild(mkU<ScaleNode>("FIXED_torso_size", &m_geomSquare, 1, 2));
    FIXED_torso_size.setColor(glm::vec3(0,0,0));

    // left limb 1
    Node &FIXED_left_limb1_root = torso_rotate.addNewChild(mkU<TranslateNode>("FIXED_left_limb1_root", nullptr, -0.5, 0));
    Node &left_limb1_rotate = FIXED_left_limb1_root.addNewChild(mkU<RotateNode>("left_limb1_rotate", nullptr, -45));
    Node &FIXED_left_limb1_shift = left_limb1_rotate.addNewChild(mkU<TranslateNode>("FIXED_left_limb1_shift", nullptr, -0.5, 0));
    Node &FIXED_left_limb1_size = FIXED_left_limb1_shift.addNewChild(mkU<ScaleNode>("FIXED_left_limb1_size", &m_geomSquare, 1.5, 0.3));
    FIXED_left_limb1_size.setColor(glm::vec3(0,0,0));

    // left limb 2
    Node &FIXED_left_limb2_root = FIXED_left_limb1_shift.addNewChild(mkU<TranslateNode>("FIXED_left_limb2_root", nullptr, -0.8, 0));
    Node &left_limb2_rotate = FIXED_left_limb2_root.addNewChild(mkU<RotateNode>("left_limb2_rotate", nullptr, 120));
    Node &FIXED_left_limb2_shift = left_limb2_rotate.addNewChild(mkU<TranslateNode>("FIXED_left_limb2_shift", nullptr, -1.2, 0));
    Node &FIXED_left_limb2_size = FIXED_left_limb2_shift.addNewChild(mkU<ScaleNode>("FIXED_left_limb2_size", &m_geomSquare, 2.5, 0.3));
    FIXED_left_limb2_size.setColor(glm::vec3(1,0,0));

    // right limb 1
    Node &FIXED_right_limb1_root = torso_rotate.addNewChild(mkU<TranslateNode>("FIXED_right_limb1_root", nullptr, 0.5, 0));
    Node &right_limb1_rotate = FIXED_right_limb1_root.addNewChild(mkU<RotateNode>("right_limb1_rotate", nullptr, 45));
    Node &FIXED_right_limb1_shift = right_limb1_rotate.addNewChild(mkU<TranslateNode>("FIXED_right_limb1_shift", nullptr, 0.5, 0));
    Node &FIXED_right_limb1_size = FIXED_right_limb1_shift.addNewChild(mkU<ScaleNode>("FIXED_right_limb1_size", &m_geomSquare, 1.5, 0.3));
    FIXED_right_limb1_size.setColor(glm::vec3(0,0,0));

    // right limb 2
    Node &FIXED_right_limb2_root = FIXED_right_limb1_shift.addNewChild(mkU<TranslateNode>("FIXED_right_limb2_root", nullptr, 0.8, 0));
    Node &right_limb2_rotate = FIXED_right_limb2_root.addNewChild(mkU<RotateNode>("right_limb2_rotate", nullptr, -120));
    Node &FIXED_right_limb2_shift = right_limb2_rotate.addNewChild(mkU<TranslateNode>("FIXED_right_limb2_shift", nullptr, 1.2, 0));
    Node &FIXED_right_limb2_size = FIXED_right_limb2_shift.addNewChild(mkU<ScaleNode>("FIXED_right_limb2_size", &m_geomSquare, 2.5, 0.3));
    FIXED_right_limb2_size.setColor(glm::vec3(1,0,0));


    // head
    Node &FIXED_head_root = torso_rotate.addNewChild(mkU<TranslateNode>("FIXED_head_root", nullptr, 0, 1));
    Node &head_rotate = FIXED_head_root.addNewChild(mkU<RotateNode>("head_rotate", nullptr, 45));
    Node &FIXED_head_size = head_rotate.addNewChild(mkU<ScaleNode>("FIXED_head_size", &m_geomSquare, 1, 1));
    FIXED_head_size.setColor(glm::vec3(1,0,0));

    // eye 1
    Node &FIXED_eye1_root = head_rotate.addNewChild(mkU<TranslateNode>("FIXED_eye1_root", nullptr, 0, 0));
    Node &FIXED_eye1_size = FIXED_eye1_root.addNewChild(mkU<ScaleNode>("FIXED_eye1_size", &m_geomSquare, 1, 0.2));
    FIXED_eye1_size.setColor(glm::vec3(1,1,0));

    // eye 2
    Node &FIXED_eye2_root = head_rotate.addNewChild(mkU<TranslateNode>("FIXED_eye2_root", nullptr, 0, 0));
    Node &FIXED_eye2_size = FIXED_eye2_root.addNewChild(mkU<ScaleNode>("FIXED_eye2_size", &m_geomSquare, 0.2, 1));
    FIXED_eye2_size.setColor(glm::vec3(1,1,0));

    // tail
    Node &FIXED_tail_root = torso_rotate.addNewChild(mkU<TranslateNode>("FIXED_tail_root", nullptr, 0, -1));
    Node &tail_rotate = FIXED_tail_root.addNewChild(mkU<RotateNode>("tail_rotate", nullptr, 45));
    Node &FIXED_tail_size = tail_rotate.addNewChild(mkU<ScaleNode>("FIXED_tail_size", &m_geomSquare, 0.7, 0.7));
    FIXED_tail_size.setColor(glm::vec3(0,0,0));

    return root;
}

void MyGL::traverseSceneGraph(const uPtr<Node> &node, glm::mat3 mat) {
    mat = mat * node->transformation();

    for (const uPtr<Node> &n : node->getChildren()) {
        traverseSceneGraph(n, mat);
    }

    if (node->getGeometry()) {
        node->getGeometry()->setColor(node->getColor());
        prog_flat.setModelMatrix(mat);
        prog_flat.draw(*this, *node->getGeometry());
    }
}


void MyGL::resizeGL(int w, int h)
{
    glm::mat3 viewMat = glm::scale(glm::mat3(), glm::vec2(0.2, 0.2)); // Screen is -5 to 5

    // Upload the view matrix to our shader (i.e. onto the graphics card)
    prog_flat.setViewMatrix(viewMat);

    printGLErrorLog();
}

// This function is called by Qt any time your GL window is supposed to update
// For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_showGrid)
    {
        prog_flat.setModelMatrix(glm::mat3());
        prog_flat.draw(*this, m_geomGrid);
    }

    //VVV CLEAR THIS CODE WHEN YOU IMPLEMENT SCENE GRAPH TRAVERSAL VVV///////////////////

    // Shapes will be drawn on top of one another, so the last object
    // drawn will appear in front of everything else

//    prog_flat.setModelMatrix(glm::mat3());
//    m_geomSquare.setColor(glm::vec3(0,1,0));
//    prog_flat.draw(*this, m_geomSquare);

//    m_geomSquare.setColor(glm::vec3(1,0,0));
//    prog_flat.setModelMatrix(glm::translate(glm::mat3(), glm::vec2(-1.f, 0.f)) * glm::rotate(glm::mat3(), glm::radians(-30.f)));
//    prog_flat.draw(*this, m_geomSquare);

//    m_geomSquare.setColor(glm::vec3(0,0,1));
//    prog_flat.setModelMatrix(glm::translate(glm::mat3(), glm::vec2(1.f, 0.f)) * glm::rotate(glm::mat3(), glm::radians(30.f)));
//    prog_flat.draw(*this, m_geomSquare);

    //^^^ CLEAR THIS CODE WHEN YOU IMPLEMENT SCENE GRAPH TRAVERSAL ^^^/////////////////

    // Here is a good spot to call your scene graph traversal function.
    // Any time you want to draw an instance of geometry, call
    // prog_flat.draw(*this, yourNonPointerGeometry);

    traverseSceneGraph(this->root, glm::mat3());

}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    switch(e->key())
    {
    case(Qt::Key_Escape):
        QApplication::quit();
        break;

    case(Qt::Key_G):
        m_showGrid = !m_showGrid;
        break;
    }
}

void MyGL::slot_setSelectedNode(QTreeWidgetItem *i) {
    mp_selectedNode = static_cast<Node*>(i);

    if (mp_selectedNode->getType() == 1) {
        emit sig_translateNodeSeleted(mp_selectedNode->getValue(0), mp_selectedNode->getValue(1));
    } else if (mp_selectedNode->getType() == 2) {
        emit sig_rotateNodeSeleted(mp_selectedNode->getValue(0));
    } else if (mp_selectedNode->getType() == 3) {
        emit sig_scaleNodeSeleted(mp_selectedNode->getValue(0), mp_selectedNode->getValue(1));
    }
}

void MyGL::slot_setTX(double x) {
    // TODO update the currently selected Node's
    // X translation value IF AND ONLY IF
    // the currently selected node can be validly
    // dynamic_cast to a TranslateNode.
    // Remember that a failed dynamic_cast
    // will return a null pointer.

    mp_selectedNode->setValue((float)x, 0);
}

void MyGL::slot_setTY(double y) {
    mp_selectedNode->setValue((float)y, 1);
}

void MyGL::slot_setR(double r) {
    mp_selectedNode->setValue((float)r, 0);
}

void MyGL::slot_setSX(double x) {
    mp_selectedNode->setValue((float)x, 0);
}

void MyGL::slot_setSY(double y) {
    mp_selectedNode->setValue((float)y, 1);
}


void MyGL::slot_addTranslateNode() {
    // TODO invoke the currently selected Node's
    // addChild function on a newly-instantiated
    // TranslateNode.

    mp_selectedNode->Node::addNewChild(mkU<TranslateNode>("new_translate", nullptr, 0, 0));
}

void MyGL::slot_addRotateNode() {
    mp_selectedNode->Node::addNewChild(mkU<RotateNode>("new_rotate", nullptr, 0));
}

void MyGL::slot_addScaleNode() {
     mp_selectedNode->Node::addNewChild(mkU<ScaleNode>("new_scale", nullptr, 1, 1));
}

void MyGL::slot_setGeometry() {
    mp_selectedNode->setGeometry(&m_geomSquare);
}
