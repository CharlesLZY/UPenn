#pragma once
#include <polygon.h>
#include <QImage>

#define LAMBERTIAN 1
#define BLINN_PHONG 2
#define IRIDESCENT 3
#define TOON 4

class Camera
{
public:
    glm::vec4 forwardDir; // forward direction
    glm::vec4 rightDir; // right direction
    glm::vec4 upDir; // up direction
    float FOV; // field of view (degree)
    glm::vec4 pose; // camera's position in world space
    float nearClip; // near clip plane
    float farClip; // far clip plane
    float ratio; // camera's aspect ratio (the ratio of width and height)

    Camera();
    // A function which returns a view matrix based on the camera's local axes and position
    glm::mat4 viewMatrix();
    // A function which returns a perspective projection matrix based on the camera's clipping planes, aspect ratio, and field of view
    glm::mat4 projectionMatrix();
    // Functions that translate the camera along each of its local axes
    void translateForward(float d);
    void translateRight(float d);
    void translateUp(float d);
    // Functions that rotate the camera about each of its local axes
    void rotateForward(float d);
    void rotateRight(float d);
    void rotateUp(float d);

};


class Rasterizer
{
private:
    //This is the set of Polygons loaded from a JSON scene file
    std::vector<Polygon> m_polygons;
public:
    Camera m_camera;

    int reflectionModel = 0;
    bool renderingLine = false;
    bool noTexture = false;

    float albedo = 1.f;
    float ambient = 0.3;

    int toon_N = 5;

    // Phone Shading
    glm::vec3 lightPos = glm::vec3(5.f, 5.f, 5.f);
    glm::vec3 light = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 ambientLight = glm::vec3(0.f, 0.f, 0.f);

    // Iridescent Shading
    glm::vec3 offset = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 amp = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 freq = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 phase = glm::vec3(0.25, 0.5, 0.75);

    float diffuseAttenuate = 1.f;
    float specularAugment = 1.f;
    int shininess = 4;

    int antiAliasing = 1;


    Rasterizer(const std::vector<Polygon>& polygons);
    QImage RenderScene();
    void ClearScene();

};


