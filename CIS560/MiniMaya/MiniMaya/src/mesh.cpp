#include "mesh.h"
#include <iostream> // for debug

Mesh::Mesh(OpenGLContext* context)
    : Drawable(context),
      verts(std::vector<uPtr<Vertex>>{}),
      faces(std::vector<uPtr<Face>>{}),
      edges(std::vector<uPtr<HalfEdge>>{}),
      joints(std::vector<uPtr<Joint>>{})
{}

void Mesh::create() {
    std::vector<GLuint> idxVec;
    std::vector<glm::vec4> posVec;
    std::vector<glm::vec4> colorVec;
    std::vector<glm::vec4> normVec;

    std::vector<glm::ivec2> jointVec;
    std::vector<glm::vec2> weightVec;

    // set up VBO data per-face
    int curIdx = 0; // index of vertex
    for (uPtr<Face> &face : this->faces) {
        HalfEdge *curEdge = face->edge;
        int edgeNum = 0; // # vertex = # edge
        do {
            posVec.push_back(glm::vec4(curEdge->vert->pos, 1));
            colorVec.push_back(glm::vec4(face->color, 1));
            /*       B
                    /
                   /
                  /______
                 A       C

                 AB = OB - OA
                 AC = OC - OA
            */
            glm::vec3 AB = curEdge->next->vert->pos - curEdge->vert->pos;
            glm::vec3 AC = curEdge->next->next->vert->pos - curEdge->next->vert->pos;
            // AB X AC is perpendicular of the surface on which AB and AC are
            normVec.push_back(glm::vec4(glm::normalize(glm::cross(AB, AC)), 0));

            // if the mesh has been bound
            if (curEdge->vert->joints.size() > 0) {
                // joint ID
                jointVec.push_back(glm::ivec2(curEdge->vert->joints[0].first, curEdge->vert->joints[1].first));
                // weight
                weightVec.push_back(glm::vec2(curEdge->vert->joints[0].second, curEdge->vert->joints[1].second));
            }

            curEdge = curEdge->next;
            edgeNum += 1;

        } while (curEdge != face->edge);

        // refer to hw3
        for (int i=0; i<edgeNum-2; i++) {
            idxVec.push_back(curIdx);
            idxVec.push_back(curIdx+i+1);
            idxVec.push_back(curIdx+i+2);
        }

        curIdx += edgeNum;
    }
    // The number of indices stored in bufIdx
    this->count = idxVec.size();

    // refer to the instruction on hw5 website
    // inherit from Drawable class
    this->generateIdx();
    this->bindIdx();
    // refer to https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glBufferData.xhtml
    this->mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxVec.size() * sizeof(GLuint), idxVec.data(), GL_STATIC_DRAW);

    this->generatePos();
    this->bindPos();
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, posVec.size() * sizeof(glm::vec4), posVec.data(), GL_STATIC_DRAW);

    this->generateCol();
    this->bindCol();
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, colorVec.size() * sizeof(glm::vec4), colorVec.data(), GL_STATIC_DRAW);

    this->generateNor();
    this->bindNor();
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, normVec.size() * sizeof(glm::vec4), normVec.data(), GL_STATIC_DRAW);


    this->generateJoint();
    this->bindJoint();
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, jointVec.size() * sizeof(glm::ivec2), jointVec.data(), GL_STATIC_DRAW);

    this->generateJointWeight();
    this->bindJointWeight();
    this->mp_context->glBufferData(GL_ARRAY_BUFFER, weightVec.size() * sizeof(glm::vec2), weightVec.data(), GL_STATIC_DRAW);
}



GLenum Mesh::drawMode() {
    return GL_TRIANGLES;
}

