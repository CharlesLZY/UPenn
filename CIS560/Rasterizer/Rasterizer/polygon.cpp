#include "polygon.h"
#include <glm/gtx/transform.hpp>

Segment::Segment(glm::vec4 p1, glm::vec4 p2)
    : p1(p1), p2(p2), dX(p2.x-p1.x), dY(p2.y-p1.y)
{}

// This function will take an X-coordinate allocated to stack
bool Segment::getIntersection(int y, float *x) {
    // beyond the bounding box
    // We only need to check whether Y coords are within the bounding box
    if (y > std::max(p1.y, p2.y) || y < std::min(p1.y, p2.y)) {
        return false;
    }
    // slope = undefined
    if (dX == 0) {
        *x = p1.x;
        return true;
    }

    // slope = 0, ignore the edge
    if (dY == 0) {
        return false;
    }

    // Point-slope form: Y = k(X - p1.x) + p1.y
    // X = (Y - p1.y) / k + p1.x
    *x = (y - p1.y) / (dY/dX) + p1.x;

    return true;
}


// Fan Method
void Polygon::Triangulate()
{
    //TODO: Populate list of triangles
    for (unsigned int i=0; i < this->m_verts.size()-2; i++) {
        Triangle tri;
        tri.m_indices[0] = 0;
        tri.m_indices[1] = i + 1;
        tri.m_indices[2] = i + 2;
        this->m_tris.push_back(tri);
    }
}

void Polygon::transform(glm::mat4 T, float width, float height, bool rendering2D) {
    // initialize
    this->m_screen_verts = std::vector<Vertex>(this->m_verts);
    if (!rendering2D) {
        for (unsigned int i = 0; i < this->m_verts.size(); i++) {
            glm::vec4 p = T * this->m_verts[i].m_pos;
            // refer to the slide 'Virtual Cameras and Spatial Transformation' P26
            // Important!!!
            p /= p.w; // normalize
            // refer to the slide 'Virtual Cameras and Spatial Transformation' P36
            p.x = (p.x + 1) * width  / 2;
            p.y = (1 - p.y) * height / 2;
            m_screen_verts[i].m_pos = p;
        }
    }

}

std::array<float, 4> Polygon::getBoundingBox(Triangle tri) {
    std::array<float, 4> bb;

        Vertex v1 = this->m_screen_verts[tri.m_indices[0]];
        Vertex v2 = this->m_screen_verts[tri.m_indices[1]];
        Vertex v3 = this->m_screen_verts[tri.m_indices[2]];
        // min X
        bb[0] = std::min({v1.m_pos.x, v2.m_pos.x, v3.m_pos.x});
        // max X
        bb[1] = std::max({v1.m_pos.x, v2.m_pos.x, v3.m_pos.x});
        // min Y
        bb[2] = std::min({v1.m_pos.y, v2.m_pos.y, v3.m_pos.y});
        // max Y
        bb[3] = std::max({v1.m_pos.y, v2.m_pos.y, v3.m_pos.y});


    return bb;
}

std::array<Segment, 3> Polygon::getEdges(Triangle tri) {
    Vertex v1 = this->m_screen_verts[tri.m_indices[0]];
    Vertex v2 = this->m_screen_verts[tri.m_indices[1]];
    Vertex v3 = this->m_screen_verts[tri.m_indices[2]];
    std::array<Segment, 3> edges = {Segment(v1.m_pos, v2.m_pos),
                                    Segment(v1.m_pos, v3.m_pos),
                                    Segment(v2.m_pos, v3.m_pos)};

    return edges;
}

float Area(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    // S = 0.5*length(cross(P1-P2, P3-P2))
    return 0.5 * glm::length(glm::cross(p1 - p2, p3 - p2));
}

// [S1 / (z1 * S), S2 / (z2 * S), S3/(z3 * S)]
glm::vec3 Polygon::BarycentricInterpolationWeight(Triangle tri, float x, float y){
    Vertex v1 = this->m_screen_verts[tri.m_indices[0]];
    Vertex v2 = this->m_screen_verts[tri.m_indices[1]];
    Vertex v3 = this->m_screen_verts[tri.m_indices[2]];

    // flatten the Z coordinate
    glm::vec3 p1 = glm::vec3(v1.m_pos.x, v1.m_pos.y, 0.f);
    glm::vec3 p2 = glm::vec3(v2.m_pos.x, v2.m_pos.y, 0.f);
    glm::vec3 p3 = glm::vec3(v3.m_pos.x, v3.m_pos.y, 0.f);
    glm::vec3 p  = glm::vec3(x, y, 0.f);

    // refer to the slide 'Perspective-Correct Interpolation' P19
    float S  = Area(p1, p2, p3);
    float S1 = Area(p, p2, p3);
    float S2 = Area(p, p3, p1);
    float S3 = Area(p, p1, p2);
    float epsilon = 1e-10; // in case z = 0

    return glm::vec3(S1 / ((v1.m_pos.z+epsilon) * S),
                     S2 / ((v2.m_pos.z+epsilon) * S),
                     S3 / ((v3.m_pos.z+epsilon) * S));
}

float Polygon::BarycentricZ(Triangle tri, float x, float y){
    glm::vec3 weights = BarycentricInterpolationWeight(tri, x, y);

    // refer to the slide 'Perspective-Correct Interpolation' P19
    float z = 1 / (weights[0] + weights[1] + weights[2]);
    return z;
}

glm::vec3 Polygon::BarycentricColor(Triangle tri, float x, float y){
    Vertex v1 = this->m_verts[tri.m_indices[0]];
    Vertex v2 = this->m_verts[tri.m_indices[1]];
    Vertex v3 = this->m_verts[tri.m_indices[2]];

    glm::vec3 weights = BarycentricInterpolationWeight(tri, x, y);
    float z = 1 / (weights[0] + weights[1] + weights[2]);

    // refer to the slide 'Perspective-Correct Interpolation' P20
    glm::vec3 Color = z * (v1.m_color * weights[0] + v2.m_color * weights[1] + v3.m_color * weights[2]);
    return Color;
}

