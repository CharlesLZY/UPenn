#ifndef FACEDISPLAY_H
#define FACEDISPLAY_H

#include "drawable.h"
#include "face.h"

class FaceDisplay : public Drawable {
protected:
    Face *representedFace;

public:
    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void create() override;
    // Change which Face representedFace points to
    void updateFace(Face* face);

    GLenum drawMode() override; // return GL_LINES

    FaceDisplay(OpenGLContext* context);
};

#endif // FACEDISPLAY_H
