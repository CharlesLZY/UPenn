#include "polygon.h"
#include <glm/gtx/matrix_transform_2d.hpp>

Polygon2D::Polygon2D(OpenGLContext* context)
    : Drawable(context), m_vertPos(), m_vertIdx(), m_numVertices(0)
{}

Polygon2D::Polygon2D(OpenGLContext* context, int numSides)
    : Drawable(context), m_vertPos(), m_vertIdx(), m_numVertices(numSides)
{
    // Vertex positions
    glm::vec3 p(0.5f, 0.f, 1.f);
    float deg = glm::radians(360.f / numSides);
    for (int i = 0; i < numSides; i++)
    {
        glm::mat3 M = glm::rotate(glm::mat3(), i * deg);
        m_vertPos.push_back(M * p);
    }
    // Indices for triangulation
    int n = numSides - 2;
    for (int i = 0; i < n; i++)
    {
        m_vertIdx.push_back(0);
        m_vertIdx.push_back(i+1);
        m_vertIdx.push_back(i+2);
    }
}

Polygon2D::Polygon2D(OpenGLContext* context, const std::vector<glm::vec3>& positions)
    : Drawable(context), m_vertPos(positions), m_vertIdx(), m_numVertices(positions.size())
{
    int n = m_numVertices - 2;
    for (int i = 0; i < n; i++)
    {
        m_vertIdx.push_back(0);
        m_vertIdx.push_back(i+1);
        m_vertIdx.push_back(i+2);
    }
}

void Polygon2D::create()
{
    // Track our index count in a member variable
    // to be read by ShaderProgram::draw().
    m_count = m_vertIdx.size();
    m_numVertices = m_vertPos.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // the number of indices multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), m_vertIdx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, m_numVertices * sizeof(glm::vec3), m_vertPos.data(), GL_STATIC_DRAW);

    // Free up memory now that we no longer need the vertex info to be stored on the CPU
    m_vertIdx.clear();
    m_vertPos.clear();
}

void Polygon2D::setColor(glm::vec3 c)
{
    if (!bindCol())
    {
        generateCol();
    }
    std::vector<glm::vec3> colors(m_numVertices);
    std::fill(colors.begin(), colors.end(), c);
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, m_numVertices * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
}
