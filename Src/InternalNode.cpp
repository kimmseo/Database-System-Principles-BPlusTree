// InternalNode.cpp

#include <iostream>
#include <sstream>
#include <queue>
#include "Exceptions.h"
#include "InternalNode.h"

InternalNode::InternalNode(int aOrder) : Node(aOrder), fLeftChild(nullptr) {}

InternalNode::InternalNode(int aOrder, Node* aParent)
    : Node(aOrder, aParent), fLeftChild(nullptr) {}

InternalNode::~InternalNode() {
    // Clean up left child
    delete fLeftChild;

    // Clean up children in fMappings
    for (auto& mapping : fMappings) {
        delete mapping.second;
    }
}

bool InternalNode::isLeaf() const { return false; }

int InternalNode::size() const {
    // The "size" is the number of real keys in fMappings
    return static_cast<int>(fMappings.size());
}

int InternalNode::minSize() const {
    // Same logic as your original: floor(maxSize()/2)
    return maxSize() / 2;
}

int InternalNode::maxSize() const {
    // Usually order() - 1, as in your code
    return order() - 1;
}

KeyType InternalNode::keyAt(int aIndex) const { return fMappings[aIndex].first; }

void InternalNode::setKeyAt(int aIndex, KeyType aKey) { fMappings[aIndex].first = aKey; }

Node* InternalNode::firstChild() const {
    // Return the leftmost child pointer
    return fLeftChild;
}

void InternalNode::populateNewRoot(Node* aOldNode, KeyType aNewKey, Node* aNewNode) {
    // The old node becomes the left child
    fLeftChild = aOldNode;
    fLeftChild->setParent(this);

    // Insert one real key for the new node
    fMappings.push_back(std::make_pair(aNewKey, aNewNode));
    aNewNode->setParent(this);
}

int InternalNode::insertNodeAfter(Node* aOldNode, KeyType aNewKey, Node* aNewNode) {
    // If the old node is the left child, insert at the front of fMappings
    if (aOldNode == fLeftChild) {
        // We want the new key/child to appear as the first pair
        fMappings.insert(fMappings.begin(), std::make_pair(aNewKey, aNewNode));
    } else {
        // Otherwise, find aOldNode in fMappings
        int index = -1;
        for (int i = 0; i < (int)fMappings.size(); i++) {
            if (fMappings[i].second == aOldNode) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            throw NodeNotFoundException(aOldNode->toString(), toString());
        }
        // Insert right after that index
        fMappings.insert(fMappings.begin() + index + 1, std::make_pair(aNewKey, aNewNode));
    }
    aNewNode->setParent(this);
    return size();
}

void InternalNode::remove(int aIndex) {
    // Remove the (key, child) pair at aIndex
    fMappings.erase(fMappings.begin() + aIndex);
}

Node* InternalNode::removeAndReturnOnlyChild() {
    // If there are no real keys, the only child is fLeftChild
    if (fMappings.empty()) {
        Node* onlyChild = fLeftChild;
        fLeftChild = nullptr;
        return onlyChild;
    }
    // Otherwise, you might want to handle the case of "only one real key"
    // ...
    return nullptr;
}

KeyType InternalNode::replaceAndReturnFirstKey() {
    // Get the first key before modifying the mappings
    KeyType newKey = fMappings[0].first;

    // Instead of erasing the first key, shift children correctly
    fLeftChild = fMappings[0].second;  // Move first child up

    // Remove the first (key, child) pair, but keep the structure
    fMappings.erase(fMappings.begin());

    return newKey;  // Return the key to be inserted into the parent
}

void InternalNode::moveHalfTo(InternalNode* aRecipient) {
    // Example: move half the pairs
    size_t total = fMappings.size();
    size_t half = total / 2;

    // Move from [half..end) to aRecipient
    for (size_t i = half; i < total; i++) {
        aRecipient->fMappings.push_back(fMappings[i]);
        aRecipient->fMappings.back().second->setParent(aRecipient);
    }
    // Erase them from this node
    fMappings.erase(fMappings.begin() + half, fMappings.end());

    // Adjust fLeftChild or aRecipient->fLeftChild as needed
    // (Depends on how you handle the "split" boundary)
}

