#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <glm/glm.hpp>
#include <QListWidget>
#include "vertex.h"
#include "face.h"


class Vertex;
class Face;


class HalfEdge : public QListWidgetItem
{
public:

    //  a static member variable that tracks the last ID assigned to a component,
    //  and increments that value by one each time a new half-edge is created
    static int lastHalfEdge;


    HalfEdge();

    // A pointer to the next HalfEdge in the loop of HalfEdges that lie on this HalfEdge's Face
    HalfEdge* next;
    // A pointer to the HalfEdge that lies parallel to this HalfEdge and which travels in the opposite direction and is part of an adjacent Face, i.e. this HalfEdge's symmetrical HalfEdge
    HalfEdge* SYM;
    // A pointer to the Face on which this HalfEdge lies
    Face* face;
    // A pointer to the Vertex between this HalfEdge and its next HalfEdge
    Vertex* vert;
    // A unique integer to identify the HalfEdge in menus and while debugging
    int ID;

};

#endif // HALFEDGE_H
