#pragma once
#include "drawable.h"

class Polygon2D : public Drawable
{
public:
    // Instantiate an empty Polygon
    Polygon2D(OpenGLContext* context);
    // Instantiate a regular polygon with N sides and
    // a bounding box of side length 1 centered at the origin
    Polygon2D(OpenGLContext* context, int numSides);
    // Instantiate a polygon with its vertex positions defined
    // in counter-clockwise order. These vertices must form a convex
    // polygon in order to be drawn correctly.
    Polygon2D(OpenGLContext* context, const std::vector<glm::vec3>& positions);
    // Initialize data required by OpenGL to render the shape
    void create() override;
    // Set the color of the polygon when it's drawn by OpenGL
    void setColor(glm::vec3 c);

protected:
    // The list of vertex positions that define this polygon's shape
    std::vector<glm::vec3> m_vertPos;
    // The order in which vertices should be read to assemble triangles
    // that, all together, form the polygon.
    std::vector<GLuint> m_vertIdx;
    // How many vertices compose this Polygon. Read by setColor
    // in order to know how many vertices need to be assigned the
    // Polygon's color.
    unsigned int m_numVertices;
};
