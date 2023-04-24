#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <queue>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this),
      m_progFlat(this),
      m_progSkeleton(this),
      m_glCamera(),
      vertToDisplay(this),
      faceToDisplay(this),
      edgeToDisplay(this),
      selectedVert(nullptr),
      selectedFace(nullptr),
      selectedEdge(nullptr),
      m_mesh(this),
      m_skeleton(this)
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
    m_mesh.destroy();
    m_skeleton.destroy();
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
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();

    m_mesh.buildCube();
    m_mesh.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the skeleton shader
    m_progSkeleton.create(":/glsl/skeleton.vert.glsl", ":/glsl/skeleton.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    emit sig_updateMesh(&m_mesh);
    emit sig_updateSkeleton(&m_skeleton);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSkeleton.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(glm::vec3(m_glCamera.eye));
    m_progFlat.setModelMatrix(glm::mat4(1.f));
    m_progLambert.setModelMatrix(glm::mat4(1.f));


    m_progSkeleton.setViewProjMatrix(m_glCamera.getViewProj());
    m_progSkeleton.setModelMatrix(glm::mat4(1.f));

    // if the mesh has been bound
    if (m_mesh.verts[0]->joints.size() > 0) {
        m_progSkeleton.draw(m_mesh);
        m_progSkeleton.draw(m_skeleton);
    }
    else {
        m_progLambert.draw(m_mesh);
    }

    // follow the instruction on hw5 website
    if (selectedVert) {
        glDisable(GL_DEPTH_TEST);
        m_progFlat.draw(vertToDisplay);
        glEnable(GL_DEPTH_TEST);
    }
    else if (selectedFace) {
        glDisable(GL_DEPTH_TEST);
        m_progFlat.draw(faceToDisplay);
        glEnable(GL_DEPTH_TEST);
    }
    else if (selectedEdge) {
        glDisable(GL_DEPTH_TEST);
        m_progFlat.draw(edgeToDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    // Always display skeleton
    glDisable(GL_DEPTH_TEST);
    m_progFlat.draw(m_skeleton);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    }
    // visual debugging
    //  NEXT half-edge of the currently selected half-edge
    else if (e->key() == Qt::Key_N) {
        if (this->selectedEdge) {
            this->selectedEdge = this->selectedEdge->next;
            this->edgeToDisplay.destroy(); // clear the buffer
            this->edgeToDisplay.updateEdge(this->selectedEdge);
            this->edgeToDisplay.create();
        }
    }
    //  SYM half-edge of the currently selected half-edge
    else if (e->key() == Qt::Key_M) {
        if (this->selectedEdge) {
            this->selectedEdge = this->selectedEdge->SYM;
            this->edgeToDisplay.destroy(); // clear the buffer
            this->edgeToDisplay.updateEdge(this->selectedEdge);
            this->edgeToDisplay.create();
        }
    }
    // FACE of the currently selected half-edge
    else if (e->key() == Qt::Key_F) {
        if (this->selectedEdge) {
            this->selectedFace = this->selectedEdge->face;
            this->faceToDisplay.destroy(); // clear the buffer
            this->faceToDisplay.updateFace(this->selectedFace);
            this->faceToDisplay.create();
            this->selectedEdge = nullptr;
        }
    }
    // VERTEX of the currently selected half-edge
    else if (e->key() == Qt::Key_V) {
        if (this->selectedEdge) {
            this->selectedVert = this->selectedEdge->vert;
            this->vertToDisplay.destroy(); // clear the buffer
            this->vertToDisplay.updateVertex(this->selectedVert);
            this->vertToDisplay.create();
            this->selectedEdge = nullptr;
        }
    }
    // HALF-EDGE of the currently selected face
    else if (e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_H) {
        if (this->selectedFace) {
            this->selectedEdge = this->selectedFace->edge;
            this->edgeToDisplay.destroy(); // clear the buffer
            this->edgeToDisplay.updateEdge(this->selectedEdge);
            this->edgeToDisplay.create();
            this->selectedFace = nullptr;
        }
    }
    // this condition must be behind shift + H
    // HALF-EDGE of the currently selected vertex
    else if (e->key() == Qt::Key_H) {
        if (this->selectedVert) {
            this->selectedEdge = this->selectedVert->edge;
            this->edgeToDisplay.destroy(); // clear the buffer
            this->edgeToDisplay.updateEdge(this->selectedEdge);
            this->edgeToDisplay.create();
            this->selectedVert = nullptr;
        }
    }


    m_glCamera.RecomputeAttributes();
    this->update();  // Calls paintGL, among other things
}



void MyGL::slot_setSelectedVertex(QListWidgetItem *vert) {
    if (vert) {
        this->selectedVert = dynamic_cast<Vertex*>(vert);

        this->vertToDisplay.destroy();
        this->vertToDisplay.updateVertex(this->selectedVert);
        this->vertToDisplay.create();

        emit sig_updateVertPosX(this->selectedVert->pos.x);
        emit sig_updateVertPosY(this->selectedVert->pos.y);
        emit sig_updateVertPosZ(this->selectedVert->pos.z);

        this->selectedFace = nullptr;
        this->selectedEdge = nullptr;

        this->update();
    }
}

void MyGL::slot_setSelectedFace(QListWidgetItem *face) {
    if (face) {
        this->selectedFace = dynamic_cast<Face*>(face);

        this->faceToDisplay.destroy();
        this->faceToDisplay.updateFace(this->selectedFace);
        this->faceToDisplay.create();

        emit sig_updateVertColorR(this->selectedFace->color.r);
        emit sig_updateVertColorG(this->selectedFace->color.g);
        emit sig_updateVertColorB(this->selectedFace->color.b);

        this->selectedVert = nullptr;
        this->selectedEdge = nullptr;

        this->update();
    }

}

void MyGL::slot_setSelectedHalfEdge(QListWidgetItem *edge) {
    if (edge) {
        this->selectedEdge = dynamic_cast<HalfEdge*>(edge);

        this->edgeToDisplay.destroy();
        this->edgeToDisplay.updateEdge(this->selectedEdge);
        this->edgeToDisplay.create();

        this->selectedVert = nullptr;
        this->selectedFace = nullptr;

        this->update();
    }
}

void MyGL::slot_setPosX(double x) {
    if (this->selectedVert) {
        this->selectedVert->pos.x = x;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->vertToDisplay.destroy();
        this->vertToDisplay.create();
        this->update();
    }
}

void MyGL::slot_setPosY(double y) {
    if (this->selectedVert) {
        this->selectedVert->pos.y = y;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->vertToDisplay.destroy();
        this->vertToDisplay.create();
        this->update();
    }
}

void MyGL::slot_setPosZ(double z) {
    if (this->selectedVert) {
        this->selectedVert->pos.z = z;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->vertToDisplay.destroy();
        this->vertToDisplay.create();
        this->update();
    }
}

void MyGL::slot_setColorR(double R) {
    if (this->selectedFace) {
        this->selectedFace->color.r = R;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->faceToDisplay.destroy();
        this->faceToDisplay.create();
        this->update();
    }
}

void MyGL::slot_setColorG(double G) {
    if (this->selectedFace) {
        this->selectedFace->color.g = G;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->faceToDisplay.destroy();
        this->faceToDisplay.create();
        this->update();
    }
}

void MyGL::slot_setColorB(double B) {
    if (this->selectedFace) {
        this->selectedFace->color.b = B;
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->faceToDisplay.destroy();
        this->faceToDisplay.create();
        this->update();
    }
}

void MyGL::slot_splitEdge() {
    if (selectedEdge) {
        this->m_mesh.splitEdge(selectedEdge);
        this->selectedEdge = nullptr;

        emit sig_updateMesh(&m_mesh);
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();
    }
}

void MyGL::slot_triangulate() {
    if (selectedFace) {
        this->m_mesh.triangulate(selectedFace);
        this->selectedFace = nullptr;

        emit sig_updateMesh(&m_mesh);
        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();
    }
}

void MyGL::slot_subdivide() {
    this->m_mesh.subdivide();
    this->selectedVert = nullptr;
    this->selectedFace = nullptr;
    this->selectedEdge = nullptr;

    emit sig_updateMesh(&m_mesh);
    this->m_mesh.destroy();
    this->m_mesh.create();
    this->update();
}


/* Copy from the lecture recording on 2.25
    void loadOBJ() {
        for each "f" line in the file {
            1. Assemble the HEs that connect the Face's vertices
            2. For each HE we just instantiated, generate a pair<Vertex*, Vertex*> indicating the Vertices that bound it
            3. Query your unordered_map<pair<V*, V*>, HE*> for a value associated with the pair you just generated
            4. If there is already a value in the map, that it your HE's SVM
            5. If there is NOT already a value in the map, add the current HE to the map with the pair as its key
        }
    }
*/

// Copy the example code from lecture recording on 2.25
class HashPair {
public:
    uint64_t operator()(const std::pair<Vertex*, Vertex*> &p) const {
        uint64_t v1Addr = reinterpret_cast<uint64_t>(p.first);
        uint64_t v2Addr = reinterpret_cast<uint64_t>(p.second);

        return v1Addr ^ v2Addr;
    }
};


void MyGL::slot_loadObj() {

    QString filename = QFileDialog::getOpenFileName(0, QString("Load OBJ File"), QDir::currentPath().append(QString("../..")), QString("*.obj"));

    QFile file(filename);

    if (file.exists()) {

        // reset the static variables
        Vertex::lastVertex = 0;
        Face::lastFace = 0;
        HalfEdge::lastHalfEdge = 0;

        // clear previous mesh
        this->m_mesh.verts.clear();
        this->m_mesh.faces.clear();
        this->m_mesh.edges.clear();

        int edgeNum = 0;

        // refer to https://doc.qt.io/qt-5/qfile.html
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);

            // store the file content
            std::vector<glm::vec3> v;
            std::vector<glm::vec2> vt;
            std::vector<glm::vec3> vn;

            // Follow the instructure from the lecture
            std::unordered_map<std::pair<Vertex*, Vertex*>, HalfEdge*, HashPair> edgeMap;

            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList content = line.split(" ");

                /*
                    OBJ File format:
                    v denotes vertex position
                    vt denotes texture UV
                    vn denotes ertex normal
                    f line defines mesh:
                      pos/UV/normal
                */

                if (content.count() > 0) { // skip empty line

                    // vertex
                    if (content[0] == "v") {
                        v.push_back(glm::vec3(content[1].toFloat(), content[2].toFloat(), content[3].toFloat()));

                        uPtr<Vertex> vert = mkU<Vertex>();
                        vert->pos = glm::vec3(content[1].toFloat(), content[2].toFloat(), content[3].toFloat());
                        this->m_mesh.verts.push_back(std::move(vert));
                    }
                    // UV
                    else if (content[0] == "vt") {
                        vt.push_back(glm::vec2(content[1].toFloat(), content[2].toFloat()));
                    }
                    // normal
                    else if (content[0] == "vn") {
                        vn.push_back(glm::vec3(content[1].toFloat(), content[2].toFloat(), content[3].toFloat()));
                    }
                    // face
                    else if (content[0] == "f") {
                        int vertNum = content.size() - 1;
                        uPtr<Face> face = mkU<Face>();
                        // assign a random color
                        face->color = glm::vec3(rand() / (float) RAND_MAX);
                        // 1. Assemble the HEs that connect the Face's vertices
                        for (int i=1; i <= vertNum; i++) { // # edge = # vertex
                            // f pos/UV/norm (index of v, vt, vn)
                            QStringList vertInfo = content[i].split("/");

                            uPtr<HalfEdge> edge = mkU<HalfEdge>();
                            edge->face = face.get();
                            int vIdx = vertInfo[0].toInt() - 1; // index of v
                            edge->vert = this->m_mesh.verts[vIdx].get();

                            this->m_mesh.verts[vIdx]->edge = edge.get();

                            face->edge = edge.get();
                            this->m_mesh.edges.push_back(std::move(edge));
                        }

                        // link new edges
                        edgeNum += vertNum; // edge = # vertex
                        for (int i=1; i <= vertNum; i++) {
                            int edgeIdx = edgeNum - vertNum + i - 1; // index of current edge

                            if (i != vertNum) {
                                HalfEdge *nextEdge = this->m_mesh.edges[edgeIdx+1].get();
                                this->m_mesh.edges[edgeIdx]->next = nextEdge; // curEdge
                            }
                            else { // the last edge link to the last face
                                HalfEdge *nextEdge = this->m_mesh.edges[edgeNum - vertNum].get();
                                this->m_mesh.edges[edgeNum - 1]->next = nextEdge;
                            }

                            HalfEdge *curEdge = m_mesh.edges[edgeIdx].get();

                            QStringList vertInfo = content[i].split("/");
                            QStringList prevVertInfo = content[i - 1].split("/");

                            if (i == 1) { // first vertex is linked by the last vertex
                                prevVertInfo = content[vertNum].split("/");
                            }

                            // 2. For each HE we just instantiated, generate a pair<Vertex*, Vertex*> indicating the Vertices that bound it
                            // 3. Query your unordered_map<pair<V*, V*>, HE*> for a value associated with the pair you just generated
                            std::pair pair    = {this->m_mesh.verts[vertInfo[0].toInt()].get(), this->m_mesh.verts[prevVertInfo[0].toInt()].get()};
                            std::pair pairSYM = {this->m_mesh.verts[prevVertInfo[0].toInt()].get(), this->m_mesh.verts[vertInfo[0].toInt()].get()};
                             // 4. If there is already a value in the map, that it your HE's SVM
                            if (edgeMap.find(pairSYM) != edgeMap.end()) {
                                HalfEdge *edgeSYM = edgeMap.find(pairSYM)->second;
                                curEdge->SYM = edgeSYM;
                                edgeSYM->SYM = curEdge;
                            }
                            // 5. If there is NOT already a value in the map, add the current HE to the map with the pair as its key
                            else {
                                edgeMap.insert({pair, curEdge});
                            }
                        }

                        this->m_mesh.faces.push_back(std::move(face));
                    }
                }
            }

            file.close();

            this->selectedVert = nullptr;
            this->selectedFace = nullptr;
            this->selectedEdge = nullptr;

            emit sig_updateMesh(&m_mesh);
            this->m_mesh.destroy();
            this->m_mesh.create();
            this->update();
        }
    }
}


