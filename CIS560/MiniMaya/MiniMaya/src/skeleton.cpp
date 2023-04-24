/*
why why why?
If the order changed, the sphere will not display.
*/
#include "skeleton.h" // the order can not change!!!
#include <glm/gtx/rotate_vector.hpp> // the order can not change!!!


Skeleton::Skeleton(OpenGLContext *context)
    : Drawable(context),
      representedJoint(nullptr),
      joints(std::vector<uPtr<Joint>>{})
{}

void Skeleton::create() {
    std::vector<GLuint> idxVec;
    std::vector<glm::vec4> posVec;
    std::vector<glm::vec4> colVec;

    int curIdx = 0; // index of vertex
    int N = 36; // how many vertices are used to draw the circle

    // draw the joints
    for (uPtr<Joint> &joint : this->joints) {

        float radius = 0.5f;
        glm::vec4 vert = glm::vec4();
        glm::vec3 X_axis = glm::vec3(1, 0, 0);
        glm::vec3 Y_axis = glm::vec3(0, 1, 0);
        glm::vec3 Z_axis = glm::vec3(0, 0, 1);

        // circle acrossed by X axis
        vert = glm::vec4(radius * Y_axis, 1);
        for (int i=0; i < N; i++) {
            posVec.push_back(joint->getOverallTransformation() * vert);

            if (joint.get() == this->representedJoint) { // if selected, then high-lighte it
                colVec.push_back(glm::vec4(1, 1, 1, 1)); // white
            } else {
                colVec.push_back(glm::vec4(1, 0, 0, 1)); // red
            }

            idxVec.push_back(curIdx + i);
            if (i == N-1) { // last vertex of the circle
                idxVec.push_back(curIdx);
            } else {
                idxVec.push_back(curIdx + i + 1);
            }
            vert = glm::rotate(vert, glm::radians(360.f / (float) N), X_axis);

        }
        curIdx += N;

        // circle acrossed by Y axis
        vert = glm::vec4(radius * Z_axis, 1);
        for (int i=0; i < N; i++) {
            posVec.push_back(joint->getOverallTransformation() * vert);

            if (joint.get() == this->representedJoint) { // if selected, then high-lighte it
                colVec.push_back(glm::vec4(1, 1, 1, 1)); // white
            } else {
                colVec.push_back(glm::vec4(0, 1, 0, 1)); // green
            }

            idxVec.push_back(curIdx + i);
            if (i == N-1) { // last vertex of the circle
                idxVec.push_back(curIdx);
            } else {
                idxVec.push_back(curIdx + i + 1);
            }
            vert = glm::rotate(vert, glm::radians(360.f / (float) N), Y_axis);

        }
        curIdx += N;

        // circle acrossed by Z axis
        vert = glm::vec4(radius * X_axis, 1);
        for (int i=0; i < N; i++) {
            posVec.push_back(joint->getOverallTransformation() * vert);

            if (joint.get() == this->representedJoint) { // if selected, then high-lighte it
                colVec.push_back(glm::vec4(1, 1, 1, 1)); // white
            } else {
                colVec.push_back(glm::vec4(0, 0, 1, 1)); // blue
            }

            idxVec.push_back(curIdx + i);
            if (i == N-1) { // last vertex of the circle
                idxVec.push_back(curIdx);
            } else {
                idxVec.push_back(curIdx + i + 1);
            }
            vert = glm::rotate(vert, glm::radians(360.f / (float) N), Z_axis);

        }
        curIdx += N;

        // draw the bone
        // indicate the direction of the relationship
        // magenta on the parent's end, yellow at the child's end
        if (joint->parent) {
            // current joint
            posVec.push_back(joint->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
            colVec.push_back(glm::vec4(1, 1, 0, 1)); // yellow
            idxVec.push_back(curIdx);

            // parent
            posVec.push_back(joint->parent->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
            colVec.push_back(glm::vec4(1, 0, 1, 1)); // magenta
            idxVec.push_back(curIdx+1);
            curIdx += 2;
        }
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
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, colVec.size() * sizeof(glm::vec4), colVec.data(), GL_STATIC_DRAW);
}

void Skeleton::updateJoint(Joint *joint) {
    this->representedJoint = joint;
}

GLenum Skeleton::drawMode() {
    return GL_LINES;
}

std::vector<glm::mat4> Skeleton::getTransformations() {
    std::vector<glm::mat4> transformations;
    for (uPtr<Joint> &joint : joints) {
        transformations.push_back(joint->getOverallTransformation());
    }
    return transformations;
}
