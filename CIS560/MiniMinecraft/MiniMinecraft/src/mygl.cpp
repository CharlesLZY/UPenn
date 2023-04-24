#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),

      m_worldAxes(this),
      m_progLambert(this),
      m_progFlat(this),
      m_progInstanced(this),
      m_progPost(this),
      m_progSky(this),
      m_terrain(this),
      m_player(glm::vec3(48.f, 210.f, 48.f), m_terrain),
      m_frameBuffer(this, this->width()*this->devicePixelRatio(), this->height()*this->devicePixelRatio(), this->devicePixelRatio()),
      m_quad(this),
      m_texture(this),
      m_textureNormal(this),
      m_textureBetter(this),
      m_time(0),
      m_currentMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()),
      openInventory(false), numGrass(10), numDirt(10), numStone(10),
      numSand(10), numSnow(10), numIce(10), numRedStone(10), numPumpkin(10),
      currBlockType(GRASS)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
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
    glEnable(GL_LINE_SMOOTH);
//    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_quad.createVBOdata();
    m_worldAxes.createVBOdata();
    m_frameBuffer.create();
    m_frameBuffer.bindFrameBuffer();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    m_progPost.create(":/glsl/post.vert.glsl", ":/glsl/post.frag.glsl");
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");


    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
    m_texture.create(":/minecraft_textures_all/minecraft_textures_all.png");
    m_texture.load(0);
    m_texture.bind(0);

    m_textureNormal.create(":/minecraft_textures_all/minecraft_normals_all.png");
    m_textureNormal.load(1);
    m_textureNormal.bind(1);

    m_textureBetter.create(":/minecraft_textures_all/textureBetter.png");
    m_textureBetter.load(2);
    m_textureBetter.bind(2);

//    m_terrain.CreateTestScene();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);




    /* TEST!!! */

    m_progLambert.set_eye(m_player.mcr_camera.mcr_position.x, m_player.mcr_camera.mcr_position.y, m_player.mcr_camera.mcr_position.z);

    m_progSky.setViewProjMatrix(glm::inverse(viewproj));
    m_progSky.set_dimensions(this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    m_progSky.set_eye(m_player.mcr_camera.mcr_position.x, m_player.mcr_camera.mcr_position.y, m_player.mcr_camera.mcr_position.z);




    m_frameBuffer.resize(w * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();
    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {


    // compute the delta-time
    float dT = (QDateTime::currentMSecsSinceEpoch() - m_currentMSecsSinceEpoch) / 1000.f;
    m_player.tick(dT, m_inputs);
    m_currentMSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch();

    m_terrain.updateTerrian(m_player.mcr_position);


    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}



// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {

    m_time++;

    m_frameBuffer.bindFrameBuffer();

    glViewport(0, 0, this->width()* this->devicePixelRatio(), this->height() *this->devicePixelRatio());


    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());



    m_progLambert.setTime(m_time++);
    m_progLambert.setCamPos(glm::vec4(m_player.mcr_position, 1));
    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progPost.setTime(m_time);


    m_progLambert.set_eye(m_player.mcr_camera.mcr_position.x, m_player.mcr_camera.mcr_position.y, m_player.mcr_camera.mcr_position.z);


    // Set the inverse of view-project matrix for mapping pixels back to world points
    m_progSky.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    // Set camera position
    m_progSky.set_eye(m_player.mcr_camera.mcr_position.x, m_player.mcr_camera.mcr_position.y, m_player.mcr_camera.mcr_position.z);
    // Set time
    m_progSky.setTime(m_time);

    m_progSky.draw(m_quad);


    renderTerrain();
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());


    glViewport(0, 0, this->width()* this->devicePixelRatio(), this->height() *this->devicePixelRatio());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_frameBuffer.bindToTextureSlot(1);

    if (m_player.getCameraBlock(m_terrain) == WATER) {
        // 1 is for under water visual effects
        m_progPost.setPostType(1);
    } else if (m_player.getCameraBlock(m_terrain) == LAVA) {
        // 2 for lava
        m_progPost.setPostType(2);
    } else {
        // any other number will be normal
        m_progPost.setPostType(0);
    }

    m_progPost.drawQuad(m_quad);

  // draw the world axes
    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progFlat.draw(m_worldAxes);

    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    // Entity.mcr_position
    int x = 16 * static_cast<int>(glm::floor(m_player.mcr_position.x / 16.f));
    int z = 16 * static_cast<int>(glm::floor(m_player.mcr_position.z / 16.f));

    m_texture.bind(0);
    m_textureNormal.bind(1);
    m_textureBetter.bind(2);
    m_terrain.draw(x - 256, x + 256, z - 256, z + 256, &m_progLambert);


}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        m_terrain.end();
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_player.rotateOnUpGlobal(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_player.rotateOnUpGlobal(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_player.rotateOnRightLocal(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_player.rotateOnRightLocal(amount);
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        if (m_inputs.isOnGround || m_player.getCameraBlock(m_terrain) == WATER || m_player.getCameraBlock(m_terrain) == LAVA) {
            m_inputs.spacePressed = true;
        }
    } else if (e->key() == Qt::Key_F) {
        m_inputs.flight_mode = !m_inputs.flight_mode;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    // not lazy and actually used the switch statement
    switch(e->key())
    {
        case Qt::Key_W:
            m_inputs.wPressed = false;
            break;
        case Qt::Key_S:
            m_inputs.sPressed = false;
            break;
        case Qt::Key_D:
            m_inputs.dPressed = false;
            break;
        case Qt::Key_A:
            m_inputs.aPressed = false;
            break;
        case Qt::Key_Q:
            m_inputs.qPressed = false;
            break;
        case Qt::Key_E:
            m_inputs.ePressed = false;
            break;
        case Qt::Key_Space:
            m_inputs.spacePressed = false;
            break;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {

//     QPoint lastPosition = QPoint(width() / 2.f, height() / 2.f);
//     float delta_x = GLfloat(lastPosition.x() - e->pos().x()) / width();
//     float delta_y = GLfloat(lastPosition.y() - e->pos().y()) / height();
//     m_player.rotateOnUpGlobal(delta_x * 360 * 0.05f);
//     m_player.rotateOnRightLocal(delta_y * 360 * 0.05f);
//     // move mouse back to center
//     moveMouseToCenter();

}

void MyGL::mousePressEvent(QMouseEvent *e) {

}