// Refer to Cube.png
void Mesh::buildCube() {

    // clear previous data
    this->verts.clear();
    this->edges.clear();
    this->faces.clear();

    // eight vertices
    glm::vec3 vertexPos[8] = {glm::vec3( 0.5,  0.5,  0.5),
                              glm::vec3( 0.5,  0.5, -0.5),
                              glm::vec3(-0.5,  0.5, -0.5),
                              glm::vec3(-0.5,  0.5,  0.5),
                              glm::vec3( 0.5, -0.5,  0.5),
                              glm::vec3( 0.5, -0.5, -0.5),
                              glm::vec3(-0.5, -0.5, -0.5),
                              glm::vec3(-0.5, -0.5,  0.5)};



    // six surfaces
    glm::vec3 colors[6] = {glm::vec3(1.f), // top
                           glm::vec3(1.f), // right
                           glm::vec3(0.f), // left
                           glm::vec3(1.f), // front
                           glm::vec3(0.f), // back
                           glm::vec3(0.f)}; // bottom

    // initialize eight vertices
    for (int i=0; i < 8; i++) {
        uPtr<Vertex> vertex = mkU<Vertex>();
        vertex->pos = vertexPos[i];
        this->verts.push_back(std::move(vertex));
    }
    // initialize six surfaces
    for (int i=0; i < 6; i++) {
        uPtr<Face> face = mkU<Face>();
        face->color = colors[i];
        this->faces.push_back(std::move(face));
    }

    // initialize 2 * 12 half edges
    for (int i=0; i < 24; i++) {
        uPtr<HalfEdge> edge = mkU<HalfEdge>();
        this->edges.push_back(std::move(edge));
    }

    // set half edge for six surfaces
    for (int i=0; i < 6; i++) {
        this->faces[i]->edge = this->edges[i*4].get();
        for (int j=0; j<4; j++) {
            this->edges[4*i + j]->face = this->faces[i].get();
            this->edges[4*i + j]->next = this->edges[4*i + (j+1)%4].get();
        }
    }
    this->verts[0]->edge = this->edges[0].get();
    this->verts[1]->edge = this->edges[1].get();
    this->verts[2]->edge = this->edges[2].get();
    this->verts[3]->edge = this->edges[3].get();
    this->verts[4]->edge = this->edges[4].get();
    this->verts[5]->edge = this->edges[5].get();
    this->verts[6]->edge = this->edges[10].get();
    this->verts[7]->edge = this->edges[15].get();

    this->edges[0]->vert = this->verts[0].get();
    this->edges[1]->vert = this->verts[1].get();
    this->edges[2]->vert = this->verts[2].get();
    this->edges[3]->vert = this->verts[3].get();
    this->edges[4]->vert = this->verts[4].get();
    this->edges[5]->vert = this->verts[5].get();
    this->edges[6]->vert = this->verts[1].get();
    this->edges[7]->vert = this->verts[0].get();
    this->edges[8]->vert = this->verts[3].get();
    this->edges[9]->vert = this->verts[2].get();
    this->edges[10]->vert = this->verts[6].get();
    this->edges[11]->vert = this->verts[7].get();
    this->edges[12]->vert = this->verts[4].get();
    this->edges[13]->vert = this->verts[0].get();
    this->edges[14]->vert = this->verts[3].get();
    this->edges[15]->vert = this->verts[7].get();
    this->edges[16]->vert = this->verts[1].get();
    this->edges[17]->vert = this->verts[5].get();
    this->edges[18]->vert = this->verts[6].get();
    this->edges[19]->vert = this->verts[2].get();
    this->edges[20]->vert = this->verts[5].get();
    this->edges[21]->vert = this->verts[4].get();
    this->edges[22]->vert = this->verts[7].get();
    this->edges[23]->vert = this->verts[6].get();

    this->edges[0]->SYM = this->edges[14].get();
    this->edges[1]->SYM = this->edges[7].get();
    this->edges[2]->SYM = this->edges[16].get();
    this->edges[3]->SYM = this->edges[9].get();
    this->edges[4]->SYM = this->edges[13].get();
    this->edges[5]->SYM = this->edges[21].get();
    this->edges[6]->SYM = this->edges[17].get();
    this->edges[7]->SYM = this->edges[1].get();
    this->edges[8]->SYM = this->edges[15].get();
    this->edges[9]->SYM = this->edges[3].get();
    this->edges[10]->SYM = this->edges[19].get();
    this->edges[11]->SYM = this->edges[23].get();
    this->edges[12]->SYM = this->edges[22].get();
    this->edges[13]->SYM = this->edges[4].get();
    this->edges[14]->SYM = this->edges[0].get();
    this->edges[15]->SYM = this->edges[8].get();
    this->edges[16]->SYM = this->edges[2].get();
    this->edges[17]->SYM = this->edges[6].get();
    this->edges[18]->SYM = this->edges[20].get();
    this->edges[19]->SYM = this->edges[10].get();
    this->edges[20]->SYM = this->edges[18].get();
    this->edges[21]->SYM = this->edges[5].get();
    this->edges[22]->SYM = this->edges[12].get();
    this->edges[23]->SYM = this->edges[11].get();

}

