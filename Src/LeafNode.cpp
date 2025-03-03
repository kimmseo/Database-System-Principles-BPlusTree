//
// Created by Minseo on 2/7/2025.
//

#include <iostream>
#include <sstream>
#include "Exceptions.h"
#include "InternalNode.h"
#include "LeafNode.h"

LeafNode::LeafNode(int aOrder) : fNext(nullptr), Node(aOrder) {}

LeafNode::LeafNode(int aOrder, Node *aParent) : fNext(nullptr), Node(aOrder, aParent) {}

LeafNode::~LeafNode() {
    for (auto &mapping : fMappings) {
        for (ValueType *valuePtr : mapping.second) {
            delete valuePtr;
        }
    }
}

bool LeafNode::isLeaf() const { return true; }

LeafNode *LeafNode::next() const { return fNext; }

void LeafNode::setNext(LeafNode *aNext) { fNext = aNext; }

int LeafNode::size() const { return static_cast<int>(fMappings.size()); }

int LeafNode::minSize() const {
    // min # of keys for leaf node = floor((maxKey + 1)/2)
    int noOfKeysPerNode = maxSize();
    return (noOfKeysPerNode + 1) / 2;
}

int LeafNode::maxSize() const { return order() - 1; }

std::string LeafNode::toString(bool aVerbose) const {
    std::ostringstream keyToTextConverter;
    if (aVerbose) {
        keyToTextConverter << "[" << std::hex << this << std::dec << "]<" << fMappings.size()
                           << "> ";
    }
    bool first = true;
    for (auto mapping : fMappings) {
        if (first) {
            first = false;
        } else {
            keyToTextConverter << " ";
        }
        keyToTextConverter << mapping.first;
    }
    if (aVerbose) {
        keyToTextConverter << "[" << std::hex << fNext << ">";
    }
    return keyToTextConverter.str();
}

std::vector<LeafNode::MappingType> LeafNode::getMappings() const { return fMappings; }

unsigned int LeafNode::getMappingsSize() const { return fMappings.size(); }

int LeafNode::createAndInsertRecord(KeyType aKey, ValueType aValue) {
    gameRecord *newRecord = new gameRecord(aValue);
    insert(aKey, newRecord);
    return static_cast<int>(fMappings.size());
}

void LeafNode::insert(KeyType aKey, gameRecord *aRecord) {
    auto insertionPoint = fMappings.begin();
    auto end = fMappings.end();
    while (insertionPoint != end && insertionPoint->first < aKey) {
        ++insertionPoint;
    }

    if (insertionPoint != end && insertionPoint->first == aKey) {
        insertionPoint->second.push_back(aRecord);
    } else {
        fMappings.insert(insertionPoint, MappingType(aKey, {aRecord}));
    }
}

void LeafNode::bulkInsert(const std::vector<MappingType> &sortedMappings) {
    fMappings = sortedMappings;
}

std::vector<ValueType *> &LeafNode::lookup(KeyType aKey) {
    for (auto &mapping : fMappings) {
        if (mapping.first == aKey) {
            return mapping.second;
        }
    }

    static std::vector<ValueType *> emptyVector;
    return emptyVector;
}

void LeafNode::copyRangeStartingFrom(KeyType aKey, std::vector<EntryType> &aVector) {
    // Debug
    //  std::cout << "Debug: Available keys in this leaf: ";
    //  for (const auto &mapping : fMappings) {
    //      std::cout << mapping.first << " ";
    //  }
    //  std::cout << std::endl;

    bool startCopying = false;

    for (const auto &mapping : fMappings) {
        KeyType key = mapping.first;

        if (!startCopying && key >= aKey) {
            startCopying = true;  // Start copying once reach the first valid key
        }
        if (startCopying) {
            for (ValueType *valuePtr : mapping.second) {
                aVector.push_back(std::make_tuple(key, *valuePtr, this));
            }
        }
    }
}

