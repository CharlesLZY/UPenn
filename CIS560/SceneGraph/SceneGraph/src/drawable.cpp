#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_bufIdx(), m_bufPos(), m_bufCol(),
      m_idxBound(false), m_posBound(false), m_colBound(false),
      mp_context(context)
{}

Drawable::~Drawable()
{
    destroy();
}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufCol);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

void Drawable::generateIdx()
{
    m_idxBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generatePos()
{
    m_posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateCol()
{
    m_colBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

bool Drawable::bindIdx()
{
    if (m_idxBound)
    {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxBound;
}

bool Drawable::bindPos()
{
    if (m_posBound)
    {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posBound;
}

bool Drawable::bindCol()
{
    if (m_colBound)
    {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colBound;
}