void MyGL::slot_loadJson() {

    QString fileName = QFileDialog::getOpenFileName(0, QString("Load JSON File"), QDir::currentPath().append(QString("../..")), QString("*.json"));

    QFile file(fileName);

    if (file.exists()) {
        // reset
        Joint::lastJoint = 0;

        for (uPtr<Vertex> &vert : m_mesh.verts) {
            // clear the bound joint
            vert->joints.clear();
        }

        // clear previous skeleton
        this->m_skeleton.representedJoint = nullptr;
        this->m_skeleton.joints.clear();


        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QByteArray content = file.readAll();
            file.close();
            // refer to https://doc.qt.io/qt-5/json.html
            QJsonDocument jsonDoc(QJsonDocument::fromJson(content));
            QJsonObject root = jsonDoc.object()["root"].toObject();

            this->setJoint(root);
        }
    }
    file.close();

    emit sig_updateSkeleton(&m_skeleton);
    this->m_skeleton.destroy();
    this->m_skeleton.create();
    this->update();
}

Joint *MyGL::setJoint(QJsonObject root) {
    QString name = root["name"].toString();
    QJsonArray position = root["pos"].toArray();
    QJsonArray rotation = root["rot"].toArray();

    glm::vec3 pos =  glm::vec3(position[0].toDouble(), position[1].toDouble(), position[2].toDouble());
    glm::vec4 rot = glm::vec4(rotation[0].toDouble(), rotation[1].toDouble(), rotation[2].toDouble(), rotation[3].toDouble());

    uPtr<Joint> joint = mkU<Joint>(name, pos, rot);

    Joint *res = joint.get();
    this->m_skeleton.joints.push_back(std::move(joint));

    // set children joint
    QJsonArray children = root["children"].toArray();
    for (int i=0; i < children.size(); i++) {
        // recursively set children of child
        Joint *child = this->setJoint(children[i].toObject());
        child->parent = res;
        res->addChild(child);
        res->children.push_back(child);
    }
    return res;
}

