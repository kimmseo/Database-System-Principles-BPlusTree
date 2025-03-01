//
// Created by Minseo on 2/7/2025.
//

#include "Node.h"

int Node::globalNodeCounter = 0; // initialize static counter

Node::Node(int aOrder) : fOrder(aOrder), fParent(nullptr) {
    nodeID = globalNodeCounter++;
}

Node::Node(int aOrder, Node* aParent) : fOrder(aOrder), fParent(aParent) {
    nodeID = globalNodeCounter++;
}

Node::~Node() {}

int Node::order() const { return fOrder; }

Node* Node::parent() const { return fParent; }

void Node::setParent(Node* aParent) { fParent = aParent; }

bool Node::isLeaf() const { return !fParent; }

bool Node::isRoot() const { return !fParent; }