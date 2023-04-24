#include "vertex.h"

int Vertex::lastVertex = 0;

Vertex::Vertex()
    : QListWidgetItem(),
      pos(glm::vec3(0.f)),
      edge(nullptr),
      ID(lastVertex++),
      joints()
{
    this->setText(QString::number(this->ID));
}



