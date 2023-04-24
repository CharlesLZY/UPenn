#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_count_transparent(-1), m_count_after_transparent(-1),
      m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufUV(), m_bufInterleave(), m_bufIdx_after_transparent(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false),
      m_uvGenerated(false), m_interleaveGenerated(false), m_interleaveGenerated_after_transparent(false),
      m_idxGenerated_transparent(false), m_interleaveGenerated_transparent(false),
      m_idxGenerated_after_transparent(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufUV);
    m_idxGenerated = m_posGenerated = m_norGenerated = m_uvGenerated = false;
    m_count = -1;
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

int Drawable::elemCount_transparent()
{
    return m_count_transparent;
}

int Drawable::elemCount_after_transparent()
{
    return m_count_after_transparent;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateInterleave(){
    m_interleaveGenerated = true;
    mp_context->glGenBuffers(1, &m_bufInterleave);
}

void Drawable::generateIdx_transparent()
{
    m_idxGenerated_transparent = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx_transparent);
}

void Drawable::generateInterleave_transparent(){
    m_interleaveGenerated_transparent = true;
    mp_context->glGenBuffers(1, &m_bufInterleave_transparent);
}

void Drawable::generateIdx_after_transparent()
{
    m_idxGenerated_after_transparent = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx_after_transparent);
}

void Drawable::generateInterleave_after_transparent(){
    m_interleaveGenerated_after_transparent = true;
    mp_context->glGenBuffers(1, &m_bufInterleave_after_transparent);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateUV(){
    m_uvGenerated = true;
    mp_context->glGenBuffers(1, &m_bufUV);

}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindInterleave(){
    if (m_interleaveGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleave);
    }
    return m_interleaveGenerated;
}

bool Drawable::bindIdx_transparent()
{
    if(m_idxGenerated_transparent) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx_transparent);
    }
    return m_idxGenerated_transparent;
}

bool Drawable::bindInterleave_transparent(){
    if (m_interleaveGenerated_transparent){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleave_transparent);
    }
    return m_interleaveGenerated_transparent;
}

bool Drawable::bindIdx_after_transparent()
{
    if(m_idxGenerated_after_transparent) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx_after_transparent);
    }
    return m_idxGenerated_after_transparent;
}

bool Drawable::bindInterleave_after_transparent(){
    if (m_interleaveGenerated_after_transparent){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufInterleave_after_transparent);
    }
    return m_interleaveGenerated_after_transparent;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindUV(){
    if (m_uvGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}


void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}