// Refer to the lecture slide 'Mesh Data Structure' P19~26
void Mesh::splitEdge(HalfEdge *HE1) {
    HalfEdge* HE2 = HE1->SYM;
    /*
    Step 1: Create the new vertex V3
    V3’s pos is the average of V1’s and V2’s
    */
    uPtr<Vertex> V3 = mkU<Vertex>();
    Vertex* V1 = HE1->vert;
    Vertex* V2 = HE2->vert;
    V3->pos = (V1->pos + V2->pos) / 2.f;

    /*
    Step 2： Create the two new half-edges HE1B and HE2B needed to surround V3
    Step 2A: Make sure that HE1B points to V1 and that HE2B points to V2
    Stip 2B: Also set the correct face pointers for HE1B and HE2B
    */
    uPtr<HalfEdge> HE1B = mkU<HalfEdge>();
    uPtr<HalfEdge> HE2B = mkU<HalfEdge>();
    HE1B->vert = V1;
    HE2B->vert = V2;
    V1->edge = HE1B.get();
    V2->edge = HE2B.get();

    HE1B->face = HE1->face;
    HE2B->face = HE2->face;

    /*
    Step 3: Adjust the SYM, next, and vert pointers of HE1, HE2, HE1B, and HE2B so the data structure flows as it did before
    */
    HE1->SYM = HE2B.get();
    HE2B->SYM = HE1;
    HE2->SYM = HE1B.get();
    HE1B->SYM = HE2;

    HE1B->next = HE1->next;
    HE2B->next = HE2->next;
    HE1->next = HE1B.get();
    HE2->next = HE2B.get();

    HE1->vert->edge = HE1B.get();
    HE2->vert->edge = HE2B.get();
    V3->edge = HE1;

    HE1->vert = V3.get();
    HE2->vert = V3.get();

    this->verts.push_back(std::move(V3));
    this->edges.push_back(std::move(HE1B));
    this->edges.push_back(std::move(HE2B));
}

