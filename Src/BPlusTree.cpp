//
// Created by Minseo on 2/7/2025.
//

#include <iostream>
#include <fstream>
#include <string>
#include "BPlusTree.h"
#include "Exceptions.h"
#include "InternalNode.h"
#include "LeafNode.h"
#include "Node.h"
#include "CSV.h"
#include <chrono>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <vector>

BPlusTree::BPlusTree(int aOrder) : fOrder{aOrder}, fRoot{nullptr} {}

bool BPlusTree::isEmpty() const { return !fRoot; }

// Insertion

void BPlusTree::insert(KeyType aKey, ValueType aValue) {
    if (isEmpty()) {
        startNewTree(aKey, aValue);
    } else {
        insertIntoLeaf(aKey, aValue);
    }
}

void BPlusTree::startNewTree(KeyType aKey, ValueType aValue) {
    LeafNode *newLeafNode = new LeafNode(fOrder);
    newLeafNode->createAndInsertRecord(aKey, aValue);
    fRoot = newLeafNode;
}

void BPlusTree::insertIntoLeaf(KeyType aKey, ValueType aValue) {
    LeafNode *leafNode = findLeafNode(aKey);
    if (!leafNode) {
        std::cerr << "Error: Leaf node not found for key " << aKey << std::endl;
        throw LeafNotFoundException(aKey);
    }

    int newSize = leafNode->createAndInsertRecord(aKey, aValue);

    if (newSize > leafNode->maxSize()) {
        LeafNode *newLeaf = split(leafNode);
        if (!newLeaf) {
            return;
        }
        // Debug
        // std::cout << "New leaf created: " << newLeaf << std::endl;

        newLeaf->setNext(leafNode->next());
        leafNode->setNext(newLeaf);

        KeyType newKey = newLeaf->firstKey();
        // Debug
        // std::cout << "New key for parent insertion: " << newKey << std::endl;

        insertIntoParent(leafNode, newKey, newLeaf);
    }
}

void BPlusTree::insertIntoParent(Node *aOldNode, KeyType aKey, Node *aNewNode) {
    InternalNode *parent = static_cast<InternalNode *>(aOldNode->parent());
    if (parent == nullptr) {
        fRoot = new InternalNode(fOrder);
        parent = static_cast<InternalNode *>(fRoot);
        aOldNode->setParent(parent);
        aNewNode->setParent(parent);
        parent->populateNewRoot(aOldNode, aKey, aNewNode);
    } else {
        int newSize = parent->insertNodeAfter(aOldNode, aKey, aNewNode);
        if (newSize > parent->maxSize()) {
            InternalNode *newNode = split(parent);
            KeyType newKey = newNode->replaceAndReturnFirstKey();
            insertIntoParent(parent, newKey, newNode);
        }
    }
}

template <typename T>
T *BPlusTree::split(T *aNode) {
    T *newNode = new T(fOrder, aNode->parent());
    aNode->moveHalfTo(newNode);
    return newNode;
}

// Removal

void BPlusTree::remove(KeyType aKey) {
    if (isEmpty()) {
        return;
    } else {
        removeFromLeaf(aKey);
    }
}

void BPlusTree::removeFromLeaf(KeyType aKey) {
    LeafNode *leafNode = findLeafNode(aKey);
    if (!leafNode) {
        return;
    }
    if (!leafNode->lookup(aKey)) {
        return;
    }
    int newSize = leafNode->removeAndDeleteRecord(aKey);
    if (newSize < leafNode->minSize()) {
        coalesceOrRedistribute(leafNode);
    }
}

template <typename N>
void BPlusTree::coalesceOrRedistribute(N *aNode) {
    if (aNode->isRoot()) {
        adjustRoot();
        return;
    }
    auto parent = static_cast<InternalNode *>(aNode->parent());
    int indexOfNodeInParent = parent->nodeIndex(aNode);
    int neighborIndex = (indexOfNodeInParent == 0) ? 1 : indexOfNodeInParent - 1;
    N *neighborNode = static_cast<N *>(parent->neighbour(neighborIndex));
    if (aNode->size() + neighborNode->size() <= neighborNode->maxSize()) {
        coalesce(neighborNode, aNode, parent, indexOfNodeInParent);
    } else {
        redistribute(neighborNode, aNode, parent, indexOfNodeInParent);
    }
}