glm::vec2 Polygon::BarycentricUV(Triangle tri, float x, float y){
    Vertex v1 = this->m_verts[tri.m_indices[0]];
    Vertex v2 = this->m_verts[tri.m_indices[1]];
    Vertex v3 = this->m_verts[tri.m_indices[2]];

    glm::vec3 weights = BarycentricInterpolationWeight(tri, x, y);
    float z = 1 / (weights[0] + weights[1] + weights[2]);

    // refer to the slide 'Perspective-Correct Interpolation' P20
    glm::vec2 UV = z * (v1.m_uv * weights[0] + v2.m_uv * weights[1] + v3.m_uv * weights[2]);
    return UV;
}

glm::vec4 Polygon::BarycentricNorm(Triangle tri, float x, float y){
    Vertex v1 = this->m_verts[tri.m_indices[0]];
    Vertex v2 = this->m_verts[tri.m_indices[1]];
    Vertex v3 = this->m_verts[tri.m_indices[2]];

    glm::vec3 weights = BarycentricInterpolationWeight(tri, x, y);
    float z = 1 / (weights[0] + weights[1] + weights[2]);

    // refer to the slide 'Perspective-Correct Interpolation' P20
    glm::vec4 Norm= z * (v1.m_normal * weights[0] + v2.m_normal * weights[1] + v3.m_normal * weights[2]);
    return Norm;
}

glm::vec3 Polygon::BarycentricCoordinate(Triangle tri, float x, float y) {
    Vertex v1 = this->m_verts[tri.m_indices[0]];
    Vertex v2 = this->m_verts[tri.m_indices[1]];
    Vertex v3 = this->m_verts[tri.m_indices[2]];

    glm::vec3 weights = BarycentricInterpolationWeight(tri, x, y);
    float z = 1 / (weights[0] + weights[1] + weights[2]);

    return glm::vec3(z * (v1.m_pos.x * weights[0] + v2.m_pos.x * weights[1] + v3.m_pos.x * weights[2]),
                     z * (v1.m_pos.y * weights[0] + v2.m_pos.y * weights[1] + v3.m_pos.y * weights[2]),
                     z);
}


// Creates a polygon from the input list of vertex positions and colors
Polygon::Polygon(const QString& name, const std::vector<glm::vec4>& pos, const std::vector<glm::vec3>& col)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    for(unsigned int i = 0; i < pos.size(); i++)
    {
        m_verts.push_back(Vertex(pos[i], col[i], glm::vec4(), glm::vec2()));
    }
    Triangulate();
}

// Creates a regular polygon with a number of sides indicated by the "sides" input integer.
// All of its vertices are of color "color", and the polygon is centered at "pos".
// It is rotated about its center by "rot" degrees, and is scaled from its center by "scale" units
Polygon::Polygon(const QString& name, int sides, glm::vec3 color, glm::vec4 pos, float rot, glm::vec4 scale)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    glm::vec4 v(0.f, 1.f, 0.f, 1.f);
    float angle = 360.f / sides;
    for(int i = 0; i < sides; i++)
    {
        glm::vec4 vert_pos = glm::translate(glm::vec3(pos.x, pos.y, pos.z))
                           * glm::rotate(rot, glm::vec3(0.f, 0.f, 1.f))
                           * glm::scale(glm::vec3(scale.x, scale.y, scale.z))
                           * glm::rotate(i * angle, glm::vec3(0.f, 0.f, 1.f))
                           * v;
        m_verts.push_back(Vertex(vert_pos, color, glm::vec4(), glm::vec2()));
    }

    Triangulate();
}

Polygon::Polygon(const QString &name)
    : m_tris(), m_verts(), m_name(name), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon()
    : m_tris(), m_verts(), m_name("Polygon"), mp_texture(nullptr), mp_normalMap(nullptr)
{}

Polygon::Polygon(const Polygon& p)
    : m_tris(p.m_tris), m_verts(p.m_verts), m_name(p.m_name), mp_texture(nullptr), mp_normalMap(nullptr)
{
    if(p.mp_texture != nullptr)
    {
        mp_texture = new QImage(*p.mp_texture);
    }
    if(p.mp_normalMap != nullptr)
    {
        mp_normalMap = new QImage(*p.mp_normalMap);
    }
}

Polygon::~Polygon()
{
    delete mp_texture;
}

void Polygon::SetTexture(QImage* i)
{
    mp_texture = i;
}

void Polygon::SetNormalMap(QImage* i)
{
    mp_normalMap = i;
}

void Polygon::AddTriangle(const Triangle& t)
{
    m_tris.push_back(t);
}

void Polygon::AddVertex(const Vertex& v)
{
    m_verts.push_back(v);
}

void Polygon::ClearTriangles()
{
    m_tris.clear();
}

Triangle& Polygon::TriAt(unsigned int i)
{
    return m_tris[i];
}

Triangle Polygon::TriAt(unsigned int i) const
{
    return m_tris[i];
}

Vertex &Polygon::VertAt(unsigned int i)
{
    return m_verts[i];
}

Vertex Polygon::VertAt(unsigned int i) const
{
    return m_verts[i];
}

glm::vec3 GetImageColor(const glm::vec2 &uv_coord, const QImage* const image)
{
    if(image)
    {
        int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
        int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
        QColor color = image->pixel(X, Y);
        return glm::vec3(color.red(), color.green(), color.blue());
    }
    return glm::vec3(255.f, 255.f, 255.f);
}
