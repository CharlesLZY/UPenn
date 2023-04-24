#ifndef HALFEDGEDISPLAY_H
#define HALFEDGEDISPLAY_H

#include "drawable.h"
#include "halfedge.h"

class HalfEdgeDisplay : public Drawable {
protected:
    HalfEdge *representedEdge;

public:
    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void create() override;
    // Change which HalfEdge representedEdge points to
    void updateEdge(HalfEdge *edge);

    GLenum drawMode() override; // return GL_LINES

    HalfEdgeDisplay(OpenGLContext *context);
};

#endif // HALFEDGEDISPLAY_H
