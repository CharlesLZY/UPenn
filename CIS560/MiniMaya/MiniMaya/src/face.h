#ifndef FACE_H
#define FACE_H

#include <glm/glm.hpp>
#include <QListWidgetItem>
#include "halfedge.h"

class HalfEdge;

class Face : public QListWidgetItem
{
public:

    //  a static member variable that tracks the last ID assigned to a component,
    //  and increments that value by one each time a new face is created
    static int lastFace;

    Face();
    // A vec3 to represent this Face's color as an RGB value
    glm::vec3 color;
    // A pointer to one of the HalfEdges that lies on this Face
    HalfEdge* edge;
    // A unique integer to identify the Face in menus and while debugging
    int ID;

};

#endif // FACE_H
