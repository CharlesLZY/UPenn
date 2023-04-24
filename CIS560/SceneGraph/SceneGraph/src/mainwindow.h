#pragma once

#include <QMainWindow>
#include <QTreeWidgetItem>


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
    void slot_addRootToTreeWidget(QTreeWidgetItem*);

    void slot_selectTranslateNode(float, float);
    void slot_selectRotateNode(float);
    void slot_selectScaleNode(float, float);

private:
    Ui::MainWindow *ui;
};
