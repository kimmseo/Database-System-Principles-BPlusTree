//
// Created by Minseo on 2/7/2025.
//

#ifndef INTERNALNODE_H
#define INTERNALNODE_H

#include <queue>
#include <vector>
#include "Definitions.h"
#include "Node.h"

class InternalNode : public Node {
  public:
    explicit InternalNode(int aOrder);
    explicit InternalNode(int aOrder, Node* aParent);
    ~InternalNode() override;
    using MappingType = std::pair<KeyType, Node*>;
    [[nodiscard]] bool isLeaf() const override;
    [[nodiscard]] int size() const override;
    [[nodiscard]] int minSize() const override;
    [[nodiscard]] int maxSize() const override;
    [[nodiscard]] KeyType keyAt(int aIndex) const;
    void setKeyAt(int aIndex, KeyType aKey);
    [[nodiscard]] Node* firstChild() const;
    void populateNewRoot(Node* aOldNode, KeyType aNewKey, Node* aNewNode);
    int insertNodeAfter(Node* aOldNode, KeyType aNewKey, Node* aNewNode);
    void remove(int aIndex);
    Node* removeAndReturnOnlyChild();
    KeyType replaceAndReturnFirstKey();
    void moveHalfTo(InternalNode* aRecipient);
    void moveAllTo(InternalNode* aRecipient, int aParentIndex);
    void moveFirstToEndOf(InternalNode* aRecipient);
    void moveLastToFrontOf(InternalNode* aRecipient, int aParentIndex);
    [[nodiscard]] Node* lookup(KeyType aKey) const;
    int nodeIndex(Node* aNode) const;
    [[nodiscard]] Node* neighbour(int aIndex) const;
    [[nodiscard]] std::string toString(bool aVerbose = false) const override;
    void queueUpChildren(std::queue<Node*>* aQueue);
    [[nodiscard]] const KeyType firstKey() const override;
    std::vector<MappingType> fMappings;
    Node* fLeftChild;

  private:
    void copyHalfFrom(std::vector<MappingType>& aMappings);
    void copyAllFrom(std::vector<MappingType>& aMappings);
    void copyLastFrom(MappingType aPair);
    void copyFirstFrom(MappingType aPair, int aParentIndex);

};

#endif  // INTERNALNODE_H