void LeafNode::copyRangeUntil(KeyType aKey, std::vector<EntryType> &aVector) {
    // Debug
    //  std::cout << "Debug: Available keys in this leaf: ";
    //  for (const auto &mapping : fMappings) {
    //      std::cout << mapping.first << " ";
    //  }
    //  std::cout << std::endl;

    bool startCopying = false;

    for (const auto &mapping : fMappings) {
        KeyType key = mapping.first;

        if (!startCopying && key <= aKey) {
            startCopying = true;
        }
        if (!startCopying) continue;

        if (key > aKey) {
            break;
        }
        for (ValueType *valuePtr : mapping.second) {
            aVector.push_back(std::make_tuple(key, *valuePtr, this));
        }
    }
}

void LeafNode::copyFullRange(std::vector<EntryType> &aVector) {
    for (auto mapping : fMappings) {
        for (ValueType *valuePtr : mapping.second) {
            aVector.push_back(std::make_tuple(mapping.first, *valuePtr, this));
        }
    }
}

void LeafNode::copyRange(KeyType aStart, KeyType aEnd, std::vector<EntryType> &aVector) {
    for (const auto &mapping : fMappings) {
        KeyType key = mapping.first;

        if (key < aStart) continue;  // Ignore keys smaller than aStart
        if (key > aEnd) {            // Stop copying if key is larger than aEnd
            break;
        }
        for (ValueType *valuePtr : mapping.second) {
            aVector.push_back(std::make_tuple(key, *valuePtr, this));
        }
    }
}

int LeafNode::removeAndDeleteRecord(KeyType aKey) {
    auto removalPoint = fMappings.begin();
    auto end = fMappings.end();

    while (removalPoint != end && removalPoint->first != aKey) {
        ++removalPoint;
    }

    if (removalPoint == end) {
        throw RecordNotFoundException(aKey);
    }

    for (ValueType *valuePtr : removalPoint->second) {
        delete valuePtr;
    }

    fMappings.erase(removalPoint);
    return static_cast<int>(fMappings.size());
}

const KeyType LeafNode::firstKey() const { return fMappings[0].first; }

void LeafNode::moveHalfTo(LeafNode *aRecipient) {
    aRecipient->copyHalfFrom(fMappings);
    size_t size = fMappings.size();
    for (size_t i = minSize(); i < size; ++i) {
        fMappings.pop_back();
    }
}

void LeafNode::copyHalfFrom(std::vector<std::pair<KeyType, std::vector<gameRecord *>>> &aMappings) {
    for (size_t i = minSize(); i < aMappings.size(); ++i) {
        std::vector<gameRecord *> copiedValues;
        for (gameRecord *record : aMappings[i].second) {
            copiedValues.push_back(new gameRecord(*record));
        }
        fMappings.emplace_back(aMappings[i].first, copiedValues);
    }
}

void LeafNode::moveAllTo(LeafNode *aRecipient, int) {
    aRecipient->copyHalfFrom(fMappings);
    fMappings.clear();
    aRecipient->setNext(next());
}

void LeafNode::copyAllFrom(std::vector<std::pair<KeyType, std::vector<gameRecord *>>> &aMappings) {
    for (const auto &mapping : aMappings) {
        std::vector<gameRecord *> copiedValues;
        for (gameRecord *record : mapping.second) {
            copiedValues.push_back(new gameRecord(*record));
        }
        fMappings.emplace_back(mapping.first, copiedValues);
    }
}

void LeafNode::moveFirstToEndOf(LeafNode *aRecipient) {
    aRecipient->copyLastFrom(fMappings.front());
    fMappings.erase(fMappings.begin());
    static_cast<InternalNode *>(parent())->setKeyAt(1, fMappings.front().first);
}

void LeafNode::copyLastFrom(MappingType aPair) { fMappings.push_back(aPair); }

void LeafNode::moveLastToFrontOf(LeafNode *aRecipient, int aParentIndex) {
    aRecipient->copyFirstFrom(fMappings.back(), aParentIndex);
    fMappings.pop_back();
}

void LeafNode::copyFirstFrom(MappingType aPair, int aParentIndex) {
    fMappings.insert(fMappings.begin(), aPair);
    static_cast<InternalNode *>(parent())->setKeyAt(aParentIndex, fMappings.front().first);
}
