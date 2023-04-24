#pragma once

#include <openGL/drawable.h>

//A cube is assumed to have side lengths of 1 and a center of <0,0,0>. This means all vertices are of the form <+/-0.5, +/-0.5, +/-0.5>
//These attributes can be altered by applying a transformation matrix to the cube.
class Cube : public Drawable
{
public:
    void create();
};
