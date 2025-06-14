//
// Created by Minseo on 2/7/2025.
//

#ifndef LEAFNODE_H
#define LEAFNODE_H

#include <tuple>
#include <utility>
#include <vector>
#include "Node.h"

class LeafNode : public Node {
  public:
    explicit LeafNode(int aOrder);
    explicit LeafNode(int aOrder, Node* aParent);
    ~LeafNode() override;
    using MappingType = std::pair<KeyType, std::vector<ValueType*>>;
    using EntryType = std::tuple<KeyType, ValueType, LeafNode*>;
    [[nodiscard]] bool isLeaf() const override;
    [[nodiscard]] LeafNode* next() const;
    void setNext(LeafNode* aNext);
    [[nodiscard]] int size() const override;
    [[nodiscard]] int minSize() const override;
    [[nodiscard]] int maxSize() const override;
    int createAndInsertRecord(KeyType aKey, ValueType aValue);
    void insert(KeyType aKey, gameRecord* aRecord);
    void bulkInsert(const std::vector<MappingType>& sortedMappings);
    std::vector<ValueType*>& lookup(KeyType aKey);
    int removeAndDeleteRecord(KeyType aKey);
    [[nodiscard]] const KeyType firstKey() const override;
    void moveHalfTo(LeafNode* aRecipient);
    void moveAllTo(LeafNode* aRecipient, int /* not used */);
    void moveFirstToEndOf(LeafNode* aRecipient);
    void moveLastToFrontOf(LeafNode* aRecipient, int aParentIndex);
    void copyRangeStartingFrom(KeyType aKey, std::vector<EntryType>& aVector);
    void copyRangeUntil(KeyType aKey, std::vector<EntryType>& aVector);
    void copyRange(KeyType aStart, KeyType aEnd, std::vector<EntryType>& aVector);
    void copyFullRange(std::vector<EntryType>& aVector);
    [[nodiscard]] std::string toString(bool aVerbose = false) const override;
    std::vector<MappingType> getMappings() const;
    unsigned int getMappingsSize() const;

  private:
    void copyHalfFrom(std::vector<MappingType>& aMappings);
    void copyAllFrom(std::vector<MappingType>& aMappings);
    void copyLastFrom(MappingType aPair);
    void copyFirstFrom(MappingType aPair, int aParentIndex);
    std::vector<MappingType> fMappings;
    LeafNode* fNext;
};

#endif  // LEAFNODE_H
