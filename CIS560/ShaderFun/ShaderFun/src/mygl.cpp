#include "mygl.h"
#include <la.h>
#include <QResizeEvent>
#include <iostream>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomQuad(this), m_camera(640, 480, glm::vec3(0, 0, 12), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
      m_surfaceShaders(), m_postprocessShaders(),
      mp_progSurfaceCurrent(nullptr),
      mp_progPostprocessCurrent(nullptr),
      m_matcapTextures(), mp_matcapTexCurrent(nullptr),
      m_frameBuffer(-1),
      m_renderedTexture(-1),
      m_depthRenderBuffer(-1),
      m_time(0.f), m_mousePosPrev()
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();

    glDeleteVertexArrays(1, &m_vao);
    m_geomQuad.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &m_vao);

    createRenderBuffers();

    m_geomQuad.create();

    createShaders();
    createMeshes();
    createMatcapTextures();

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(m_vao);
}

void MyGL::resizeGL(int w, int h)
{
    mp_progSurfaceCurrent->setViewProjMatrix(m_camera.getView(), m_camera.getProj());

    for(std::shared_ptr<PostProcessShader> p : m_postprocessShaders)
    {
        p->setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    }

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    mp_progSurfaceCurrent->setCamPos(m_camera.eye);

    render3DScene();

    performPostprocessRenderPass();

    mp_progSurfaceCurrent->setTime(m_time);
    mp_progPostprocessCurrent->setTime(m_time);
    m_time++;

}

void MyGL::render3DScene()
{
    // Render the 3D scene to our frame buffer

    // Render to our framebuffer rather than the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the background texture first
    mp_modelCurrent->bindBGTexture();
    mp_progPostprocessNoOp->draw(m_geomQuad, 2);

    // Set the surface shader's transformation matrices
    mp_progSurfaceCurrent->setViewProjMatrix(m_camera.getView(), m_camera.getProj());
    mp_progSurfaceCurrent->setModelMatrix(glm::mat4());

    bindAppropriateTexture();

    // Draw the model
    mp_progSurfaceCurrent->draw(*mp_modelCurrent, 0);
}

void MyGL::performPostprocessRenderPass()
{
    // Render the frame buffer as a texture on a screen-size quad

    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

    mp_progPostprocessCurrent->draw(m_geomQuad, 0);
}

void MyGL::createRenderBuffers()
{
    // Initialize the frame buffers and render textures
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_renderedTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }
}

void MyGL::createShaders()
{
    // Surface shaders
    std::shared_ptr<SurfaceShader> lambert = std::make_shared<SurfaceShader>(this);
    lambert->create(":/glsl/surface/lambert.vert.glsl", ":/glsl/surface/lambert.frag.glsl");
    m_surfaceShaders.push_back(lambert);

    std::shared_ptr<SurfaceShader> blinnPhong = std::make_shared<SurfaceShader>(this);
    blinnPhong->create(":/glsl/surface/blinnPhong.vert.glsl", ":/glsl/surface/blinnPhong.frag.glsl");
    m_surfaceShaders.push_back(blinnPhong);

    std::shared_ptr<SurfaceShader> matcap = std::make_shared<SurfaceShader>(this);
    matcap->create(":/glsl/surface/matcap.vert.glsl", ":/glsl/surface/matcap.frag.glsl");
    m_surfaceShaders.push_back(matcap);

    std::shared_ptr<SurfaceShader> gradient = std::make_shared<SurfaceShader>(this);
    gradient->create(":/glsl/surface/gradient.vert.glsl", ":/glsl/surface/gradient.frag.glsl");
    m_surfaceShaders.push_back(gradient);

    std::shared_ptr<SurfaceShader> deform = std::make_shared<SurfaceShader>(this);
    deform->create(":/glsl/surface/deform.vert.glsl", ":/glsl/surface/deform.frag.glsl");
    m_surfaceShaders.push_back(deform);

    slot_setCurrentSurfaceShaderProgram(0);

    // Post-process shaders
    std::shared_ptr<PostProcessShader> noOp = std::make_shared<PostProcessShader>(this);
    noOp->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/noOp.frag.glsl");
    m_postprocessShaders.push_back(noOp);

    std::shared_ptr<PostProcessShader> greyscale = std::make_shared<PostProcessShader>(this);
    greyscale->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/greyscale.frag.glsl");
    m_postprocessShaders.push_back(greyscale);

    std::shared_ptr<PostProcessShader> gaussian = std::make_shared<PostProcessShader>(this);
    gaussian->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/gaussian.frag.glsl");
    m_postprocessShaders.push_back(gaussian);

    std::shared_ptr<PostProcessShader> sobel = std::make_shared<PostProcessShader>(this);
    sobel->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/sobel.frag.glsl");
    m_postprocessShaders.push_back(sobel);

    std::shared_ptr<PostProcessShader> bloom = std::make_shared<PostProcessShader>(this);
    bloom->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/bloom.frag.glsl");
    m_postprocessShaders.push_back(bloom);

    std::shared_ptr<PostProcessShader> mosaic = std::make_shared<PostProcessShader>(this);
    mosaic->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/mosaic.frag.glsl");
    m_postprocessShaders.push_back(mosaic);

    std::shared_ptr<PostProcessShader> shift = std::make_shared<PostProcessShader>(this);
    shift->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/shift.frag.glsl");
    m_postprocessShaders.push_back(shift);

    std::shared_ptr<PostProcessShader> bubble = std::make_shared<PostProcessShader>(this);
    bubble->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/bubble.frag.glsl");
    m_postprocessShaders.push_back(bubble);

    std::shared_ptr<PostProcessShader> fried = std::make_shared<PostProcessShader>(this);
    fried->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/fried.frag.glsl");
    m_postprocessShaders.push_back(fried);

    std::shared_ptr<PostProcessShader> test = std::make_shared<PostProcessShader>(this);
    test->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/test.frag.glsl");
    m_postprocessShaders.push_back(test);

    std::shared_ptr<PostProcessShader> biome = std::make_shared<PostProcessShader>(this);
    biome->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/biome.frag.glsl");
    m_postprocessShaders.push_back(biome);

    std::shared_ptr<PostProcessShader> height = std::make_shared<PostProcessShader>(this);
    height->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/height.frag.glsl");
    m_postprocessShaders.push_back(height);

    std::shared_ptr<PostProcessShader> venom = std::make_shared<PostProcessShader>(this);
    venom->create(":/glsl/post/passthrough.vert.glsl", ":/glsl/post/venom.frag.glsl");
    m_postprocessShaders.push_back(venom);

    slot_setCurrentPostprocessShaderProgram(0);
    mp_progPostprocessNoOp = m_postprocessShaders[0].get();

}

