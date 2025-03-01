//
// Created by Minseo on 2/7/2025.
//

#ifndef NODE_H
#define NODE_H

#include <string>
#include "Definitions.h"

// Dummy key for when only entry's pointer has meaning
const KeyType DUMMY_KEY{-1};

// Abstract class
class Node {
  public:
    static int globalNodeCounter;  // global counter for node IDs
    int nodeID;                    // unique ID for this node
    explicit Node(int aOrder);
    explicit Node(int aOrder, Node* aParent);
    virtual ~Node();
    int order() const;
    Node* parent() const;
    void setParent(Node* aParent);
    bool isRoot() const;
    virtual bool isLeaf() const = 0;
    virtual int size() const = 0;
    virtual int minSize() const = 0;
    virtual int maxSize() const = 0;
    virtual std::string toString(bool aVerbose = false) const = 0;
    virtual const KeyType firstKey() const = 0;

  private:
    const int fOrder;
    Node* fParent;
};

#endif  // NODE_H
