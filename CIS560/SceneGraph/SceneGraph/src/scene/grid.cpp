#include "grid.h"
#include <la.h>
#include <iostream>

const static int NUM_IDX = 36;

Grid::Grid(OpenGLContext *context) : Drawable(context)
{}

void Grid::create()
{
    GLuint idx[NUM_IDX];
    glm::vec3 vertPos[NUM_IDX];
    glm::vec3 vertCol[NUM_IDX];

    for (int i = 0; i < NUM_IDX; i++)
    {
        idx[i] = i;
    }

    // -4 to 4
    for (int row = 0; row < 9; row++)
    {
        vertPos[row * 2] = glm::vec3(row - 4.f, 5.f, 0.f);
        vertPos[row * 2 + 1] = glm::vec3(row - 4.f, -5.f, 0.f);
    }
    for (int col = 0; col < 9; col++)
    {
        vertPos[col * 2 + 18] = glm::vec3(5.f, col - 4.f, 0.f);
        vertPos[col * 2 + 19] = glm::vec3(-5.f, col - 4.f, 0.f);
    }

    for (int i = 0; i < NUM_IDX; i++)
    {
        vertCol[i] = glm::vec3(0,0,0);
    }
    vertCol[8] = vertCol[9] = glm::vec3(1,1,1);
    vertCol[26] = vertCol[27] = glm::vec3(1,1,1);

    m_count = NUM_IDX;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, NUM_IDX * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, NUM_IDX * sizeof(glm::vec3), vertPos, GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, NUM_IDX * sizeof(glm::vec3), vertCol, GL_STATIC_DRAW);
}

GLenum Grid::drawMode()
{
    return GL_LINES;
}
