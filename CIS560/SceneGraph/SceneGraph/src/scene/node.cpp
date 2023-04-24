#include "node.h"


Node::Node(const QString &name, Polygon2D *p, int type)
    : children(std::vector<uPtr<Node>>()), geometry(p), color(glm::vec3()), name(name), type(type)
{
    this->setText(0, name);
}

Node::~Node()
{}


Node &Node::addNewChild(uPtr<Node> node){
    this->addChild(node.get());

    Node &ref = *node;
    this->children.push_back(std::move(node));
    return ref;
}

const std::vector<uPtr<Node>> &Node::getChildren() {
    return this->children;
}

void Node::setColor(const glm::vec3 &c) {
    this->color = c;
}

glm::vec3 Node::getColor() {
    return this->color;
}

Polygon2D *Node::getGeometry() {
    return this->geometry;
}

int Node::getType() {
    return this->type;
}

void Node::setGeometry(Polygon2D *geometry) {
    this->geometry = geometry;
}

///////////////////////////////////////////////////////////////

TranslateNode::TranslateNode(const QString &name, Polygon2D *p, float translateX, float translateY)
    : Node(name, p, 1), translateX(translateX), translateY(translateY)
{}

TranslateNode::~TranslateNode()
{}

glm::mat3 TranslateNode::transformation() {
    return glm::translate(glm::mat3(), glm::vec2(this->translateX, this->translateY));
}

void TranslateNode::setValue(float v, int index) {
    if (index == 0) {
        this->translateX = v;
    } else if (index == 1) {
        this->translateY = v;
    }
}

float TranslateNode::getValue(int index) {
    if (index == 0) {
        return this->translateX;
    } else if (index == 1) {
        return this->translateY;
    }
    return 0;
}

///////////////////////////////////////////////////////////////


RotateNode::RotateNode(const QString &name, Polygon2D *p, float theta)
    : Node(name, p, 2), theta(theta)
{}

RotateNode::~RotateNode()
{}

glm::mat3 RotateNode::transformation() {
    return glm::rotate(glm::mat3(), glm::radians(this->theta));
}

void RotateNode::setValue(float v, int index) {
    if (index == 0) {
        this->theta = v;
    }
}

float RotateNode::getValue(int index) {
    if (index == 0) {
        return this->theta;
    }
    return 0;
}

//////////////////////////////////////////////////////////////

ScaleNode::ScaleNode(const QString &name, Polygon2D *p, float scaleX, float scaleY)
    : Node(name, p, 3), scaleX(scaleX), scaleY(scaleY)
{}

ScaleNode::~ScaleNode()
{}

glm::mat3 ScaleNode::transformation() {
    return glm::scale(glm::mat3(), glm::vec2(this->scaleX, this->scaleY));
}

void ScaleNode::setValue(float v, int index) {
    if (index == 0) {
        this->scaleX = v;
    } else if (index == 1) {
        this->scaleY = v;
    }
}

float ScaleNode::getValue(int index) {
    if (index == 0) {
        return this->scaleX;
    } else if (index == 1) {
        return this->scaleY;
    }
    return 0;
}




