// Refer to the lecture slide 'Mesh Data Structure' P28~30
void Mesh::triangulate(Face *FACE1) {
    HalfEdge *HE_0 = FACE1->edge;

    int vertNum = 0;
        do {
            vertNum += 1;
            HE_0 = HE_0->next;
        } while (HE_0 != FACE1->edge);

    for (int i=0; i < vertNum - 3; i++) {

        /*
        Step 1: Create two new half-edges HE_A and HE_B
        HE_A points to HE_0.vert
        HE_B points to HE_0.next.next.vert
        HE_A and HE_B have each other for syms
        */
        uPtr<HalfEdge> HE_A = mkU<HalfEdge>();
        uPtr<HalfEdge> HE_B = mkU<HalfEdge>();
        HE_A->vert = HE_0->vert;
        HE_B->vert = HE_0->next->next->vert;
        HE_A->SYM = HE_B.get();
        HE_B->SYM = HE_A.get();

        /*
        Step 2: Create a second face FACE2
        HE_A, HE_0.next, and HE_0.next.next now all point to FACE2
        HE_B points to FACE1
        FACE2’s arbitrary half-edge pointer can be HE_A
        */
        uPtr<Face> FACE2 = mkU<Face>();
        HE_A->face = FACE2.get();
        HE_0->next->face = FACE2.get();
        HE_0->next->next->face = FACE2.get();
        HE_B->face = FACE1;
        FACE2->edge = HE_A.get();

        /*
        Step 3: Fix up tHE next pointers for our half-edges
        HE_B.next = HE_0.next.next.next
        HE_0.next.next.next = HE_A
        HE_A.next = HE_0.next
        HE_0.next = HE_B
        */
        HE_B->next = HE_0->next->next->next;
        HE_0->next->next->next = HE_A.get();
        HE_A->next = HE_0->next;
        HE_0->next = HE_B.get();

        this->faces.push_back(std::move(FACE2));
        this->edges.push_back(std::move(HE_A));
        this->edges.push_back(std::move(HE_B));
    }
}




// Refer to the slide "Subdivision Techniques" P11
// Compute the centroid of a face
void Mesh::computeCentroid(Face *face, std::unordered_map<int, Vertex*> &centroidMap) {
    // Centroid = average of all vertices
    uPtr<Vertex> centroid = mkU<Vertex>();

    HalfEdge *curEdge = face->edge;
    int edgeNum = 0;
    do {
        centroid->pos += curEdge->vert->pos;
        edgeNum++;

        curEdge = curEdge->next;
    } while (curEdge != face->edge);
    centroid->pos /= (float) edgeNum;

    centroidMap.insert({face->ID, centroid.get()});

    this->verts.push_back(std::move(centroid));
}

// Refer to the slide "Subdivision Techniques" P12
// Compute the smoothed midpoint of an edge
void Mesh::computeMidpoint(HalfEdge* edge, std::unordered_map<int, Vertex*> &centroidMap) {
    HalfEdge *edgeSYM = edge->SYM;

    Vertex *v1 = edge->vert;
    Vertex *v2 = edgeSYM->vert;

    Face *face1 = edge->face;
    Face *face2 = edgeSYM->face;
    Vertex *f1 = centroidMap[face1->ID];
    Vertex *f2 = centroidMap[face2->ID];

    // Create the smoothed midpoint
    uPtr<Vertex> midpoint = mkU<Vertex>(); // e

    // link to current edge
    midpoint->edge = edge;
    // average the pos
    if (face2) {
        // e = (v1 + v2 + f1 + f2) / 4
        midpoint = mkU<Vertex>();
        midpoint->pos = (v1->pos + v2->pos + f1->pos + f2->pos) / 4.f;
    }
    // If only one face is incident to the edge
    else {
        // e = (v1 + v2 + f) / 3
        midpoint = mkU<Vertex>();
        midpoint->pos = (v1->pos + v2->pos + f1->pos) / 3.f;
    }

    // Similar to splitEdge()
    // Refer to the lecture slide 'Mesh Data Structure' P19~26
    // V3 = midpoint
    // HE1 = edge
    // HE2 = edgeSYM
    // HE1B = newEdge1
    // HE2B = newEdge2

    /*
    Create the two new half-edges HE1B and HE2B needed to surround V3
    Step A: Make sure that HE1B points to V1 and that HE2B points to V2
    Stip B: Also set the correct face pointers for HE1B and HE2B
    */
    uPtr<HalfEdge> newEdge1 = mkU<HalfEdge>();
    uPtr<HalfEdge> newEdge2 = mkU<HalfEdge>();

    newEdge1->vert = v1; // HE1B->vert = V1
    newEdge2->vert = v2; // HE2B->vert = V2
    v1->edge = newEdge1.get(); // V1->edge = HE1B
    v2->edge = newEdge2.get(); // V2->edge = HE2B

    newEdge1->face = edge->face;    // HE1B->face = HE1->face
    newEdge2->face = edgeSYM->face; // HE2B->face = HE2->face

    /*
    Adjust the SYM, next, and vert pointers of HE1, HE2, HE1B, and HE2B
    so the data structure flows as it did before
    */

    edge->SYM = newEdge2.get();    // HE1->SYM = HE2B
    newEdge2->SYM = edge;          // HE2B->SYM = HE1
    edgeSYM->SYM = newEdge1.get(); // HE2->SYM = HE1B
    newEdge1->SYM = edgeSYM;       // HE1B->SYM = HE2

    // prevEdge -> newEdge -> prevEdge -> newEdge
    /*
       o —————>o        o——>o——>o
       |       |        |       |
       |       |   ->   o       o
       o ————— o        |       |
                        o<——o<——o
    */
    newEdge1->next = edge->next;    // HE1B->next = HE1->next
    newEdge2->next = edgeSYM->next; // HE2B->next = HE2->next
    edge->next = newEdge1.get();    // H1->next = HE1B->next
    edgeSYM->next = newEdge2.get(); // H2->next = HE2B->next

    edge->vert = midpoint.get();    // HE1->vert = V3
    edgeSYM->vert = midpoint.get(); // HE2->vert = V3

    this->verts.push_back(std::move(midpoint));
    this->edges.push_back(std::move(newEdge1));
    this->edges.push_back(std::move(newEdge2));

}