void InternalNode::moveAllTo(InternalNode* aRecipient, int /*aParentIndex*/) {
    // If aRecipient has no children yet, set its left child
    if (aRecipient->fMappings.empty() && !aRecipient->fLeftChild) {
        aRecipient->fLeftChild = fLeftChild;
        if (aRecipient->fLeftChild) {
            aRecipient->fLeftChild->setParent(aRecipient);
        }
    }
    // Move all fMappings
    for (auto& m : fMappings) {
        aRecipient->fMappings.push_back(m);
        m.second->setParent(aRecipient);
    }
    fMappings.clear();
    fLeftChild = nullptr;
}

void InternalNode::moveFirstToEndOf(InternalNode* aRecipient) {
    // Move the first (key, child) in fMappings to the end of aRecipient->fMappings
    aRecipient->fMappings.push_back(fMappings.front());
    aRecipient->fMappings.back().second->setParent(aRecipient);
    fMappings.erase(fMappings.begin());
}

void InternalNode::moveLastToFrontOf(InternalNode* aRecipient, int /*aParentIndex*/) {
    // Move the last (key, child) in fMappings to the front of aRecipient->fMappings
    aRecipient->fMappings.insert(aRecipient->fMappings.begin(), fMappings.back());
    aRecipient->fMappings.front().second->setParent(aRecipient);
    fMappings.pop_back();
}

Node* InternalNode::lookup(KeyType aKey) const {
    // If no real keys, go left
    if (fMappings.empty()) {
        return fLeftChild;
    }

    // If aKey < the first real key, go left
    if (aKey < fMappings[0].first) {
        return fLeftChild;
    }

    // Otherwise, find the correct child
    for (size_t i = 1; i < fMappings.size(); i++) {
        if (aKey < fMappings[i].first) {
            // Child is the one at i-1
            return fMappings[i - 1].second;
        }
    }
    // If >= all keys, go to the rightmost child
    return fMappings.back().second;
}

int InternalNode::nodeIndex(Node* aNode) const {
    // If it's the left child, index is 0
    if (fLeftChild == aNode) {
        return 0;
    }
    // Otherwise, find it in fMappings
    for (int i = 0; i < (int)fMappings.size(); i++) {
        if (fMappings[i].second == aNode) {
            // If left child is "index 0", then the first pair is "index 1", etc.
            return i + 1;
        }
    }
    throw NodeNotFoundException(aNode->toString(), toString());
}

Node* InternalNode::neighbour(int aIndex) const {
    // aIndex == 0 => left child
    if (aIndex == 0) {
        return fLeftChild;
    }
    // Otherwise fMappings[aIndex-1].second
    return fMappings[aIndex - 1].second;
}

std::string InternalNode::toString(bool aVerbose) const {
    std::ostringstream oss;
    if (aVerbose) {
        oss << "[" << std::hex << this << std::dec << "]<" << size() << "> ";
    }
    bool first = true;
    for (auto& m : fMappings) {
        if (!first) {
            oss << " ";
        }
        oss << m.first;
        if (aVerbose) {
            oss << "(" << std::hex << m.second << std::dec << ")";
        }
        first = false;
    }
    return oss.str();
}

void InternalNode::queueUpChildren(std::queue<Node*>* aQueue) {
    // Push the left child
    if (fLeftChild) {
        aQueue->push(fLeftChild);
    }
    // Then push all children in fMappings
    for (auto& mapping : fMappings) {
        if (mapping.second) {
            aQueue->push(mapping.second);
        }
    }
}

const KeyType InternalNode::firstKey() const {
    // If empty, there's no "first key"
    if (fMappings.empty()) {
        return 0;  // or some sentinel
    }
    return fMappings[0].first;
}

void InternalNode::copyHalfFrom(std::vector<MappingType>& aMappings) {
    // For splitting. Example approach:
    size_t total = aMappings.size();
    size_t half = total / 2;
    for (size_t i = half; i < total; i++) {
        fMappings.push_back(aMappings[i]);
        fMappings.back().second->setParent(this);
    }
}

void InternalNode::copyAllFrom(std::vector<MappingType>& aMappings) {
    for (auto& m : aMappings) {
        fMappings.push_back(m);
        m.second->setParent(this);
    }
}

void InternalNode::copyLastFrom(MappingType aPair) {
    fMappings.push_back(aPair);
    fMappings.back().second->setParent(this);
}

void InternalNode::copyFirstFrom(MappingType aPair, int /*aParentIndex*/) {
    fMappings.insert(fMappings.begin(), aPair);
    fMappings.front().second->setParent(this);
}
