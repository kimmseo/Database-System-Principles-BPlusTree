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
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <vector>
#include "CSV.h"
#include <algorithm>
#include "DiskManager.h"

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

    std::vector<ValueType *> &record = leafNode->lookup(aKey);

    if (!record.empty()) {
        record.push_back(new ValueType(aValue));
        return;
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
    std::vector<ValueType *> &record = leafNode->lookup(aKey);
    if (record.empty()) {
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
LeafNode *BPlusTree::findLeafNodeWithCount(KeyType aKey, int *indexNodeCount, bool aPrinting,
                                           bool aVerbose) {
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
        (*indexNodeCount)++;
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
    // Debug to check number of records in leaf node being inserted
    // std::cout << "Number of records in leaf node: " << getNumberOfRecords(static_cast<LeafNode
    // *>(node)) <<std::endl;
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

    std::vector<ValueType *> &record = leaf->lookup(aKey);
    if (record.empty()) {
        std::cout << "Record not found with key " << aKey << "." << std::endl;
        return;
    }

    if (aPrintPath) {
        std::cout << "\t";
    }

    std::cout << "Records found at location " << std::hex << &record << std::dec << ":"
              << std::endl;
    for (const auto *valuePtr : record) {
        std::cout << "\tKey: " << aKey << "   Value: " << *valuePtr << std::endl;
    }
    std::cout << "Number of records in the block: " << getNumberOfRecords(leaf) << std::endl;
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

QueryStats BPlusTree::rangeWithStats(KeyType aStart, KeyType aEnd) {
    QueryStats stats;

    auto startTime = std::chrono::high_resolution_clock::now();

    auto startLeaf = findLeafNodeWithCount(
        aStart, &stats.indexNodesAccessed);  // Enable verbose to count index nodes
    auto endLeaf = findLeafNodeWithCount(aEnd, &stats.indexNodesAccessed);

    if (!startLeaf || !endLeaf) {
        return stats;  // Return empty stats if range is invalid
    }

    std::vector<std::tuple<KeyType, ValueType, LeafNode *>> entries;
    stats.dataBlocksAccessed = 0;
    double fgsum = 0.0;

    if (startLeaf == endLeaf) {
        startLeaf->copyRange(aStart, aEnd, entries);
        stats.dataBlocksAccessed++;  // Single data block accessed
    } else {
        startLeaf->copyRangeStartingFrom(aStart, entries);
        stats.dataBlocksAccessed++;
        startLeaf = startLeaf->next();

        while (startLeaf && startLeaf != endLeaf) {
            startLeaf->copyFullRange(entries);
            stats.dataBlocksAccessed++;
            startLeaf = startLeaf->next();
        }

        startLeaf->copyRangeUntil(aEnd, entries);
        stats.dataBlocksAccessed++;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.queryTime = std::chrono::duration<double>(endTime - startTime).count();

    // Calculate avg FG_PCT_home
    for (const auto &entry : entries) {
        fgsum += std::get<1>(entry).FG_PCT_home;
    }

    stats.recordCount = entries.size();
    if (stats.recordCount > 0) {
        stats.avgfgpct = fgsum / stats.recordCount;
    }

    return stats;
}

QueryStats BPlusTree::rangeWithStatsV2(KeyType aStart, KeyType aEnd) {
    QueryStats stats;

    auto startTime = std::chrono::high_resolution_clock::now();

    LeafNode *currentLeaf = findLeafNodeWithCount(aStart, &stats.indexNodesAccessed);
    if (!currentLeaf) {
        return stats;  // Return empty stats if no valid starting node
    }

    std::vector<std::tuple<KeyType, ValueType, LeafNode *>> entries;
    stats.dataBlocksAccessed = 0;
    double fgsum = 0.0;

    while (currentLeaf) {
        stats.dataBlocksAccessed++;

        for (const auto &mapping : currentLeaf->getMappings()) {
            KeyType key = mapping.first;

            if (key > aEnd) {
                auto endTime = std::chrono::high_resolution_clock::now();
                stats.queryTime = std::chrono::duration<double>(endTime - startTime).count();
                stats.recordCount = entries.size();
                if (stats.recordCount > 0) {
                    stats.avgfgpct = fgsum / stats.recordCount;
                }
                return stats;
            }

            if (key >= aStart) {
                for (ValueType *valuePtr : mapping.second) {
                    entries.emplace_back(key, *valuePtr, currentLeaf);
                    fgsum += valuePtr->FG_PCT_home;
                }
            }
        }

        currentLeaf = currentLeaf->next();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.queryTime = std::chrono::duration<double>(endTime - startTime).count();
    stats.recordCount = entries.size();
    if (stats.recordCount > 0) {
        stats.avgfgpct = fgsum / stats.recordCount;
    }

    return stats;
}

QueryStats BPlusTree::linearScan(KeyType aStart, KeyType aEnd) {
    QueryStats stats;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Start at the root and traverse down to the leftmost leaf
    Node *node = fRoot;
    while (node && !node->isLeaf()) {
        node = static_cast<InternalNode *>(node)->firstChild();
    }

    LeafNode *leaf = static_cast<LeafNode *>(node);
    double fgsum = 0.0;

    while (leaf) {
        stats.dataBlocksAccessed++;

        for (const auto &mapping : leaf->getMappings()) {
            KeyType key = mapping.first;
            if (key >= aStart && key <= aEnd) {
                for (const ValueType *valuePtr : mapping.second) {
                    fgsum += valuePtr->FG_PCT_home;
                    stats.recordCount++;
                }
            }
        }
        leaf = leaf->next();  // Move to the next leaf
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.queryTime = std::chrono::duration<double>(endTime - startTime).count();

    if (stats.recordCount > 0) {
        stats.avgfgpct = fgsum / stats.recordCount;
    }

    return stats;
}

void BPlusTree::printRangeWithStats(KeyType aStart, KeyType aEnd) {
    QueryStats indexQueryStats = rangeWithStatsV2(aStart, aEnd);
    QueryStats linearScanStats = linearScan(aStart, aEnd);

    std::cout << "\nB+ Tree Indexed Range Query Statistics:\n";
    std::cout << "Index Nodes Accessed: " << indexQueryStats.indexNodesAccessed << "\n";
    std::cout << "Data Blocks Accessed: " << indexQueryStats.dataBlocksAccessed << "\n";
    std::cout << "Avg FG_PCT_home: " << indexQueryStats.avgfgpct << "\n";
    std::cout << "Query Execution Time: " << indexQueryStats.queryTime << " seconds\n";

    std::cout << "\nBrute-Force Linear Scan Statistics:\n";
    std::cout << "Data Blocks Accessed: " << linearScanStats.dataBlocksAccessed << "\n";
    std::cout << "Avg FG_PCT_home: " << linearScanStats.avgfgpct << "\n";
    std::cout << "Query Execution Time: " << linearScanStats.queryTime << " seconds\n";
}

std::vector<BPlusTree::EntryType> BPlusTree::range(KeyType aStart, KeyType aEnd) {
    auto startLeaf = findLeafNode(aStart);
    auto endLeaf = findLeafNode(aEnd);

    if (!startLeaf || !endLeaf) {  // cannot find
        return {};
    }

    std::vector<std::tuple<KeyType, ValueType, LeafNode *>> entries;

    if (startLeaf == endLeaf) {
        startLeaf->copyRange(aStart, aEnd, entries);
        return entries;
    }

    startLeaf->copyRangeStartingFrom(aStart, entries);
    startLeaf = startLeaf->next();

    while (startLeaf && startLeaf != endLeaf) {
        startLeaf->copyFullRange(entries);
        startLeaf = startLeaf->next();
    }

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
            std::optional<float> keyOpt = safeStof(row[keyColumn]);
            if (!keyOpt.has_value()) {
                continue;  // Skip this row
            }

            KeyType key = keyOpt.value();  // Extract valid float value
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
        auto it = std::find_if(
            leafMappings.begin(), leafMappings.end(),
            [&](const LeafNode::MappingType &mapping) { return mapping.first == entry.first; });

        if (it != leafMappings.end()) {
            it->second.push_back(new gameRecord(entry.second));
        } else {
            leafMappings.emplace_back(entry.first,
                                      std::vector<ValueType *>{new gameRecord(entry.second)});
        }
    }

    // Insert remaining keys
    if (!leafMappings.empty()) {
        if (leafMappings.size() <= currentLeaf->minSize()) {
            while (!leafMappings.empty()) {
                KeyType key = leafMappings.front().first;
                std::vector<ValueType *> values = leafMappings.front().second;
                for (ValueType *value : values) {
                    prevLeaf->insert(key, value);
                }
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
            std::optional<float> keyOpt = safeStof(row[keyColumn]);
            if (!keyOpt.has_value()) {
                continue;  // Skip this row
            }

            KeyType key = keyOpt.value();  // Extract valid float value
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

unsigned int BPlusTree::getNumberOfRecords(LeafNode *aLeaf) { return aLeaf->getMappingsSize(); }
void BPlusTree::saveToDisk(const std::string &filename)
{
    if (!fRoot) {
        std::cerr << "Tree is empty, nothing to save.\n";
        return;
    }

    DiskManager dm(filename);

    // 1) BFS to assign nodeIDs
    std::queue<Node*> nodeQ;
    nodeQ.push(fRoot);

    std::unordered_map<Node*, int> nodeIDMap;
    int currentID = 0;

    while (!nodeQ.empty()) {
        Node* node = nodeQ.front();
        nodeQ.pop();

        if (nodeIDMap.find(node) == nodeIDMap.end()) {
            nodeIDMap[node] = currentID++;
            // Debug
            std::cout << "[DEBUG saveToDisk] Assigning nodeID=" << (currentID-1)
                      << " to Node*=" << node 
                      << " (isLeaf=" << node->isLeaf() << ")\n";
        }

        // If internal, push children
        if (!node->isLeaf()) {
            InternalNode* in = static_cast<InternalNode*>(node);
            if (in->firstChild()) {
                nodeQ.push(in->firstChild());
            }
            // We need access to fMappings, so either make it public or
            // create a getter. Here we assume it's accessible:
            for (int i = 0; i < in->size(); i++) {
                Node* child = in->neighbour(i+1);
                if (child && nodeIDMap.find(child) == nodeIDMap.end()) {
                    nodeQ.push(child);
                }
            }
        } else {
            // If leaf, push next leaf
            LeafNode* ln = static_cast<LeafNode*>(node);
            if (ln->next() && nodeIDMap.find(ln->next()) == nodeIDMap.end()) {
                nodeQ.push(ln->next());
            }
        }
    }

    // 2) BFS again to actually write the blocks
    //    (or you can store the BFS results in a vector)
    nodeQ.push(fRoot);
    std::unordered_set<Node*> visited;

    while (!nodeQ.empty()) {
        Node* node = nodeQ.front();
        nodeQ.pop();
        if (visited.find(node) != visited.end()) continue;
        visited.insert(node);

        NodeBlock block;
        int thisID = nodeIDMap[node];
        block.nodeID = thisID;
        block.isLeaf = node->isLeaf();
        block.size = node->size();

        // parentID
        if (node->parent()) {
            block.parentID = nodeIDMap[node->parent()];
        } else {
            block.parentID = -1; // root
        }

        if (block.isLeaf) {
            LeafNode* ln = static_cast<LeafNode*>(node);
            // next leaf
            if (ln->next()) {
                block.nextLeafID = nodeIDMap[ln->next()];
            } else {
                block.nextLeafID = -1;
            }

            // copy leaf keys
            for (int i = 0; i < ln->size(); i++) {
                block.leafKeys[i] = ln->getMappings()[i].first;
                // If you want to store partial data from each record,
                // you'd store it here, e.g. block.leafData[i] = ...
            }
        } else {
            InternalNode* in = static_cast<InternalNode*>(node);
            // left child
            if (in->firstChild()) {
                block.leftChildID = nodeIDMap[in->firstChild()];
            } else {
                block.leftChildID = -1;
            }

            // copy keys + childIDs
            for (int i = 0; i < in->size(); i++) {
                block.keys[i] = in->keyAt(i);
                Node* child = in->neighbour(i+1);
                block.childIDs[i] = (child ? nodeIDMap[child] : -1);
            }
        }

        // Debug print
        std::cout << "[DEBUG saveToDisk] Writing blockID=" << thisID
                  << " isLeaf=" << block.isLeaf
                  << " size=" << block.size
                  << " parentID=" << block.parentID
                  << " nextLeafID=" << block.nextLeafID
                  << " leftChildID=" << block.leftChildID << "\n";
        for (int i = 0; i < block.size; i++) {
            if (block.isLeaf) {
                std::cout << "   leafKeys[" << i << "]=" << block.leafKeys[i] << "\n";
            } else {
                std::cout << "   keys[" << i << "]=" << block.keys[i]
                          << " childIDs[" << i << "]=" << block.childIDs[i] << "\n";
            }
        }

        dm.writeBlock(thisID, block);

        // Push children to BFS
        if (!block.isLeaf) {
            InternalNode* in = static_cast<InternalNode*>(node);
            if (in->firstChild()) nodeQ.push(in->firstChild());
            for (int i = 0; i < in->size(); i++) {
                Node* c = in->neighbour(i+1);
                if (c) nodeQ.push(c);
            }
        } else {
            LeafNode* ln = static_cast<LeafNode*>(node);
            if (ln->next()) nodeQ.push(ln->next());
        }
    }

    std::cout << "[DEBUG saveToDisk] B+ Tree saved to " << filename 
              << " with total blocks=" << currentID << "\n";
}
void BPlusTree::loadFromDisk(const std::string &filename)
{
    DiskManager dm(filename);

    std::vector<NodeBlock> blocks;
    NodeBlock temp;
    int blockID = 0;

    // 1) Read all blocks until readBlock fails
    while (dm.readBlock(blockID, temp)) {
        // debug print what we read
        std::cout << "[DEBUG loadFromDisk] readBlock(" << blockID << "):\n"
                  << "   nodeID=" << temp.nodeID
                  << " isLeaf=" << temp.isLeaf
                  << " size=" << temp.size
                  << " parentID=" << temp.parentID
                  << " nextLeafID=" << temp.nextLeafID
                  << " leftChildID=" << temp.leftChildID << "\n";
        for (int i = 0; i < temp.size; i++) {
            if (temp.isLeaf) {
                std::cout << "      leafKeys[" << i << "]=" << temp.leafKeys[i] << "\n";
            } else {
                std::cout << "      keys[" << i << "]=" << temp.keys[i]
                          << " childIDs[" << i << "]=" << temp.childIDs[i] << "\n";
            }
        }

        blocks.push_back(temp);
        blockID++;
    }

    if (blocks.empty()) {
        std::cerr << "[DEBUG loadFromDisk] No blocks read from " << filename << "\n";
        fRoot = nullptr;
        return;
    }

    // 2) create Node* array
    //    assume nodeIDs go from 0..(blocks.size()-1)
    std::vector<Node*> nodePtr(blocks.size(), nullptr);

    // first pass: allocate LeafNode or InternalNode
    for (auto &b : blocks) {
        Node* newNode = nullptr;
        if (b.isLeaf) {
            newNode = new LeafNode(fOrder);
            std::cout << "[DEBUG loadFromDisk] Creating LeafNode for blockID=" << b.nodeID << "\n";
        } else {
            newNode = new InternalNode(fOrder);
            std::cout << "[DEBUG loadFromDisk] Creating InternalNode for blockID=" << b.nodeID << "\n";
        }
        nodePtr[b.nodeID] = newNode;
    }

    // 3) second pass: fill pointers, keys, etc.
    for (auto &b : blocks) {
        Node* n = nodePtr[b.nodeID];
        // set parent
        if (b.parentID >= 0 && b.parentID < (int)nodePtr.size()) {
            n->setParent(nodePtr[b.parentID]);
        } else {
            // -1 => root
            std::cout << "[DEBUG loadFromDisk] blockID=" << b.nodeID 
                      << " has parentID=" << b.parentID << " => possibly root.\n";
        }

        if (b.isLeaf) {
            LeafNode* ln = static_cast<LeafNode*>(n);
            // next leaf
            if (b.nextLeafID >= 0 && b.nextLeafID < (int)nodePtr.size()) {
                ln->setNext(static_cast<LeafNode*>(nodePtr[b.nextLeafID]));
                std::cout << "[DEBUG loadFromDisk] Leaf " << b.nodeID 
                          << " nextLeaf=" << b.nextLeafID << "\n";
            }
            // rebuild fMappings
            for (int i = 0; i < b.size; i++) {
                float key = b.leafKeys[i];
                // if you stored partial data, you'd reconstruct ValueType
                // for now, just do something like:
                ValueType dummyVal; // or from block
                ln->createAndInsertRecord(key, dummyVal);
            }
        } else {
            InternalNode* in = static_cast<InternalNode*>(n);
            // leftChild
            if (b.leftChildID >= 0 && b.leftChildID < (int)nodePtr.size()) {
                in->fLeftChild = nodePtr[b.leftChildID];
                in->fLeftChild->setParent(in);
                std::cout << "[DEBUG loadFromDisk] Internal " << b.nodeID 
                          << " leftChild=" << b.leftChildID << "\n";
            }
            // fMappings
            for (int i = 0; i < b.size; i++) {
                float key = b.keys[i];
                int cID = b.childIDs[i];
                if (cID >= 0 && cID < (int)nodePtr.size()) {
                    Node* childPtr = nodePtr[cID];
                    childPtr->setParent(in);
                    in->fMappings.push_back({key, childPtr});
                    std::cout << "[DEBUG loadFromDisk] Internal " << b.nodeID 
                              << " key[" << i << "]=" << key
                              << " childID[" << i << "]=" << cID << "\n";
                }
            }
        }
    }

    // 4) find root (the node with parentID = -1)
    fRoot = nullptr;
    for (auto &b : blocks) {
        if (b.parentID < 0) {
            fRoot = nodePtr[b.nodeID];
            std::cout << "[DEBUG loadFromDisk] Found root nodeID=" << b.nodeID << "\n";
            break;
        }
    }

    if (!fRoot) {
        std::cerr << "[DEBUG loadFromDisk] No root found. Possibly corrupt file.\n";
    }

    std::cout << "[DEBUG loadFromDisk] B+ Tree loaded from " << filename << "\n";
}