template <typename N>
void BPlusTree::coalesce(N *aNeighborNode, N *aNode, InternalNode *aParent, int aIndex) {
    if (aIndex == 0) {
        std::swap(aNode, aNeighborNode);
        aIndex = 1;
    }
    aNode->moveAllTo(aNeighborNode, aIndex);
    aParent->remove(aIndex);
    if (aParent->size() < aParent->minSize()) {
        coalesceOrRedistribute(aParent);
    }
    delete aNode;
}

template <typename N>
void BPlusTree::redistribute(N *aNeighborNode, N *aNode, InternalNode *aParent, int aIndex) {
    if (aIndex == 0) {
        aNeighborNode->moveFirstToEndOf(aNode);
    } else {
        aNeighborNode->moveLastToFrontOf(aNode, aIndex);
    }
}

void BPlusTree::adjustRoot() {
    if (!fRoot->isLeaf() && fRoot->size() == 1) {
        auto discardedNode = static_cast<InternalNode *>(fRoot);
        fRoot = static_cast<InternalNode *>(fRoot)->removeAndReturnOnlyChild();
        fRoot->setParent(nullptr);
        delete discardedNode;
    } else if (!fRoot->size()) {
        delete fRoot;
        fRoot = nullptr;
    }
}

// Utilitise and printing

LeafNode *BPlusTree::findLeafNode(KeyType aKey, bool aPrinting, bool aVerbose) {
    if (isEmpty()) {
        if (aPrinting) {
            std::cout << "Not found: empty tree." << std::endl;
        }
        return nullptr;
    }

    auto node = fRoot;

    if (aPrinting) {
        std::cout << "Root: ";
        if (fRoot->isLeaf()) {
            std::cout << "\t" << static_cast<LeafNode *>(fRoot)->toString(aVerbose);
        } else {
            std::cout << "\t" << static_cast<InternalNode *>(fRoot)->toString(aVerbose);
        }
        std::cout << std::endl;
    }

    // Added visited nodes tracking to detect infinite loops, since data is large and terminal may
    // explode
    std::unordered_set<Node *> visitedNodes;

    while (!node->isLeaf()) {
        // Track if node visited.
        if (visitedNodes.find(node) != visitedNodes.end()) {
            std::cerr << "ERROR: Infinite loop detected in findLeafNode()! Node already visited: "
                      << node << std::endl;
            return nullptr;
        }
        visitedNodes.insert(node);

        auto internalNode = static_cast<InternalNode *>(node);
        // Debug
        // std::cout << "Current internal node: " << internalNode->firstKey() << std::endl;

        Node *nextNode = internalNode->lookup(aKey);
        // Debug
        // std::cout << "Next node: " << nextNode << std::endl;

        if (nextNode == nullptr) {
            std::cerr << "ERROR: lookup() returned nullptr for key " << aKey << std::endl;
            return nullptr;
        }

        node = nextNode;
    }

    return static_cast<LeafNode *>(node);
}

void BPlusTree::readInputFromFile(std::string aFileName) {
    std::ifstream inputFile(aFileName);
    if (!inputFile) {
        std::cerr << "Error: Could not open file " << aFileName << std::endl;
        return;
    }

    float key;
    while (inputFile >> key) {
        gameRecord record;    // Create a default gameRecord
        insert(key, record);  // Insert with a placeholder record
    }

    std::cout << "Loaded data from " << aFileName << " into B+ Tree." << std::endl;
}

void BPlusTree::print(bool aVerbose) {
    fPrinter.setVerbose(aVerbose);
    fPrinter.printTree(fRoot);
}

void BPlusTree::printLeaves(bool aVerbose) {
    fPrinter.setVerbose(aVerbose);
    fPrinter.printLeaves(fRoot);
}

void BPlusTree::destroyTree() {
    if (fRoot->isLeaf()) {
        delete static_cast<LeafNode *>(fRoot);
    } else {
        delete static_cast<InternalNode *>(fRoot);
    }
    fRoot = nullptr;
}

