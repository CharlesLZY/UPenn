#include "halfedgedisplay.h"


HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext *context)
    : Drawable(context),
      representedEdge(nullptr)
{}

void HalfEdgeDisplay::create() {
    if (this->representedEdge) {
        std::vector<GLuint> idxVec;
        std::vector<glm::vec4> posVec;
        std::vector<glm::vec4> colorVec;

        // an edge has two vertices
        idxVec.push_back(0);
        idxVec.push_back(1);

        posVec.push_back(glm::vec4(this->representedEdge->SYM->vert->pos, 1));
        posVec.push_back(glm::vec4(this->representedEdge->vert->pos, 1));

        // The line should gradiate from red to yellow,
        // with the yellow end corresponding to the Vertex to which the HalfEdge points.
        colorVec.push_back(glm::vec4(1, 0, 0, 1)); // red
        colorVec.push_back(glm::vec4(1, 1, 0, 1)); // yellow

        this->count = idxVec.size();

        this->generateIdx();
        this->bindIdx();
        this->mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxVec.size() * sizeof(GLuint), idxVec.data(), GL_STATIC_DRAW);

        this->generatePos();
        this->bindPos();
        this->mp_context->glBufferData(GL_ARRAY_BUFFER, posVec.size() * sizeof(glm::vec4), posVec.data(), GL_STATIC_DRAW);

        this->generateCol();
        this->bindCol();
        this->mp_context->glBufferData(GL_ARRAY_BUFFER, colorVec.size() * sizeof(glm::vec4), colorVec.data(), GL_STATIC_DRAW);
    }
}

void HalfEdgeDisplay::updateEdge(HalfEdge* edge) {
    this->representedEdge = edge;
}

GLenum HalfEdgeDisplay::drawMode() {
    return GL_LINES;
}
