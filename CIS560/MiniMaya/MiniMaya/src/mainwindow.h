#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <vertex.h>
#include <face.h>
#include <halfedge.h>
#include <mesh.h>
#include <skeleton.h>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

public slots:

    void slot_updateMesh(Mesh* mesh);

    void slot_updateSkeleton(Skeleton *skeleton);

private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
