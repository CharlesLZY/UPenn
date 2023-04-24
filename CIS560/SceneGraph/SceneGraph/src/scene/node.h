#pragma once
#include <QTreeWidgetItem>
#include "smartpointerhelp.h"
#include "polygon.h"

class Node : public QTreeWidgetItem {
private:
    // A set of unique_ptrs to the node's children
    std::vector<uPtr<Node>> children;

    // A raw, C-style pointer to one instance of Polygon2D
    Polygon2D *geometry;

    // The color with which to draw the Polygon2D pointed to by the node
    glm::vec3 color; // same data type as the parameter of setColor() in polygon.h

    // A QString to represent a name for the node
    QString name;

    // Node type
    // 0: default
    // 1: TranslateNode
    // 2: RotateNode
    // 3: ScaleNode
    int type;

public:
    Node(const QString &name, Polygon2D *p, int type = 0);
    virtual ~Node();

    // A purely virtual function that computes and returns a 3x3 homogeneous matrix representing the transformation in the node
    virtual glm::mat3 transformation() = 0;

    // A function that adds a given unique_ptr as a child to this node
    Node &addNewChild(uPtr<Node> node);
    // A function that gets the children vector stored in this node
    const std::vector<uPtr<Node>> &getChildren();

    // A function that allows the user to modify the color stored in this node
    void setColor(const glm::vec3 &c);

    glm::vec3 getColor();

    // A function that gets the geometry stored in this node
    Polygon2D *getGeometry();

    // A function that gets the node type
    int getType();

    virtual void setValue(float v, int index) = 0;

    virtual float getValue(int index) = 0;

    void setGeometry(Polygon2D *geometry);
};

class TranslateNode : public Node {
private:
    float translateX; // translation in the X direction
    float translateY; // translation in the Y direction
public:
    TranslateNode(const QString &name, Polygon2D *p, float translateX, float translateY);
    virtual ~TranslateNode();

    glm::mat3 transformation();

    void setValue(float v, int index);
    float getValue(int index);
};


class RotateNode : public Node {
private:
    float theta; // the magnitude of its rotation in degrees
public:
    RotateNode(const QString &name, Polygon2D *p, float theta);
    virtual ~RotateNode();

    glm::mat3 transformation();

    void setValue(float v, int index);
    float getValue(int index);
};

class ScaleNode : public Node {
private:
    float scaleX; // scale in the X direction
    float scaleY; // scale in the Y direction
public:
    ScaleNode(const QString &name, Polygon2D *p, float scaleX, float scaleY);
    virtual ~ScaleNode();

    glm::mat3 transformation();

    void setValue(float v, int index);
    float getValue(int index);
};








