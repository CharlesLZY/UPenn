#include "rasterizer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

Rasterizer::Rasterizer(const std::vector<Polygon>& polygons)
    : m_polygons(polygons)
{}

QImage Rasterizer::RenderScene()
{
    bool rendering2D = false;
//    bool renderingLine = true;
//    int reflectionModel = 4;
//    bool noTexture = true; // use blank texture
//    float albedo = 1.f; // 反射率
//    float ambient = 0.3; // 环境光

    int displayWidth = 512;
    int displayHeight = 512;
//    int antiAliasing = 4;

    // object mesh and texture are independent of screen size
    int width = displayWidth * antiAliasing;
    int height = displayHeight * antiAliasing;

    // z-buffering
//    float z_buffer[width * height];
//    float *z_buffer = new float[width * height]; // have to allocate on heap, or the stack will overflow (MUST BE DELETE AFTER USED)
//    std::fill_n(z_buffer, width * height, 1e10);

    std::unique_ptr<float[]> z_buffer = std::make_unique<float[]>(width * height);
    std::fill_n(z_buffer.get(), width * height, 1e10);

    // Transformation from world space to screen space
    glm::mat4 T = this->m_camera.projectionMatrix() * this->m_camera.viewMatrix();


    QImage result(width, height, QImage::Format_RGB32);
    // Fill the image with black pixels.
    // Note that qRgb creates a QColor,
    // and takes in values [0, 255] rather than [0, 1].
    result.fill(qRgb(0.f, 0.f, 0.f));
    // TODO: Complete the various components of code that make up this function
    // It should return the rasterized image of the current scene

    // Render each polygon
    for (unsigned int i = 0; i < this->m_polygons.size(); i++) {


        // transform the m_verts from world space to screen space
        this->m_polygons[i].transform(T, (float) width, (float) height, rendering2D);


        // Render each triangle
        for (unsigned int j = 0; j < this->m_polygons[i].m_tris.size(); j++) {
            std::array<float, 4> bb = this->m_polygons[i].getBoundingBox(this->m_polygons[i].m_tris[j]);
            float min_X = bb[0];
            float max_X = bb[1];
            float min_Y = bb[2];
            float max_Y = bb[3];
            // beyond the window
            if (max_X < 0 || max_Y < 0 || min_X >= width || min_Y >= height) {
                continue;
            }

            std::array<Segment, 3> edges = this->m_polygons[i].getEdges(this->m_polygons[i].m_tris[j]);

            // render horizontal edge
            if (renderingLine) {
                for (int n=0; n<3; n++) {
                    int row = (int) edges[n].p1.y;
                    if (edges[n].dY == 0 && row >= 0 && row < height) {
                        int lp = glm::max((int)  ceil(glm::min(edges[n].p1.x, edges[n].p2.x)), 0);
                        int rp = glm::min((int) floor(glm::max(edges[n].p1.x, edges[n].p2.x)), width - 1);
                        // Render each pixel
                        for (int col = std::max((int) lp+1, 0); col < std::min((int) rp+1, width); col++) {
                            float z = this->m_polygons[i].BarycentricZ(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                            if (z < z_buffer[row + width * col]) {
                                z_buffer[row + width * col] = z;
                                glm::vec3 Color = glm::vec3(255.f, 255.f, 255.f);
                                result.setPixel(col, row, qRgb(Color[0], Color[1], Color[2]));
                            }

                        } // end loop for each column
                    }
                } // end loop for three edges
            }

            // Render each row
            for (int row = std::max((int) min_Y, 0); row < std::min((int) max_Y+1, height); row++){
                float lp = width - 1; // leftmost intersection
                float rp = 0; // rightmost intersection
                float intersection = 0;
                for (int n=0; n<3; n++) {
                    if (edges[n].getIntersection(row, &intersection)) {
                        lp = std::min(lp, intersection);
                        rp = std::max(rp, intersection);
                    }
                }

                bool flag = false; // whether middle pixels are ignored
                // Render each pixel
                for (int col = std::max((int) lp+1, 0); col < std::min((int) rp+1, width); col++) {
                    float z = this->m_polygons[i].BarycentricZ(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                    if (z < z_buffer[row + width * col]) {
                        z_buffer[row + width * col] = z;
                        if (rendering2D) {
                            glm::vec3 Color = this->m_polygons[i].BarycentricColor(this->m_polygons[i].m_tris[j], (float) col, (float) row);

                            if (renderingLine) {
                                Color = glm::vec3(255.f, 255.f, 255.f);
                            }
                            Color = glm::clamp(Color, 0.f, 255.f);
                            result.setPixel(col, row, qRgb(Color[0], Color[1], Color[2]));
                        }
                        else { // rendering3D
                            glm::vec3 Color;
                            glm::vec2 UV = this->m_polygons[i].BarycentricUV(this->m_polygons[i].m_tris[j], (float) col, (float) row);

                            if (noTexture) {
                                Color = glm::vec3(255.f, 255.f, 255.f);
                            }
                            else if (this->m_polygons[i].mp_texture) {
                                Color = GetImageColor(UV, this->m_polygons[i].mp_texture);
                            }
                            else {
                                Color = this->m_polygons[i].BarycentricColor(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                            }

                            if (reflectionModel == LAMBERTIAN) {
                                glm::vec4 Norm = this->m_polygons[i].BarycentricNorm(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                                // Using the camera's look vector as the light direction
                                glm::vec4 lightDir = this->m_camera.forwardDir;

                                // refer to the slide 'Texturing, Shading. and Lighting' P18
                                // Lambertian reflection model to attenuate the brightness of each point on the surface of a mesh
                                float attenuate = glm::clamp(glm::dot(Norm, glm::normalize(-lightDir)), 0.f, 1.f); // weaken the light
                                // there is physical meaning behind the model， 正宗的物理的模型，点积对应的通量
                                // 一个材质的反射，包含了3个部分： 1. diffusion 漫反射 2. specular reflection 镜面反射 3. ambient 环境光 (除了前面两个，剩下的其他各种来源的光）
                                // Phong Model是基于物理现象做的一个经验建模，可以很好的模拟一些光学现象

                                Color = attenuate* Color + ambient * Color;
                                Color = Color * albedo;
                            }
                            else if (reflectionModel == BLINN_PHONG) {
//                                glm::vec3 lightPos = glm::vec3(5.f, 5.f, 5.f); // position of light source
//                                glm::vec3 light = glm::vec3(1.f, 1.f, 1.f); // color of light source 光源，漫反射和高光都来自它
//                                glm::vec3 ambientLight = glm::vec3(0.f, 0.f, 0.f); // color of ambient light 环境光，类似于底色
//                                float diffusionAttenuate = 1.f; // scalar to make diffusion darker
//                                float specularAugment = 1.f; // scalar to make highlight brighter
//                                 // shininess controls how diffuse the highlight is,
//                                 // with smaller values being more diffuse
//                                int shininess = 4; // 越小越亮，高光越大（广）

                                // the world coordinate for the pixel
                                glm::vec3 position = this->m_polygons[i].BarycentricCoordinate(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                                // L
                                glm::vec3 lightDir = glm::normalize(lightPos - position);
                                // N
                                glm::vec3 Norm = glm::vec3(this->m_polygons[i].BarycentricNorm(this->m_polygons[i].m_tris[j], (float) col, (float) row));

                                // Use Lambertian Reflection to simulate diffusion
                                glm::vec3 diffusion = glm::max(glm::dot(lightDir, glm::vec3(Norm)), 0.f) * light * diffuseAttenuate;

                                // refer to the slide "Texturing, Shading, and Lighting" P25
                                // V
                                glm::vec3 viewDir = glm::normalize(glm::vec3(m_camera.pose) - position);
                                // H
                                glm::vec3 halfDir = (viewDir + lightDir) / 2.f;
                                // S
                                glm::vec3 specular = std::max((float)pow(glm::dot(halfDir, Norm), shininess), 0.f) * light * specularAugment;
                                // Mathematically, the highlight is strongest when V aligns with the ray formed by reflecting L about N,
                                // which is the ray of perfect specular reflection

                                Color *= (albedo * ambientLight + diffusion + specular); // Phong model
                            }
                            // NPR: Non-Photorealistic Rendering
                            else if (reflectionModel == IRIDESCENT) {
                                // the world coordinate for the pixel
                                glm::vec3 position = this->m_polygons[i].BarycentricCoordinate(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                                glm::vec3 viewDir = glm::normalize(glm::vec3(m_camera.pose) - position);
                                glm::vec3 Norm = glm::vec3(this->m_polygons[i].BarycentricNorm(this->m_polygons[i].m_tris[j], (float) col, (float) row));
                                // Refer to the slide 'Procedural Color' P9 P18
                                float t = glm::clamp(glm::dot(viewDir, Norm), 0.f, 1.f);

//                                glm::vec3 offset = glm::vec3(0.5, 0.5, 0.5);
//                                glm::vec3 amp = glm::vec3(0.5, 0.5, 0.5);
//                                glm::vec3 freq = glm::vec3(1.f, 1.f, 1.f);
//                                glm::vec3 phase = glm::vec3(0, 0.33333, 0.66666);

                                // refer to http://dev.thi.ng/gradients/
                                float R = offset[0] + amp[0] * glm::cos(2 * glm::pi<float>() * (freq[0]*t + phase[0]));
                                float G = offset[1] + amp[1] * glm::cos(2 * glm::pi<float>() * (freq[1]*t + phase[1]));
                                float B = offset[2] + amp[2] * glm::cos(2 * glm::pi<float>() * (freq[2]*t + phase[2]));
                                Color = glm::vec3(R, G, B) * 255.f;

                            }
                            else if (reflectionModel == TOON) {

                                // Same as Lambertian Reflection
                                glm::vec4 Norm = this->m_polygons[i].BarycentricNorm(this->m_polygons[i].m_tris[j], (float) col, (float) row);
                                // Using the camera's look vector as the light direction
                                glm::vec4 lightDir = this->m_camera.forwardDir;
                                float attenuate = glm::clamp(glm::dot(Norm, glm::normalize(-lightDir)), 0.f, 1.f); // weaken the light
                                Color = attenuate * Color + ambient * Color;
                                Color = Color * albedo;
                                // refer to the slide 'Procedural Color' P16
                                // Discretize color space
//                                int toon_N = 5; // color section number
                                for (int c = 0; c < 3; c++) {
                                    Color[c] = (float) ((int) Color[c] / (255 / toon_N)) * (255.f / (float) toon_N);
                                }
                            }

                            if (renderingLine) {
                                Color = glm::vec3(255.f, 255.f, 255.f);
                            }
                            Color = glm::clamp(Color, 0.f, 255.f);
                            result.setPixel(col, row, qRgb(Color[0], Color[1], Color[2]));
                        }

                    }

                    if (renderingLine && !flag) {
                        flag = true;
                        col = std::min((int) rp+1, width) - 2;
                    }

                } // end loop for each column
            } // end loop for each row
        } // end loop for each triangle
    } // end loop for each polygon

//    delete[] z_buffer;
    return result.scaled(displayWidth, displayHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void Rasterizer::ClearScene()
{
    m_polygons.clear();
}

Camera::Camera()
      : forwardDir(glm::vec4(0.f, 0.f, -1.f, 0.f)),
        rightDir(glm::vec4(1.f, 0.f, 0.f, 0.f)),
        upDir(glm::vec4(0.f, 1.f, 0.f, 0.f)),
        FOV(45.f),
        pose(glm::vec4(0.f, 0.f, 10.f, 1.f)),
        nearClip(0.01),
        farClip(100.f),
        ratio(1.f)
{}

// refer to the slide 'Virtual Cameras and Spatial Transformation' P35
glm::mat4 Camera::viewMatrix() {
    glm::vec4 col1 = glm::vec4(this->rightDir.x, this->upDir.x, this->forwardDir.x, 0.f);
    glm::vec4 col2 = glm::vec4(this->rightDir.y, this->upDir.y, this->forwardDir.y, 0.f);
    glm::vec4 col3 = glm::vec4(this->rightDir.z, this->upDir.z, this->forwardDir.z, 0.f);
    glm::vec4 col4 = glm::vec4(0.f, 0.f, 0.f, 1.f);
    glm::mat4 viewOrientMatrix = glm::mat4(col1, col2, col3, col4);

    col1 = glm::vec4(1.f, 0.f, 0.f, 0.f);
    col2 = glm::vec4(0.f, 1.f, 0.f, 0.f);
    col3 = glm::vec4(0.f, 0.f, 1.f, 0.f);
    col4 = glm::vec4(-this->pose.x, -this->pose.y, -this->pose.z, 1.f);
    glm::mat4 viewTranslateMatrix = glm::mat4(col1, col2, col3, col4);

    return viewOrientMatrix * viewTranslateMatrix;
}

// refer to the slide 'Virtual Cameras and Spatial Transformation' P30
glm::mat4 Camera::projectionMatrix() {
    glm::vec4 col1 = glm::vec4(1 / (this->ratio * glm::tan(this->FOV/2)), 0.f, 0.f, 0.f);
    glm::vec4 col2 = glm::vec4(0.f, 1 / (glm::tan(this->FOV/2)), 0.f, 0.f);
    glm::vec4 col3 = glm::vec4(0.f, 0.f, this->farClip / (this->farClip - this->nearClip), 1.f);
    glm::vec4 col4 = glm::vec4(0.f, 0.f, -this->farClip * this->nearClip / (this->farClip - this->nearClip), 0.f);
    return glm::mat4(col1, col2, col3, col4);

}

void Camera::translateForward(float d) {
    this->pose += this->forwardDir * d;
}

void Camera::translateRight(float d) {
    this->pose += this->rightDir * d;
}

void Camera::translateUp(float d) {
    this->pose += this->upDir * d;
}

void Camera::rotateForward(float d) {
    // glm::rotate(mat, angle, axis)
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.f), d, glm::vec3(this->forwardDir));
    this->forwardDir = rotateMatrix * this->forwardDir;
    this->rightDir = rotateMatrix * this->rightDir;
    this->upDir = rotateMatrix * this->upDir;
}

void Camera::rotateRight(float d) {
    // glm::rotate(mat, angle, axis)
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.f), d, glm::vec3(this->rightDir));
    this->forwardDir = rotateMatrix * this->forwardDir;
    this->rightDir = rotateMatrix * this->rightDir;
    this->upDir = rotateMatrix * this->upDir;
}

void Camera::rotateUp(float d) {
    // glm::rotate(mat, angle, axis)
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.f), d, glm::vec3(this->upDir));
    this->forwardDir = rotateMatrix * this->forwardDir;
    this->rightDir = rotateMatrix * this->rightDir;
    this->upDir = rotateMatrix * this->upDir;
}