// refer to https://stackoverflow.com/questions/5782658/extracting-yaw-from-a-quaternion#:~:text=Having%20given%20a%20Quaternion%20q,*q.y%20%2D%20q.z*q.z)%3B
void MyGL::updateRot() {
    glm::quat q = this->m_skeleton.representedJoint->rot;

    // Quaternion to roll, pitch, yaw
    // roll (x-axis rotation)
    double roll = std::atan2(2.0*(q.y*q.z + q.w*q.x), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

    // pitch (y-axis rotation)
    double pitch = std::asin(-2.0*(q.x*q.z - q.w*q.y));

    // yaw (z-axis rotation)
    double yaw = std::atan2(2.0*(q.x*q.y + q.w*q.z), q.w*q.w + q.x*q.x - q.y*q.y - q.z*q.z);

    QString curRot = QString::fromStdString(std::string("roll: ").append(std::to_string(roll))
                                                .append(" pitch: ").append(std::to_string(pitch))
                                                .append(" yaw: ").append(std::to_string(yaw)));


    emit sig_updateJointRot(curRot);
}

void MyGL::slot_setSelectedJoint(QTreeWidgetItem *joint) {
    if (joint) {

        this->m_skeleton.updateJoint(dynamic_cast<Joint*>(joint));

        emit sig_updateJointPosX(this->m_skeleton.representedJoint->pos.x);
        emit sig_updateJointPosY(this->m_skeleton.representedJoint->pos.y);
        emit sig_updateJointPosZ(this->m_skeleton.representedJoint->pos.z);

        this->updateRot();

        this->m_skeleton.destroy();
        this->m_skeleton.create();
        this->update();
    }
}

void MyGL::slot_setJointPosX(double x) {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->pos.x = x;
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();
    }
}