void BPlusTree::printValue(KeyType aKey, bool aVerbose) { printValue(aKey, false, aVerbose); }

void BPlusTree::printValue(KeyType aKey, bool aPrintPath, bool aVerbose) {
    LeafNode *leaf = findLeafNode(aKey, aPrintPath, aVerbose);
    if (!leaf) {
        std::cout << "Leaf not found with key " << aKey << "." << std::endl;
        return;
    }
    if (aPrintPath) {
        std::cout << "\t";
    }
    std::cout << "Leaf: " << leaf->toString(aVerbose) << std::endl;
    gameRecord *record = leaf->lookup(aKey);
    if (!record) {
        std::cout << "Record not found with key " << aKey << "." << std::endl;
        return;
    }
    if (aPrintPath) {
        std::cout << "\t";
    }
    std::cout << "Record found at location " << std::hex << record << std::dec << ":" << std::endl;
    std::cout << "\tKey: " << aKey << "   Value: " << record << std::endl;
}

void BPlusTree::printPathTo(KeyType aKey, bool aVerbose) { printValue(aKey, true, aVerbose); }

void BPlusTree::printRange(KeyType aStart, KeyType aEnd) {
    auto rangeVector = range(aStart, aEnd);
    for (auto entry : rangeVector) {
        std::cout << "Key: " << std::get<0>(entry);
        std::cout << "    Value: " << std::get<1>(entry);
        std::cout << "    Leaf: " << std::hex << std::get<2>(entry) << std::dec << std::endl;
    }
}

std::vector<BPlusTree::EntryType> BPlusTree::range(KeyType aStart, KeyType aEnd) {
    auto startLeaf = findLeafNode(aStart);
    auto endLeaf = findLeafNode(aEnd);
    std::vector<std::tuple<KeyType, ValueType, LeafNode *>> entries;
    if (!startLeaf || !endLeaf) {
        return entries;
    }
    startLeaf->copyRangeStartingFrom(aStart, entries);
    startLeaf = startLeaf->next();
    while (startLeaf != endLeaf) {
        //std::cout<<"debug check\n";
        startLeaf->copyRange(entries);
        if (startLeaf->next() == nullptr) { break; }
        startLeaf = startLeaf->next();
    }
    //std::cout<<"while loop outside, debugging\n";
    startLeaf->copyRangeUntil(aEnd, entries);
    return entries;
}

void BPlusTree::printTreeInfo() {
    if (!fRoot) {
        std::cout << "Empty tree.\n";
        return;
    }

    std::queue<Node *> nodeQueue;
    nodeQueue.push(fRoot);
    int level = 0;
    int totalNodes = 0;

    while (!nodeQueue.empty()) {
        int nodeCount = nodeQueue.size();
        totalNodes += nodeCount;

        for (int i = 0; i < nodeCount; ++i) {
            Node *currentNode = nodeQueue.front();
            nodeQueue.pop();

            if (auto *internalNode = dynamic_cast<InternalNode *>(currentNode)) {
                internalNode->queueUpChildren(&nodeQueue);
            }
            // Ignore for leaf node it has no children to queue.
        }
        ++level;
    }

    std::cout << "Total Levels: " << level << "\n";
    std::cout << "Total Nodes: " << totalNodes << "\n";

    std::cout << "Root Node Content:\n[Root] Keys: ";
    if (!fRoot->isLeaf()) {
        auto *rootInternal = static_cast<InternalNode *>(fRoot);
        for (int i = 0; i < rootInternal->size(); ++i) {
            std::cout << rootInternal->keyAt(i) << " ";
        }
    } else {
        auto *rootLeaf = static_cast<LeafNode *>(fRoot);
        while (rootLeaf) {
            std::cout << rootLeaf->toString();
            rootLeaf = rootLeaf->next();
        }
    }
    std::cout << "" << std::endl;
}

#include "CSV.h"
#include <algorithm>

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";  // Empty string if all spaces
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

