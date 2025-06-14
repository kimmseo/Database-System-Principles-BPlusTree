//
// Created by Minseo on 2/7/2025.
//

#ifndef PRINTER_H
#define PRINTER_H

#include <queue>

class Node;

class Printer {
  public:
    Printer();
    bool verbose() const;
    void setVerbose(bool aVerbose);
    void printTree(Node* aRoot) const;
    void printLeaves(Node* aRoot);

  private:
    void printEmptyTree() const;
    void printNonEmptyTree(Node* aRoot) const;
    void printCurrentRank(std::queue<Node*>* aCurrentRank, std::queue<Node*>* aNextRank) const;
    bool fVerbose;
};

#endif  // PRINTER_H
