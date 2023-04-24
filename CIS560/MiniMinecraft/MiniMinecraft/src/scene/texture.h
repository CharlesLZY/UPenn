#ifndef TEXTURE_H
#define TEXTURE_H

#include "openglcontext.h"
#include "glm_includes.h"
#include "smartpointerhelp.h"


class Texture
{
public:
    OpenGLContext* m_context;
    GLuint m_textureHandle;
    std::shared_ptr<QImage> m_textureImage;

    Texture(OpenGLContext* context);
    ~Texture();

    void create(const char *texturePath);
    void load(GLuint texSlot);
    void bind(GLuint texSlot);

};

#endif // TEXTURE_H
