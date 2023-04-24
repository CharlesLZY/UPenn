#include "face.h"

int Face::lastFace = 0;

Face::Face()
    : QListWidgetItem(),
      color(glm::vec3(0.f)),
      edge(nullptr),
      ID(lastFace++)
{
    this->setText(QString::number(this->ID));
}


