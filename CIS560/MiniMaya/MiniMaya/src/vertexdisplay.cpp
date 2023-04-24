#include "vertexdisplay.h"

VertexDisplay::VertexDisplay(OpenGLContext* context)
    : Drawable(context),
      representedVertex(nullptr)
{}

void VertexDisplay::create() {
    if (this->representedVertex) {
        GLuint idx = 0;
        glm::vec4 pos = glm::vec4(this->representedVertex->pos, 1.f);
        glm::vec4 color = glm::vec4(1.f);

        this->count = 1;

        this->generateIdx();
        this->bindIdx();
        this->mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint), &idx, GL_STATIC_DRAW);

        this->generatePos();
        this->bindPos();
        this->mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), &pos, GL_STATIC_DRAW);

        this->generateCol();
        this->bindCol();
        this->mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), &color, GL_STATIC_DRAW);
    }
}

void VertexDisplay::updateVertex(Vertex* vert) {
    this->representedVertex = vert;
}

GLenum VertexDisplay::drawMode() {
    return GL_POINTS;
}
