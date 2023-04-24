#ifndef SKELETON_H
#define SKELETON_H

#include "joint.h"
#include "drawable.h"
#include "smartpointerhelp.h"

class Skeleton : public Drawable
{
public:
    Joint *representedJoint;
    std::vector<uPtr<Joint>> joints;

    // Creates VBO data to make a visual
    // representation of the skeleton
    void create() override;
    // Change which Joint representedJoint points to
    void updateJoint(Joint *joint);

    // Get all transformation matrixs
    std::vector<glm::mat4> getTransformations();

    GLenum drawMode() override; // return GL_LINES;

    Skeleton(OpenGLContext* context);
};

#endif // SKELETON_H
