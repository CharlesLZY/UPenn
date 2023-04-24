#ifndef MESH_H
#define MESH_H

#include "smartpointerhelp.h"
#include "drawable.h"
#include "vertex.h"
#include "face.h"
#include "halfedge.h"
#include "joint.h"

#include <unordered_map>
#include <unordered_set>

class Mesh : public Drawable
{
public:
    Mesh(OpenGLContext* context);

    std::vector<uPtr<Vertex>> verts;
    std::vector<uPtr<Face>> faces;
    std::vector<uPtr<HalfEdge>> edges;

    std::vector<uPtr<Joint>> joints;

    // setting up the VBOs(vertex buffer object）
    void create() override;
    GLenum drawMode() override; // return GL_TRIANGLES

    void buildCube();

    void splitEdge(HalfEdge *HE1);
    void triangulate(Face *FACE1);



    // Catmull-Clark subdivision
    void subdivide();

    void computeCentroid(Face *face, std::unordered_map<int, Vertex*> &centroidMap);

    void computeMidpoint(HalfEdge* edge, std::unordered_map<int, Vertex*> &centroidMap);

    void smoothVertex(Vertex *v, std::unordered_map<int, Vertex*> &centroidMap);

    void quadrangulate(Face *face, Vertex *centroid, std::vector<HalfEdge*> &prevEdges, std::vector<HalfEdge*> &newEdges);


};

#endif // MESH_H
