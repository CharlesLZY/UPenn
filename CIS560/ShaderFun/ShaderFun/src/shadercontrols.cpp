#include "shadercontrols.h"
#include "ui_shadercontrols.h"


ShaderControls::ShaderControls(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShaderControls)
{
    ui->setupUi(this);

    connect(ui->modelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_setCurrentModel(int)));
    connect(ui->surfaceShaderComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_setCurrentSurfaceShaderProgram(int)));
    connect(ui->postprocessShaderComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_setCurrentPostprocessShaderProgram(int)));
    connect(ui->matcapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_setCurrentMatcapTexture(int)));
}

ShaderControls::~ShaderControls()
{
    delete ui;
}

void ShaderControls::slot_setCurrentModel(int i)
{
    emit sig_setCurrentModel(i);
}

void ShaderControls::slot_setCurrentSurfaceShaderProgram(int i)
{
    emit sig_setCurrentSurfaceShaderProgram(i);
}

void ShaderControls::slot_setCurrentPostprocessShaderProgram(int i)
{
    emit sig_setCurrentPostprocessShaderProgram(i);
}

void ShaderControls::slot_setCurrentMatcapTexture(int i)
{
    emit sig_setCurrentMatcapTexture(i);
}
