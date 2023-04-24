#include "worldaxes.h"

void WorldAxes::create()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    glm::vec4 pos[6] = {glm::vec4(32,128,32,1), glm::vec4(40,128,32,1),
                        glm::vec4(32,128,32,1), glm::vec4(32,136,32,1),
                        glm::vec4(32,128,32,1), glm::vec4(32,128,40,1)};
    glm::vec4 col[6] = {glm::vec4(1,0,0,1), glm::vec4(1,0,0,1),
                        glm::vec4(0,1,0,1), glm::vec4(0,1,0,1),
                        glm::vec4(0,0,1,1), glm::vec4(0,0,1,1)};

    count = 6;

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), pos, GL_STATIC_DRAW);
    generateCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    context->glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), col, GL_STATIC_DRAW);
}

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
