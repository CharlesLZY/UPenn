#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QGraphicsScene>
#include <polygon.h>
#include <rasterizer.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void DisplayQImage(QImage &i);

    void keyPressEvent(QKeyEvent *e);

public slots:
    void slot_setAmbientR(double);
    void slot_setAmbientG(double);
    void slot_setAmbientB(double);

    void slot_setLightR(double);
    void slot_setLightG(double);
    void slot_setLightB(double);

    void slot_setLightPosX(double);
    void slot_setLightPosY(double);
    void slot_setLightPosZ(double);

    void slot_setShininess(int);

    void slot_setDiffuseAttenuate(double);

    void slot_setSpecularAugment(double);

    void slot_setAntiAliasing(int);

    void slot_setToonN(int);

    void slot_setAmbient(double);

    void slot_setAlbedo(double);

    void slot_setOffsetR(double);
    void slot_setOffsetG(double);
    void slot_setOffsetB(double);

    void slot_setAmpR(double);
    void slot_setAmpG(double);
    void slot_setAmpB(double);

    void slot_setFreqR(double);
    void slot_setFreqG(double);
    void slot_setFreqB(double);

    void slot_setPhaseR(double);
    void slot_setPhaseG(double);
    void slot_setPhaseB(double);


private slots:
    void on_actionLoad_Scene_triggered();

    void on_actionSave_Image_triggered();

    void on_actionEquilateral_Triangle_triggered();

    void on_actionQuit_Esc_triggered();

    void on_actionRender_noReflection_triggered();
    void on_actionRender_Lambertian_triggered();
    void on_actionRender_Phong_triggered();
    void on_actionRender_Iridescent_triggered();
    void on_actionRender_Toon_triggered();

    void on_actionRender_noTexture_triggered();

    void on_actionRender_Line_triggered();
    void on_actionRender_Reset_triggered();

private:
    Ui::MainWindow *ui;
    Polygon LoadOBJ(const QString &file, const QString &polyName);

    //This is used to display the QImage produced by RenderScene in the GUI
    QGraphicsScene graphics_scene;

    //This is the image rendered by your program when it loads a scene
    QImage rendered_image;

    //The instance of the Rasterizer used to render our scene
    Rasterizer rasterizer;

};

#endif // MAINWINDOW_H