void MyGL::createMeshes()
{
    std::shared_ptr<Mesh> wahoo = std::make_shared<Mesh>(this);
    wahoo->createFromOBJ(":/objs/wahoo.obj", ":/textures/wahoo.bmp", ":/textures/smb.jpg");
//    wahoo->createFromOBJ(":/objs/wahoo.obj", ":/textures/wahoo.bmp", ":/textures/bg.jpg");
    wahoo->loadTexture();
    wahoo->loadBGTexture();
    m_models.push_back(wahoo);

    std::shared_ptr<Mesh> cow = std::make_shared<Mesh>(this);
    cow->createFromOBJ(":/objs/cow.obj", ":/textures/uvTest.jpg", ":/textures/winxp.jpg");
    cow->loadTexture();
    cow->loadBGTexture();
    m_models.push_back(cow);

    std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(this);
    cube->createCube(":/textures/fractal.jpg", ":/textures/mengersponge.jpg");
    cube->loadTexture();
    cube->loadBGTexture();
    m_models.push_back(cube);

    slot_setCurrentModel(0);
}

void MyGL::createMatcapTextures()
{
    std::shared_ptr<Texture> zbrush = std::make_shared<Texture>(this);
    zbrush->create(":/textures/matcaps/00ZBrush_RedWax.png");
    m_matcapTextures.push_back(zbrush);

    std::shared_ptr<Texture> redPlastic = std::make_shared<Texture>(this);
    redPlastic->create(":/textures/matcaps/JG_Red.png");
    m_matcapTextures.push_back(redPlastic);

    std::shared_ptr<Texture> chrome = std::make_shared<Texture>(this);
    chrome->create(":/textures/matcaps/silver.jpg");
    m_matcapTextures.push_back(chrome);

    std::shared_ptr<Texture> pearl = std::make_shared<Texture>(this);
    pearl->create(":/textures/matcaps/pearl.jpg");
    m_matcapTextures.push_back(pearl);

    std::shared_ptr<Texture> orangeGreen = std::make_shared<Texture>(this);
    orangeGreen->create(":/textures/matcaps/JGSpecial_01.png");
    m_matcapTextures.push_back(orangeGreen);

    std::shared_ptr<Texture> blurple = std::make_shared<Texture>(this);
    blurple->create(":/textures/matcaps/Shiny_Fire_1c.png");
    m_matcapTextures.push_back(blurple);

    std::shared_ptr<Texture> outline = std::make_shared<Texture>(this);
    outline->create(":/textures/matcaps/Outline.png");
    m_matcapTextures.push_back(outline);

    std::shared_ptr<Texture> normals = std::make_shared<Texture>(this);
    normals->create(":/textures/matcaps/normals.jpg");
    m_matcapTextures.push_back(normals);

    // If this vector of textures were to be altered
    // while the program was running, storing a pointer
    // to an element within it would be bad news, since
    // the elements inside would not be guaranteed to be
    // in the same memory as before.
    // However, this vector's contents are only ever modified
    // in this function, which is called exactly once in the
    // program's lifetime.

    slot_setCurrentMatcapTexture(0);
}

void MyGL::bindAppropriateTexture()
{
    // Matcap
    if(mp_progSurfaceCurrent == m_surfaceShaders[2].get())
    {
        mp_matcapTexCurrent->bind(0);
    }
    // Default case, use the texture provided by the model to be rendered
    else
    {
        mp_modelCurrent->bindTexture();
    }
}
