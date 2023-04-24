#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <QString>
#include <QImage>
#include <QColor>

// A Vertex is a point in space that defines one corner of a polygon.
// Each Vertex has several attributes that determine how they contribute to the
// appearance of their Polygon, such as coloration.
struct Vertex
{
    glm::vec4 m_pos;    // The position of the vertex. In hw02, this is in pixel space.
    glm::vec3 m_color;  // The color of the vertex. X corresponds to Red, Y corresponds to Green, and Z corresponds to Blue.
    glm::vec4 m_normal; // The surface normal of the vertex (not yet used)
    glm::vec2 m_uv;     // The texture coordinates of the vertex (not yet used)

    Vertex(glm::vec4 p, glm::vec3 c, glm::vec4 n, glm::vec2 u)
        : m_pos(p), m_color(c), m_normal(n), m_uv(u)
    {}
};

// Each Polygon can be decomposed into triangles that fill its area.
struct Triangle
{
    // The indices of the Vertices that make up this triangle.
    // The indices correspond to the std::vector of Vertices stored in the Polygon
    // which stores this Triangle
    unsigned int m_indices[3];
};

// Each Segment instance will represent one edge of a triangle
class Segment
{
public:
    // Two vectors representing the segment's endpoints
    glm::vec4 p1; // same as Vertex.m_pos
    glm::vec4 p2; // same as Vertex.m_pos
    // The slope of the line segment
    float dX;
    float dY;
    // A constructor that takes in both endpoints of the line segment
    Segment(glm::vec4 p1, glm::vec4 p2);
    // A function that computes the x-intersection of the line segment with a horizontal line
    // based on the horizontal line's y-coordinate
    bool getIntersection(int y, float *x);
};

class Polygon
{
public:
    // TODO: Populate this list of triangles in Triangulate()
    std::vector<Triangle> m_tris;
    // The list of Vertices that define this polygon. This is already filled by the Polygon constructor.
    std::vector<Vertex> m_verts;
    // Vertices transformed to screen space
    std::vector<Vertex> m_screen_verts;

    // The name of this polygon, primarily to help you debug
    QString m_name;
    // The image that can be read to determine pixel color when used in conjunction with UV coordinates
    // Not used until homework 3.
    QImage* mp_texture;
    // The image that can be read to determine surface normal offset when used in conjunction with UV coordinates
    // Not used until homework 3
    QImage* mp_normalMap;

    // Polygon class constructors
    Polygon(const QString& name, const std::vector<glm::vec4>& pos, const std::vector<glm::vec3> &col);
    Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale);
    Polygon(const QString& name);
    Polygon();
    Polygon(const Polygon& p);
    ~Polygon();

    // TODO: Complete the body of Triangulate() in polygon.cpp
    // Creates a set of triangles that, when combined, fill the area of this convex polygon.
    void Triangulate();

    // Compute the 2D axis-aligned bounding box of a triangle
    std::array<float, 4> getBoundingBox(Triangle tri);

    // Get the edges of a triangle
    std::array<Segment, 3> getEdges(Triangle tri);

    // Barycentric interpolation
    glm::vec3 BarycentricInterpolationWeight(Triangle tri, float x, float y);
    float BarycentricZ(Triangle tri, float x, float y);
    glm::vec3 BarycentricColor(Triangle tri, float x, float y);
    glm::vec2 BarycentricUV(Triangle tri, float x, float y);
    glm::vec4 BarycentricNorm(Triangle tri, float x, float y);

    // Get the world coordinate of a point within the triangle
    glm::vec3 BarycentricCoordinate(Triangle tri, float x, float y);

    // Transform from world space to screen coordinates
    void transform(glm::mat4 T, float width, float height, bool rendering2D);

    // Copies the input QImage into this Polygon's texture
    void SetTexture(QImage*);

    // Copies the input QImage into this Polygon's normal map
    void SetNormalMap(QImage*);

    // Various getter, setter, and adder functions
    void AddVertex(const Vertex&);
    void AddTriangle(const Triangle&);
    void ClearTriangles();

    Triangle& TriAt(unsigned int);
    Triangle TriAt(unsigned int) const;

    Vertex& VertAt(unsigned int);
    Vertex VertAt(unsigned int) const;
};

// Returns the color of the pixel in the image at the specified texture coordinates.
// Returns white if the image is a null pointer
glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image);

float Area(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
