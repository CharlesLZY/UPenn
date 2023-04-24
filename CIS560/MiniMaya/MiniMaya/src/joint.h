#ifndef JOINT_H
#define JOINT_H

#include <QTreeWidget>
#include "la.h"

// follow the instruction on the hw7 website
class Joint : public QTreeWidgetItem
{
public:
    static int lastJoint;

    // The name of this joint which will be displayed in your QTreeWidget of joints.
    QString name;
    // The joint that is the parent of this joint.
    Joint *parent;
    // The set of joints that have this joint as their parent.
    std::vector<Joint*> children;
    // The position of this joint relative to its parent joint.
    glm::vec3 pos;
    // The quaternion that represents this joint's current orientation.
    glm::quat rot;
    // The inverse of the joint's compound transformation matrix at the time a mesh is bound to the joint's skeleton.
    glm::mat4 bindMatrix;
    int ID;

    // A function which returns a mat4 that represents the concatenation of a joint's position and rotation
    glm::mat4 getLocalTransformation();

    // A function which returns a mat4 that represents the concatentation of this joint's local transformation with the transformations of its chain of parent joints
    glm::mat4 getOverallTransformation();

    Joint();
    Joint(QString name, glm::vec3 pos, glm::vec4 rot);
};

#endif // JOINT_H
