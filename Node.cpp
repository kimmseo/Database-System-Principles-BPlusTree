//
// Created by Minseo on 2/7/2025.
//

#include "Node.h"

Node::Node(int aOrder) : fOrder(aOrder), fParent(nullptr) {}

Node::Node(int aOrder, Node* aParent) : fOrder(aOrder), fParent(aParent) {}

Node::~Node() {}

int Node::order() const {
    return fOrder;
}

Node* Node::parent() const {
    return fParent;
}

void Node::setParent(Node* aParent) {
    fParent = aParent;
}

bool Node::isLeaf() const {
    return !fParent;
}

bool Node::isRoot() const
{
    return !fParent;
}