#include "facedisplay.h"

FaceDisplay::FaceDisplay(OpenGLContext* context)
    : Drawable(context),
      representedFace(nullptr)
{}

void FaceDisplay::create() {
    if (this->representedFace) {
        std::vector<GLuint> idxVec;
        std::vector<glm::vec4> posVec;
        std::vector<glm::vec4> colorVec;

        HalfEdge *curEdge = this->representedFace->edge;
        int edgeNum = 0; // # vertex = # edge
        do {
            posVec.push_back(glm::vec4(curEdge->vert->pos, 1));
            // colored with the opposite color of the Face
            colorVec.push_back(glm::vec4(1.f - this->representedFace->color, 1));

            curEdge = curEdge->next;
            edgeNum += 1;
        } while (curEdge != this->representedFace->edge);

        for (int i=0; i<edgeNum; i++) {
            idxVec.push_back(i);
            idxVec.push_back((i+1) % edgeNum);
        }

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

void FaceDisplay::updateFace(Face* face) {
    this->representedFace = face;
}

GLenum FaceDisplay::drawMode() {
    return GL_LINES;
}
