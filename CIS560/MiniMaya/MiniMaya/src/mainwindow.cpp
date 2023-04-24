#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // refer to hw2
    connect(ui->mygl, SIGNAL(sig_updateMesh(Mesh*)),
            this, SLOT(slot_updateMesh(Mesh*)));

    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedVertex(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedFace(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedHalfEdge(QListWidgetItem*)));

    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setPosX(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setPosY(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setPosZ(double)));

    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setColorR(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setColorG(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setColorB(double)));

    // update spin box according to the selected object
    connect(ui->mygl, SIGNAL(sig_updateVertPosX(double)),
            ui->vertPosXSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateVertPosY(double)),
            ui->vertPosYSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateVertPosZ(double)),
            ui->vertPosZSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateVertColorR(double)),
                ui->faceRedSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateVertColorG(double)),
            ui->faceGreenSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateVertColorB(double)),
            ui->faceBlueSpinBox, SLOT(setValue(double)));


    // Topology Editing Functions
    connect(ui->splitEdgeButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_splitEdge()));
    connect(ui->triangulateButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_triangulate()));

    connect(ui->divideButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_subdivide()));
    connect(ui->loadObjButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_loadObj()));



    connect(ui->mygl, SIGNAL(sig_updateSkeleton(Skeleton*)),
            this, SLOT(slot_updateSkeleton(Skeleton*)));
    connect(ui->jointTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            ui->mygl, SLOT(slot_setSelectedJoint(QTreeWidgetItem*)));

    // update spin box according to the selected object
    connect(ui->mygl, SIGNAL(sig_updateJointPosX(double)),
            ui->jointPosXSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateJointPosY(double)),
            ui->jointPosYSpinBox, SLOT(setValue(double)));
    connect(ui->mygl, SIGNAL(sig_updateJointPosZ(double)),
            ui->jointPosZSpinBox, SLOT(setValue(double)));

    connect(ui->mygl, SIGNAL(sig_updateJointRot(QString)),
            ui->curRot, SLOT(setText(QString)));


    connect(ui->jointPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setJointPosX(double)));
    connect(ui->jointPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setJointPosY(double)));
    connect(ui->jointPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_setJointPosZ(double)));


    connect(ui->jointRotXButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotX()));
    connect(ui->jointRotYButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotY()));
    connect(ui->jointRotZButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotZ()));

    connect(ui->jointRotXButtonInv, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotXInv()));
    connect(ui->jointRotYButtonInv, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotYInv()));
    connect(ui->jointRotZButtonInv, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_setJointRotZInv()));


    connect(ui->loadJsonButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_loadJson()));

    connect(ui->bindButton, SIGNAL(clicked(bool)),
            ui->mygl, SLOT(slot_bindMesh()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}



void MainWindow::slot_updateMesh(Mesh* mesh) {
    for (uPtr<Vertex> &vertex : mesh->verts) {
        ui->vertsListWidget->addItem(vertex.get());
    }

    for (uPtr<Face> &face : mesh->faces) {
        ui->facesListWidget->addItem(face.get());
    }

    for (uPtr<HalfEdge> &edge : mesh->edges) {
        ui->halfEdgesListWidget->addItem(edge.get());
    }

}

void MainWindow::slot_updateSkeleton(Skeleton *skeleton) {
    for (const uPtr<Joint> &j : skeleton->joints) {
        ui->jointTreeWidget->addTopLevelItem(j.get());
    }
}