void MyGL::slot_setJointPosY(double y) {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->pos.y = y;
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();
    }
}

void MyGL::slot_setJointPosZ(double z) {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->pos.z = z;
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();
    }
}


void MyGL::slot_setJointRotX() {

    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(5.f), glm::vec3(1, 0, 0));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}

void MyGL::slot_setJointRotY() {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(5.f), glm::vec3(0, 1, 0));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}


void MyGL::slot_setJointRotZ() {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(5.f), glm::vec3(0, 0, 1));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}



void MyGL::slot_setJointRotXInv() {

    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(-5.f), glm::vec3(1, 0, 0));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}

void MyGL::slot_setJointRotYInv() {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(-5.f), glm::vec3(0, 1, 0));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}


void MyGL::slot_setJointRotZInv() {
    if (this->m_skeleton.representedJoint) {
        this->m_skeleton.representedJoint->rot *= glm::rotate(glm::quat(), glm::radians(-5.f), glm::vec3(0, 0, 1));
        this->m_progSkeleton.setTransformationMatrix(this->m_skeleton.getTransformations());

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->m_mesh.destroy();
        this->m_mesh.create();
        this->update();

        this->updateRot();
    }
}


void MyGL::slot_bindMesh() {
    if (this->m_skeleton.joints.size() > 0) {

        // Set bind matrices
        std::vector<glm::mat4> transformations;
        std::vector<glm::mat4> bindMatrixs;

        for (uPtr<Joint> &joint : this->m_skeleton.joints) {
            glm::mat4 transformation = joint->getOverallTransformation();
            joint->bindMatrix = glm::inverse(transformation);

            bindMatrixs.push_back(joint->bindMatrix);
            transformations.push_back(transformation);
        }

        // refer to the slide "Skeletons, Skinning, and Quaternions" P9
        // Find 2 nearest joints to the vertex and bind it
        for (uPtr<Vertex> &vert : m_mesh.verts) {
            int joint1 = -1;
            float dist1 = FLT_MAX;
            int joint2 = -1;
            float dist2 = FLT_MAX;

            for (int i=0; i < (int) m_skeleton.joints.size(); i++) {
                glm::vec4 pos = transformations[i] * glm::vec4(0, 0, 0, 1);
                float temp = glm::distance2(glm::vec4(vert->pos, 1), pos);

                if (joint1 == -1) {
                    joint1 = i;
                    dist1 = temp;
                }
                else if (joint2 == -1) {
                    if (temp < dist1) {
                        joint2 = joint1;
                        dist2 = dist1;
                        joint1 = i;
                        dist1 = temp;
                    }
                    else {
                        joint2 = i;
                        dist2 = temp;
                    }
                }
                else {
                    if (temp < dist1) {
                        joint2 = joint1;
                        dist2 = dist1;
                        joint1 = i;
                        dist1 = temp;
                    }
                    else if (temp < dist2) {
                        joint2 = i;
                        dist2 = temp;
                    }
                }

            }

            vert->joints.push_back({joint1, 1 - dist1 / (dist1 + dist2)});
            vert->joints.push_back({joint2, 1 - dist2 / (dist1 + dist2)});
        }

        // Pass to shader
        m_progSkeleton.setTransformationMatrix(transformations);
        m_progSkeleton.setBindMatrix(bindMatrixs);

        this->m_mesh.destroy();
        this->m_mesh.create();

        this->m_skeleton.destroy();
        this->m_skeleton.create();

        this->update();
    }
}

