#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>
#include <QListWidget>
#include <halfedge.h>


class HalfEdge;



class Vertex : public QListWidgetItem
{
public:

    //  a static member variable that tracks the last ID assigned to a component,
    //  and increments that value by one each time a new vertex is created
    static int lastVertex;

    Vertex();

    // A vec3 for storing its position
    glm::vec3 pos;
    // A pointer to one of the HalfEdges that points to this Vertex
    HalfEdge* edge;
    // A unique integer to identify the Vertex in menus and while debugging
    int ID;

    // Vertex's influencer IDs and weight
    std::vector<std::pair<int, float>> joints;

};

#endif // VERTEX_H
