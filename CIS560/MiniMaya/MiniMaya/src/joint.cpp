#include "joint.h"

int Joint::lastJoint = 0;

Joint::Joint()
     : QTreeWidgetItem(),
       name(),
       parent(nullptr),
       children(),
       pos(glm::vec4()),
       rot(glm::quat()),
       bindMatrix(glm::mat4()),
       ID(lastJoint++)
{
    // refer to hw2
    this->setText(0, this->name);
}

Joint::Joint(QString name, glm::vec3 pos, glm::vec4 rot)
     : QTreeWidgetItem(),
       name(name),
       parent(nullptr),
       children(),
       pos(pos),
       bindMatrix(glm::mat4()),
       ID(lastJoint++)
{
    // refer to the slide "Skeletons, Skinning, and Quaternions" P22
    // q = [cos(theta/2), sin(theta/2)v_x, sin(theta/2)v_y, sin(theta/2)v_z]
    // where (v_x, v_y, v_z) is a unit vector
    this->rot = glm::quat(cos(glm::radians(rot[0] / 2.f)),
                          sin(glm::radians(rot[0] / 2.f)) * rot[1],
                          sin(glm::radians(rot[0] / 2.f)) * rot[2],
                          sin(glm::radians(rot[0] / 2.f)) * rot[3]);

    this->setText(0, this->name);
}

glm::mat4 Joint::getLocalTransformation() {
    // translation * rotation
    return glm::translate(glm::mat4(1.f), this->pos) * glm::toMat4(this->rot);
}

glm::mat4 Joint::getOverallTransformation() {
    // chain transformation
    if (this->parent) {
        return this->parent->getOverallTransformation() * this->getLocalTransformation();
    }
    else {
        return this->getLocalTransformation();
    }
}