// Refer to the slide "Subdivision Techniques" P13
// Smooth the original vertices
void Mesh::smoothVertex(Vertex *v, std::unordered_map<int, Vertex*> &centroidMap) {
    // v' = (n-2)v/n + sum(e)/n^2 + sum(f)/n^2
    // Where v is the vertex's original pos
    // sum(e) is the sum of all adjacent midpoints
    // sum(f) is the sum of the centroids of all faces incident to v
    // n is the number of adjacent midpoints
    int n = 0; // n
    glm::vec3 centroids = glm::vec3(0.f); // sum(e)
    glm::vec3 midpoints = glm::vec3(0.f); // sum(f)

    HalfEdge *curEdge = v->edge;
    do {
        centroids += centroidMap[curEdge->face->ID]->pos;
        midpoints += curEdge->SYM->vert->pos; // vertex has been linked to midpoint in step 2

        n += 1;
        curEdge = curEdge->next->SYM;
    } while (curEdge != v->edge);

    // v' = (n-2)v/n + sum(e)/n^2 + sum(f)/n^2
    v->pos = (float) (n - 2) * v->pos / (float) n + centroids / (float) (n * n) + midpoints / (float) (n * n);
}

// Refer to the slide "Subdivision Techniques" P14
// Split a face into N quadrangle faces
void Mesh::quadrangulate(Face* face, Vertex *centroid, std::vector<HalfEdge*> &prevEdges, std::vector<HalfEdge*> &newEdges) {
    // number of vertices that the face originally had
    int N = prevEdges.size(); // # vertex = # edge

    std::vector<uPtr<HalfEdge>> edgesToCentroid; // towards to centroid
    std::vector<uPtr<HalfEdge>> edgesToMidpoint; // towards to midpoint

    for (int i=0; i < N; i++) {
        HalfEdge *prevEdge = prevEdges[i];
        HalfEdge *newEdge = newEdges[i];

        uPtr<HalfEdge>  inEdge = mkU<HalfEdge>(); // towards to centroid
        uPtr<HalfEdge> outEdge = mkU<HalfEdge>(); // towards to midpoint

        /*
           o——>o——>o         o——>o——>o
           |       |         |   |   |
           o       o   ->    o———o———o
           |       |         |   |   |
           o<——o<——o         o<——o<——o
        */

        newEdge->next = inEdge.get();
        inEdge->next = outEdge.get();
        outEdge->next = prevEdge;

        inEdge->vert = centroid;
        outEdge->vert = prevEdge->SYM->vert;

        if (i == 0) { // the original face
            centroid->edge = inEdge.get();
            face->edge = outEdge.get();

            inEdge->face = face;
            outEdge->face = face;
        } else {
            uPtr<Face> newFace = mkU<Face>(); // N-1 new faces in total
            // assign a random color
            newFace->color = glm::vec3(rand() / (float) RAND_MAX);
            newFace->edge = inEdge.get();

            inEdge->face = newFace.get();
            outEdge->face = newFace.get();
            newEdge->face = newFace.get();
            prevEdge->face = newFace.get();

            this->faces.push_back(std::move(newFace));
        }

        edgesToCentroid.push_back(std::move(inEdge));
        edgesToMidpoint.push_back(std::move(outEdge));

    }


    for (int i=0; i < N; i++) {
        /*
           o——>o——>o
           |   |   |
           o———o———o
           |   |   |
           o<——o<——o
        */
        // inEdges[i]->SYM = outEdges[i+1]
        edgesToCentroid[i]->SYM = edgesToMidpoint[(i + 1) % N].get();
        edgesToMidpoint[(i + 1) % N]->SYM = edgesToCentroid[i].get();

        this->edges.push_back(std::move(edgesToCentroid[i]));
        this->edges.push_back(std::move(edgesToMidpoint[(i + 1) % N]));
    }
}



