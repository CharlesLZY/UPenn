#ifndef SHADERCONTROLS_H
#define SHADERCONTROLS_H

#include <QWidget>

namespace Ui {
class ShaderControls;
}

class ShaderControls : public QWidget
{
    Q_OBJECT

public:
    explicit ShaderControls(QWidget *parent = 0);
    ~ShaderControls();

    Ui::ShaderControls *ui;

public slots:
    void slot_setCurrentModel(int);
    void slot_setCurrentSurfaceShaderProgram(int);
    void slot_setCurrentPostprocessShaderProgram(int);
    void slot_setCurrentMatcapTexture(int);

signals:
    void sig_setCurrentModel(int);
    void sig_setCurrentSurfaceShaderProgram(int);
    void sig_setCurrentPostprocessShaderProgram(int);
    void sig_setCurrentMatcapTexture(int);
};

#endif // SHADERCONTROLS_H
