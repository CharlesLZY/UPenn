#include "texture.h"

Texture::Texture(OpenGLContext* context)
    :m_context(context), m_textureHandle(0), m_textureImage(nullptr)
{

}

Texture::~Texture(){

}

void Texture::create(const char *texturePath){
    m_context->printGLErrorLog();

    QImage img(texturePath);
    img.convertToFormat(QImage::Format_ARGB32);
    img = img.mirrored();
    m_textureImage = std::make_shared<QImage> (img);
    m_context->glGenTextures(1, &m_textureHandle);

    m_context->printGLErrorLog();
}

void Texture::load(GLuint texSlot = 0){
    m_context->printGLErrorLog();

    m_context->glActiveTexture(GL_TEXTURE0 + texSlot);
    m_context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                            m_textureImage->width(), m_textureImage->height(),
                            0, GL_BGRA, GL_UNSIGNED_BYTE,
                            m_textureImage->bits());

    m_context->printGLErrorLog();

}

void Texture::bind(GLuint texSlot){
    m_context->glActiveTexture(GL_TEXTURE0 + texSlot);
    m_context->glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}