void Mesh::subdivide() {
    // previous state
    int prevVertNum = this->verts.size();
    int prevFaceNum = this->faces.size();
    int prevEdgeNum = this->edges.size();

    // storing centroids in the way that we can access a centroid given its corresponding face
    std::unordered_map<int, Vertex*> centroidMap; // faceID - centroid

    /*
     Step 1: For each Face, compute its centroid
    */
    for (int i=0; i < prevFaceNum; i++) {
        Face *face = this->faces[i].get();
        // Compute centroid
        this->computeCentroid(face, centroidMap);

    }


    /*
     Step 2: Compute the smoothed midpoint of each edge in the mesh
    */
    std::unordered_set<int> visited;

    for (int i=0; i < prevEdgeNum; i++) {
        HalfEdge *edge = this->edges[i].get();
        // refer to https://www.cplusplus.com/reference/unordered_set/unordered_set/find/
        if (visited.find(edge->ID) == visited.end()) {
            // Only split each full edge once
            visited.insert(edge->ID);
            visited.insert(edge->SYM->ID); // an edge and its SYM correspond to the same edge

            this->computeMidpoint(edge, centroidMap);
        }
    }


    /*
     Step 3: Smooth the original vertices
    */
    for (int i=0; i < prevVertNum; i++) {
        Vertex *vert = this->verts[i].get();

        this->smoothVertex(vert, centroidMap);
    }


    /*
     Step 4: For each original face, split that face into N quadrangle faces
    */
    int num_ori_faces = this->faces.size();
    for (int i = 0; i < num_ori_faces; i++) {
        Face *face = this->faces[i].get();
        Vertex *centroid = centroidMap[face->ID];

        std::vector<HalfEdge*> prevEdges;
        std::vector<HalfEdge*> newEdges; // the set of half-edges that point to the midpoints on that face

        HalfEdge *curEdge = face->edge->next;

        do {
            // after splitting the original edges, the link order will be like this:
            // prevEdge -> newEdge -> prevEdge -> newEdge
            /*
               o —————>o        o——>o——>o
               |       |        |       |
               |       |   ->   o       o
               o ————— o        |       |
                                o<——o<——o
            */
            prevEdges.push_back(curEdge);
            newEdges.push_back(curEdge->next);
            curEdge = curEdge->next->next;
        } while (curEdge != face->edge->next);

        this->quadrangulate(face, centroid, prevEdges, newEdges);
    }

}