double BPlusTree::bulkLoadFromCSV(const std::string &filename, int keyColumn) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the CSV file: " << filename << std::endl;
        return -1.0;
    }

    std::vector<std::pair<KeyType, ValueType>> data;
    std::string line;

    std::getline(file, line);  // Skip header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;

        while (std::getline(ss, cell, '\t')) {
            row.push_back(trim(cell));
        }

        if (row.size() != 9) {
            std::cerr << "Invalid row: Expected 9 columns, found " << row.size() << " -> " << line
                      << std::endl;
            continue;
        }

        try {
            KeyType key = safeStof(row[keyColumn]);
            ValueType record(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7],
                             row[8]);
            data.emplace_back(key, record);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing row: " << e.what() << " -> " << line << std::endl;
        }
    }
    file.close();

    std::cout << "Finished reading " << data.size() << " records. Sorting now...\n";

    // Step 1: Sort Data before bulk loading
    std::sort(data.begin(), data.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    auto sortingEndTime = std::chrono::high_resolution_clock::now();
    std::cout << "Sorting completed. Inserting into B+ Tree...\n";

    // Start timing bulk load
    auto startBulk = std::chrono::high_resolution_clock::now();

    // Step 2: create leafnodes
    std::vector<Node *> leafNodes;
    LeafNode *currentLeaf = new LeafNode(fOrder);
    LeafNode *prevLeaf;
    std::vector<LeafNode::MappingType> leafMappings;

    for (const auto &entry : data) {
        if (leafMappings.size() == currentLeaf->maxSize()) {
            currentLeaf->bulkInsert(leafMappings);
            leafNodes.push_back(currentLeaf);
            leafMappings.clear();

            LeafNode *newLeaf = new LeafNode(fOrder);
            currentLeaf->setNext(newLeaf);
            prevLeaf = currentLeaf;
            currentLeaf = newLeaf;
        }
        leafMappings.emplace_back(entry.first, new gameRecord(entry.second));
    }

    // Insert remaining keys
    if (!leafMappings.empty()) {
        if (leafMappings.size() <= currentLeaf->minSize()) {
            while (!leafMappings.empty()) {
                prevLeaf->insert(leafMappings.front().first, leafMappings.front().second);
                leafMappings.erase(leafMappings.begin());

                if (prevLeaf->size() > prevLeaf->maxSize()) {
                    LeafNode *newNode = new LeafNode(fOrder);
                    prevLeaf->moveHalfTo(newNode);
                    prevLeaf->setNext(newNode);
                    prevLeaf = newNode;  // Move to the new leaf
                }
            }
            leafNodes.push_back(prevLeaf);
        } else {
            currentLeaf->bulkInsert(leafMappings);
            leafNodes.push_back(currentLeaf);
        }
        leafMappings.clear();
    }
    fRoot = leafNodes[0];

    // Step 3: Build Internal Nodes from LeafNodes
    for (size_t i = 1; i < leafNodes.size(); i++) {
        KeyType separatorKey = leafNodes[i]->firstKey();
        insertIntoParent(leafNodes[i - 1], separatorKey, leafNodes[i]);
    }

    auto endBulk = std::chrono::high_resolution_clock::now();
    double bulkLoadTime = std::chrono::duration<double>(endBulk - startBulk).count();

    return bulkLoadTime;  // Return bulk load time
}

double BPlusTree::normalInsertFromCSV(const std::string &filename, int keyColumn) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the CSV file: " << filename << std::endl;
        return -1.0;
    }

    std::vector<std::pair<KeyType, ValueType>> data;
    std::string line;

    std::getline(file, line);  // Skip header

    auto startNormalInsert = std::chrono::high_resolution_clock::now();

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;

        while (std::getline(ss, cell, '\t')) {
            row.push_back(trim(cell));
        }

        if (row.size() != 9) {
            std::cerr << "Invalid row: Expected 9 columns, found " << row.size() << " -> " << line
                      << std::endl;
            continue;
        }

        try {
            KeyType key = safeStof(row[keyColumn]);
            ValueType record(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7],
                             row[8]);
            insert(key, record);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing row: " << e.what() << " -> " << line << std::endl;
        }
    }

    file.close();

    auto endNormalInsert = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(endNormalInsert - startNormalInsert).count();
}
