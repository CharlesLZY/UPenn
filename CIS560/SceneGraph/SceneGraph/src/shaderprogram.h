#pragma once

#include <openglcontext.h>
#include <la.h>
#include <glm/glm.hpp>

#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>
#include "drawable.h"


class ShaderProgram
{
public:
    GLuint m_vertShader; // A handle for the vertex shader stored in this shader program
    GLuint m_fragShader; // A handle for the fragment shader stored in this shader program
    GLuint m_prog;       // A handle for the linked shader program stored in this class

    int m_attrPos; // A handle for the "in" vec3 representing vertex position in the vertex shader
    int m_attrCol; // A handle for the "in" vec3 representing vertex color in the vertex shader

    int m_unifModel; // A handle for the "uniform" mat3 representing model matrix in the vertex shader
    int m_unifView; // A handle for the "uniform" mat3 representing the matrix used to scale geometry to the desired size in the vertex shader

public:
    ShaderProgram(OpenGLContext* context);
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat3 &model);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewMatrix(const glm::mat3 &vp);
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(OpenGLContext &f, Drawable &d);
    // Utility function used in create()
    char* textFileRead(const char*);
    // Utility function used in create()
    QString qTextFileRead(const char*);
    // Utility function that prints any shader compilation errors to the console
    void printShaderInfoLog(int shader);
    // Utility function that prints any shader linking errors to the console
    void printLinkInfoLog(int prog);

private:
    OpenGLContext* context;   // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                            // we need to pass our OpenGL context to the Drawable in order to call GL functions
                            // from within this class.
};

