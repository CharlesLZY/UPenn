#include "halfedge.h"

int HalfEdge::lastHalfEdge = 0;

HalfEdge::HalfEdge()
    : QListWidgetItem(),
      next(nullptr),
      SYM(nullptr),
      face(nullptr),
      vert(nullptr),
      ID(lastHalfEdge++)
{
    this->setText(QString::number(this->ID));
}

