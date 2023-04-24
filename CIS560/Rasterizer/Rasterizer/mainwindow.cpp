#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QFileDialog>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QImageWriter>
#include <QDebug>
#include <tiny_obj_loader.h>

//Poke around in this file if you want, but it's virtually uncommented!
//You won't need to modify anything in here to complete the assignment.

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    //The key shortcuts for the other menu commands were set in Qt's GUI
    //editor. This one was implemented as a key press event for illustration purposes.
    case Qt::Key_Escape : on_actionQuit_Esc_triggered();                break;

    case Qt::Key_W      : rasterizer.m_camera.translateForward(0.5f);   break;
    case Qt::Key_S      : rasterizer.m_camera.translateForward(-0.5f);  break;
    case Qt::Key_A      : rasterizer.m_camera.translateRight(-0.5f);    break;
    case Qt::Key_D      : rasterizer.m_camera.translateRight(0.5f);     break;
    case Qt::Key_Q      : rasterizer.m_camera.translateUp(-0.5f);       break;
    case Qt::Key_E      : rasterizer.m_camera.translateUp(0.5f);        break;
    case Qt::Key_Up     : rasterizer.m_camera.rotateRight(-5.f);        break;
    case Qt::Key_Down   : rasterizer.m_camera.rotateRight(5.f);         break;
    case Qt::Key_Left   : rasterizer.m_camera.rotateUp(5.f);            break;
    case Qt::Key_Right  : rasterizer.m_camera.rotateUp(-5.f);           break;
    case Qt::Key_Z      : rasterizer.m_camera.rotateForward(5.f);       break;
    case Qt::Key_X      : rasterizer.m_camera.rotateForward(-5.f);      break;
    }



    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    rasterizer(std::vector<Polygon>())
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    connect(ui->ambientR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmbientR(double)));
    connect(ui->ambientG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmbientG(double)));
    connect(ui->ambientB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmbientB(double)));

    connect(ui->lR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightR(double)));
    connect(ui->lG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightG(double)));
    connect(ui->lB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightB(double)));

    connect(ui->lpX, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightPosX(double)));
    connect(ui->lpY, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightPosY(double)));
    connect(ui->lpZ, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setLightPosZ(double)));

    connect(ui->shininess, SIGNAL(valueChanged(int)),
            this, SLOT(slot_setShininess(int)));

    connect(ui->diffuse, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setDiffuseAttenuate(double)));

    connect(ui->specular, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setSpecularAugment(double)));

    connect(ui->antiAliasing, SIGNAL(valueChanged(int)),
            this, SLOT(slot_setAntiAliasing(int)));

    connect(ui->toon, SIGNAL(valueChanged(int)),
            this, SLOT(slot_setToonN(int)));

    connect(ui->ambient, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmbient(double)));

    connect(ui->albedo, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAlbedo(double)));

    connect(ui->offsetR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setOffsetR(double)));
    connect(ui->offsetG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setOffsetG(double)));
    connect(ui->offsetB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setOffsetB(double)));

    connect(ui->ampR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmpR(double)));
    connect(ui->ampG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmpG(double)));
    connect(ui->ampB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setAmpB(double)));

    connect(ui->ampR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setFreqR(double)));
    connect(ui->ampG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setFreqG(double)));
    connect(ui->ampB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setFreqB(double)));

    connect(ui->phaseR, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setPhaseR(double)));
    connect(ui->phaseG, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setPhaseG(double)));
    connect(ui->phaseB, SIGNAL(valueChanged(double)),
            this, SLOT(slot_setPhaseB(double)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DisplayQImage(QImage &i)
{
    QPixmap pixmap(QPixmap::fromImage(i));
    graphics_scene.addPixmap(pixmap);
    graphics_scene.setSceneRect(pixmap.rect());
    ui->scene_display->setScene(&graphics_scene);
}

void MainWindow::on_actionLoad_Scene_triggered()
{
    std::vector<Polygon> polygons;

    QString filename = QFileDialog::getOpenFileName(0, QString("Load Scene File"), QDir::currentPath().append(QString("../..")), QString("*.json"));
    int i = filename.length() - 1;
    while(QString::compare(filename.at(i), QChar('/')) != 0)
    {
        i--;
    }
    QString local_path = filename.left(i+1);

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        qWarning("Could not open the JSON file.");
        return;
    }
    QByteArray file_data = file.readAll();

    QJsonDocument jdoc(QJsonDocument::fromJson(file_data));
    //Read the mesh data in the file
    QJsonArray objects = jdoc.object()["objects"].toArray();
    for(int i = 0; i < objects.size(); i++)
    {
        std::vector<glm::vec4> vert_pos;
        std::vector<glm::vec3> vert_col;
        QJsonObject obj = objects[i].toObject();
        QString type = obj["type"].toString();
        //Custom Polygon case
        if(QString::compare(type, QString("custom")) == 0)
        {
            QString name = obj["name"].toString();
            QJsonArray pos = obj["vertexPos"].toArray();
            for(int j = 0; j < pos.size(); j++)
            {
                QJsonArray arr = pos[j].toArray();
                glm::vec4 p(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble(), 1);
                vert_pos.push_back(p);
            }
            QJsonArray col = obj["vertexCol"].toArray();
            for(int j = 0; j < col.size(); j++)
            {
                QJsonArray arr = col[j].toArray();
                glm::vec3 c(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
                vert_col.push_back(c);
            }
            Polygon p(name, vert_pos, vert_col);
            polygons.push_back(p);
        }
        //Regular Polygon case
        else if(QString::compare(type, QString("regular")) == 0)
        {
            QString name = obj["name"].toString();
            int sides = obj["sides"].toInt();
            QJsonArray colorA = obj["color"].toArray();
            glm::vec3 color(colorA[0].toDouble(), colorA[1].toDouble(), colorA[2].toDouble());
            QJsonArray posA = obj["pos"].toArray();
            glm::vec4 pos(posA[0].toDouble(), posA[1].toDouble(), posA[2].toDouble(),1);
            float rot = obj["rot"].toDouble();
            QJsonArray scaleA = obj["scale"].toArray();
            glm::vec4 scale(scaleA[0].toDouble(), scaleA[1].toDouble(), scaleA[2].toDouble(),1);
            Polygon p(name, sides, color, pos, rot, scale);
            polygons.push_back(p);
        }
        //OBJ file case
        else if(QString::compare(type, QString("obj")) == 0)
        {
            QString name = obj["name"].toString();
            QString filename = local_path + obj["filename"].toString();
            Polygon p = LoadOBJ(filename, name);
            p.SetTexture(new QImage(local_path + obj["texture"].toString()));
            if(obj.contains(QString("normalMap")))
            {
                p.SetNormalMap(new QImage(local_path + obj["normalMap"].toString()));
            }
            polygons.push_back(p);
        }
    }

    rasterizer = Rasterizer(polygons);

    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}


Polygon MainWindow::LoadOBJ(const QString &file, const QString &polyName)
{
    Polygon p(polyName);
    QString filepath = file;
    std::vector<tinyobj::shape_t> shapes; std::vector<tinyobj::material_t> materials;
    std::string errors = tinyobj::LoadObj(shapes, materials, filepath.toStdString().c_str());
    std::cout << errors << std::endl;
    if(errors.size() == 0)
    {
        int min_idx = 0;
        //Read the information from the vector of shape_ts
        for(unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<glm::vec4> pos, nor;
            std::vector<glm::vec2> uv;
            std::vector<float> &positions = shapes[i].mesh.positions;
            std::vector<float> &normals = shapes[i].mesh.normals;
            std::vector<float> &uvs = shapes[i].mesh.texcoords;
            for(unsigned int j = 0; j < positions.size()/3; j++)
            {
                pos.push_back(glm::vec4(positions[j*3], positions[j*3+1], positions[j*3+2],1));
            }
            for(unsigned int j = 0; j < normals.size()/3; j++)
            {
                nor.push_back(glm::vec4(normals[j*3], normals[j*3+1], normals[j*3+2],0));
            }
            for(unsigned int j = 0; j < uvs.size()/2; j++)
            {
                uv.push_back(glm::vec2(uvs[j*2], uvs[j*2+1]));
            }
            for(unsigned int j = 0; j < pos.size(); j++)
            {
                p.AddVertex(Vertex(pos[j], glm::vec3(255,255,255), nor[j], uv[j]));
            }

            std::vector<unsigned int> indices = shapes[i].mesh.indices;
            for(unsigned int j = 0; j < indices.size(); j += 3)
            {
                Triangle t;
                t.m_indices[0] = indices[j] + min_idx;
                t.m_indices[1] = indices[j+1] + min_idx;
                t.m_indices[2] = indices[j+2] + min_idx;
                p.AddTriangle(t);
            }

            min_idx += pos.size();
        }
    }
    else
    {
        //An error loading the OBJ occurred!
        std::cout << errors << std::endl;
    }
    return p;
}


void MainWindow::on_actionSave_Image_triggered()
{
    QString filename = QFileDialog::getSaveFileName(0, QString("Save Image"), QString("../.."), QString("*.bmp"));
    QString ext = filename.right(4);
    if(QString::compare(ext, QString(".bmp")) != 0)
    {
        filename.append(QString(".bmp"));
    }
    QImageWriter writer(filename);
    writer.setFormat("bmp");
    if(!writer.write(rendered_image))
    {
        qDebug() << writer.errorString();
    }
}

void MainWindow::on_actionEquilateral_Triangle_triggered()
{
    std::vector<glm::vec4> pos;
    pos.push_back(glm::vec4(384,382,0,1));
    pos.push_back(glm::vec4(256,160,0,1));
    pos.push_back(glm::vec4(128,382,0,1));

    std::vector<glm::vec3> col;
    col.push_back(glm::vec3(0,0,255));
    col.push_back(glm::vec3(0,255,0));
    col.push_back(glm::vec3(255,0,0));

    Polygon p(QString("Equilateral"), pos, col);

    p.ClearTriangles();

    Triangle t;
    for(unsigned int i = 0; i < 3; i++)
    {
        t.m_indices[i] = i;
    }

    p.AddTriangle(t);
    std::vector<Polygon> vec; vec.push_back(p);

    rasterizer = Rasterizer(vec);

    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionQuit_Esc_triggered()
{
    QApplication::exit();
}


void MainWindow::on_actionRender_noReflection_triggered() {
    rasterizer.reflectionModel = 0;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_noTexture_triggered() {
    rasterizer.noTexture = true;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Lambertian_triggered() {
    rasterizer.reflectionModel = 1;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Phong_triggered() {
    rasterizer.reflectionModel = 2;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Iridescent_triggered() {
    rasterizer.reflectionModel = 3;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Toon_triggered() {
    rasterizer.reflectionModel = 4;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Line_triggered() {
    rasterizer.renderingLine = true;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::on_actionRender_Reset_triggered() {
    rasterizer.renderingLine = false;
    rasterizer.noTexture = false;
    rasterizer.reflectionModel = 0;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmbientR(double R) {
    rasterizer.ambientLight[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmbientG(double G) {
    rasterizer.ambientLight[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmbientB(double B) {
    rasterizer.ambientLight[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}


void MainWindow::slot_setLightR(double R) {
    rasterizer.light[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setLightG(double G) {
    rasterizer.light[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setLightB(double B) {
    rasterizer.light[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setLightPosX(double X) {
    rasterizer.lightPos.x = (float) X;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setLightPosY(double Y) {
    rasterizer.lightPos.y = (float) Y;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setLightPosZ(double Z) {
    rasterizer.lightPos.z = (float) Z;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setShininess(int shininess) {
    rasterizer.shininess = shininess;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setDiffuseAttenuate(double r) {
    rasterizer.diffuseAttenuate = (float) r;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setSpecularAugment(double r) {
    rasterizer.specularAugment = (float) r;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAntiAliasing(int k) {
    rasterizer.antiAliasing = k;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setToonN(int n) {
    rasterizer.toon_N = n;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmbient(double r) {
    rasterizer.ambient = (float) r;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAlbedo(double r) {
    rasterizer.albedo = (float) r;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setOffsetR(double R) {
    rasterizer.offset[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setOffsetG(double G) {
    rasterizer.offset[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setOffsetB(double B) {
    rasterizer.offset[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmpR(double R) {
    rasterizer.amp[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmpG(double G) {
    rasterizer.amp[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setAmpB(double B) {
    rasterizer.amp[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setFreqR(double R) {
    rasterizer.freq[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setFreqG(double G) {
    rasterizer.freq[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setFreqB(double B) {
    rasterizer.freq[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setPhaseR(double R) {
    rasterizer.phase[0] = (float) R;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setPhaseG(double G) {
    rasterizer.phase[1] = (float) G;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}

void MainWindow::slot_setPhaseB(double B) {
    rasterizer.phase[2] = (float) B;
    rendered_image = rasterizer.RenderScene();
    DisplayQImage(rendered_image);
}






