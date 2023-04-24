#include <scene/geometry.h>
#include<la.h>

Geometry::Geometry()
    : bufIdx(QOpenGLBuffer::IndexBuffer),
      bufPos(QOpenGLBuffer::VertexBuffer),
      bufNor(QOpenGLBuffer::VertexBuffer),
      bufCol(QOpenGLBuffer::VertexBuffer)
{}


void Geometry::destroy()
{
    bufIdx.destroy();
    bufPos.destroy();
    bufNor.destroy();
    bufCol.destroy();
}

GLenum Geometry::drawMode()
{
    return GL_TRIANGLES;
}

int Geometry::elemCount()
{
    return count;
}

bool Geometry::bindIdx()
{
    return bufIdx.bind();
}

bool Geometry::bindPos()
{
    return bufPos.bind();
}

bool Geometry::bindNor()
{
    return bufNor.bind();
}

bool Geometry::bindCol()
{
    return bufCol.bind();
}
