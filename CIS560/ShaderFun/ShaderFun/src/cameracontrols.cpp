#include "mygl.h"
#include <QKeyEvent>
#include <QApplication>

void MyGL::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons() & (Qt::LeftButton | Qt::RightButton))
    {
        m_mousePosPrev = glm::vec2(e->pos().x(), e->pos().y());
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    glm::vec2 pos(e->pos().x(), e->pos().y());
    if(e->buttons() & Qt::LeftButton)
    {
        // Rotation
        glm::vec2 diff = 0.2f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_camera.RotatePhi(-diff.x);
        m_camera.RotateTheta(-diff.y);
    }
    else if(e->buttons() & Qt::RightButton)
    {
        // Panning
        glm::vec2 diff = 0.05f * (pos - m_mousePosPrev);
        m_mousePosPrev = pos;
        m_camera.TranslateAlongRight(-diff.x);
        m_camera.TranslateAlongUp(diff.y);
    }
}

void MyGL::wheelEvent(QWheelEvent *e)
{
    m_camera.Zoom(e->angleDelta().y() * 0.02f);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{

    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    switch(e->key())
    {
    case (Qt::Key_Escape):
        QApplication::quit();
        break;
    case (Qt::Key_Right):
        m_camera.RotateAboutUp(-amount);
        break;
    case (Qt::Key_Left):
        m_camera.RotateAboutUp(amount);
        break;
    case (Qt::Key_Up):
        m_camera.RotateAboutRight(-amount);
        break;
    case (Qt::Key_Down):
        m_camera.RotateAboutRight(amount);
        break;
    case (Qt::Key_1):
        m_camera.fovy += amount;
        break;
    case (Qt::Key_2):
        m_camera.fovy -= amount;
        break;
    case (Qt::Key_W):
        m_camera.TranslateAlongLook(amount);
        break;
    case (Qt::Key_S):
        m_camera.TranslateAlongLook(-amount);
        break;
    case (Qt::Key_D):
        m_camera.TranslateAlongRight(amount);
        break;
    case (Qt::Key_A):
        m_camera.TranslateAlongRight(-amount);
        break;
    case (Qt::Key_Q):
        m_camera.TranslateAlongUp(-amount);
        break;
    case (Qt::Key_E):
        m_camera.TranslateAlongUp(amount);
        break;
    case (Qt::Key_F):
        m_camera.Reset();
        break;
    }
    m_camera.RecomputeAttributes();
}
