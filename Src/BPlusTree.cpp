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

BPlusTree::BPlusTree(int aOrder) : fOrder{aOrder}, fRoot{nullptr} {}

bool BPlusTree::isEmpty() const { return !fRoot; }


//function to save to Disk
void BPlusTree::saveToDisk(const std::string &filename)
{
    if (!fRoot) {
        std::cerr << "Tree is empty, nothing to save.\n";
        return;
    }

    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Cannot open " << filename << " for writing.\n";
        return;
    }

    // Write the root node's ID first
    out << fRoot->nodeID << "\n";

    // BFS
    std::queue<Node*> nodeQ;
    nodeQ.push(fRoot);

    while (!nodeQ.empty()) {
        Node* current = nodeQ.front();
        nodeQ.pop();

        // 1) nodeID
        out << current->nodeID << " ";

        // 2) isLeaf? (1 or 0)
        bool leaf = current->isLeaf();
        out << (leaf ? 1 : 0) << " ";

        // 3) size (# of keys)
        int sz = current->size();
        out << sz << " ";

        if (leaf) {
            // LeafNode
            LeafNode* ln = static_cast<LeafNode*>(current);

            // 4) Write the keys
            for (int i = 0; i < ln->size(); i++) {
                // We can read directly from ln->fMappings if we add a getter, or 
                // do something like ln->keyAt(i) if you add that method.
                // For now, let's do a quick hack:
                KeyType key = ln->fMappings[i].first; // If you make fMappings public or add a getter
                out << key << " ";
            }

            // 5) Write next leaf ID (or -1 if none)
            LeafNode* nxt = ln->next();
            out << (nxt ? nxt->nodeID : -1) << " ";

        } else {
            // InternalNode
            InternalNode* in = static_cast<InternalNode*>(current);

            // 4) Write the keys
            for (int i = 0; i < sz; i++) {
                KeyType key = in->keyAt(i);
                out << key << " ";
            }

            // 5) Write child IDs: leftChild + each in fMappings
            Node* leftChild = in->firstChild();
            out << (leftChild ? leftChild->nodeID : -1) << " ";
            if (leftChild) nodeQ.push(leftChild);

            // For each (key, child) in fMappings
            for (int i = 0; i < sz; i++) {
                Node* child = in->neighbour(i+1);
                out << (child ? child->nodeID : -1) << " ";
                if (child) nodeQ.push(child);
            }
        }

        out << "\n"; // end of this node's line
    }

    out.close();
    std::cout << "B+ Tree saved to " << filename << std::endl;
}

void BPlusTree::loadFromDisk(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open " << filename << " for reading.\n";
        return;
    }

    int rootID;
    in >> rootID;
    if (!in) {
        std::cerr << "File " << filename << " is empty or corrupt.\n";
        return;
    }

    // Temporary struct to store each node's line
    struct NodeDiskData {
        int nodeID;
        bool isLeaf;
        int size;
        std::vector<KeyType> keys;   // 'size' keys
        int nextLeafID;             // only if leaf
        std::vector<int> childIDs;  // left child + size children if internal
    };

    std::unordered_map<int, NodeDiskData> diskMap;

    // Read each node line
    while (!in.eof()) {
        NodeDiskData nd;
        if (!(in >> nd.nodeID)) break;  // no more lines
        int leafInt;
        in >> leafInt;
        nd.isLeaf = (leafInt != 0);

        in >> nd.size;
        nd.keys.resize(nd.size);
        for (int i = 0; i < nd.size; i++) {
            in >> nd.keys[i];
        }

        if (nd.isLeaf) {
            // For a leaf: read nextLeafID
            in >> nd.nextLeafID;
        } else {
            // For an internal node: read leftChild + 'size' children => total of size+1
            nd.childIDs.resize(nd.size + 1);
            for (int i = 0; i < nd.size + 1; i++) {
                in >> nd.childIDs[i];
            }
        }

        diskMap[nd.nodeID] = nd;
    }
    in.close();

    // Create fresh nodes in memory for each nodeID
    std::unordered_map<int, Node*> nodePtrMap;
    for (auto &pair : diskMap) {
        const auto &nd = pair.second;
        Node* node = nullptr;
        if (nd.isLeaf) {
            node = new LeafNode(fOrder);
        } else {
            node = new InternalNode(fOrder);
        }
        // If you want each Node to remember its nodeID:
        //   node->nodeID = nd.nodeID;  // if you added 'nodeID' to Node
        nodePtrMap[nd.nodeID] = node;
    }

    // Fill each node’s keys, link children
    for (auto &pair : diskMap) {
        const auto &nd = pair.second;
        Node* node = nodePtrMap[nd.nodeID];

        if (nd.isLeaf) {
            // Leaf node
            LeafNode* ln = static_cast<LeafNode*>(node);

            // Insert each key with a dummy "value" = same as key
            for (KeyType k : nd.keys) {
                ln->createAndInsertRecord(k, k);
            }
            // Link next leaf if nextLeafID != -1
            if (nd.nextLeafID >= 0) {
                LeafNode* nxt = static_cast<LeafNode*>(nodePtrMap[nd.nextLeafID]);
                ln->setNext(nxt);
            }

        } else {
            // Internal node
            InternalNode* inNode = static_cast<InternalNode*>(node);

            // childIDs[0] is the left child
            if (!nd.childIDs.empty()) {
                int leftID = nd.childIDs[0];
                if (leftID >= 0) {
                    Node* leftChild = nodePtrMap[leftID];
                    leftChild->setParent(inNode);

                    // ***** CRUCIAL STEP: set the leftChild pointer *****
                    inNode->fLeftChild = leftChild; 
                    // (If fLeftChild is private, add a setter method or make it accessible.)
                }

                // For each key in nd.keys, the child is at childIDs[i+1]
                for (int i = 0; i < nd.size; i++) {
                    KeyType k = nd.keys[i];
                    int childID = nd.childIDs[i + 1];
                    Node* childPtr = (childID >= 0) ? nodePtrMap[childID] : nullptr;
                    if (childPtr) {
                        childPtr->setParent(inNode);
                    }
                    // Insert into fMappings: (key, childPtr)
                    inNode->fMappings.push_back(std::make_pair(k, childPtr));
                }
            }
        }
    }

    // The root is the node with ID == rootID
    fRoot = nodePtrMap[rootID];
    // Optionally: fRoot->setParent(nullptr);  // if you want the root to have no parent

    std::cout << "B+ Tree loaded from " << filename 
              << " with rootID=" << rootID << "\n";
}


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

    Record *record = leafNode->lookup(aKey);
    if (record) {
        // aKey already exists
        record->setValue(aValue);
        return;
    }

    int newSize = leafNode->createAndInsertRecord(aKey, aValue);

    if (newSize > leafNode->maxSize()) {
        LeafNode *newLeaf = split(leafNode);
        if (!newLeaf) {
            return;
        }

        std::cout << "New leaf created: " << newLeaf << std::endl;

        newLeaf->setNext(leafNode->next());
        leafNode->setNext(newLeaf);

        KeyType newKey = newLeaf->firstKey();
        std::cout << "New key for parent insertion: " << newKey << std::endl;

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
    while (!node->isLeaf()) {
        auto internalNode = static_cast<InternalNode *>(node);
        std::cout << "Current internal node: " << internalNode->firstKey() << std::endl;

        Node *nextNode = internalNode->lookup(aKey);
        std::cout << "Next node: " << nextNode << std::endl;

        if (nextNode == nullptr) {  // Check if lookup() returns nullptr
            std::cerr << "ERROR: lookup() returned nullptr for key " << aKey << std::endl;
            return nullptr;
        }

        node = nextNode;
    }
    return static_cast<LeafNode *>(node);
}

void BPlusTree::readInputFromFile(std::string aFileName) {
    int key;
    std::ifstream input(aFileName);
    while (input) {
        input >> key;
        insert(key, key);
    }
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
    if (!fRoot) {
        return;
    }

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
    Record *record = leaf->lookup(aKey);
    if (!record) {
        std::cout << "Record not found with key " << aKey << "." << std::endl;
        return;
    }
    if (aPrintPath) {
        std::cout << "\t";
    }
    std::cout << "Record found at location " << std::hex << record << std::dec << ":" << std::endl;
    std::cout << "\tKey: " << aKey << "   Value: " << record->value() << std::endl;
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
        startLeaf->copyRange(entries);
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

#include "CSV.h"
#include <algorithm>

void BPlusTree::bulkLoadFromCSV(const std::string& filename, int columnID, int columnIndex, int numberOfCharsToIndex) {
    CSVDatabase database;

    if (!readCSV(filename.c_str(), database, columnID, columnIndex, numberOfCharsToIndex)) {
        std::cerr << "Error: Could not read the CSV file." << std::endl;
        return;
    }

    std::vector<std::pair<KeyType, ValueType>> data;

    for (const auto& row : database) {
        if (row.size() < 2) {
            std::cerr << "Invalid row: expecting at least two columns (key, value)." << std::endl;
            continue;
        }

        try {
            KeyType key = std::stoi(row[columnID]);
            ValueType value = std::stoi(row[columnIndex]);
            data.push_back({key, value});
        } catch (const std::exception& e) {
            std::cerr << "Error parsing row: " << e.what() << std::endl;
        }
    }

    std::sort(data.begin(), data.end(), [](const std::pair<KeyType, ValueType>& a, const std::pair<KeyType, ValueType>& b) {
        return a.first < b.first;
    });

    for (const auto& entry : data) {
        insert(entry.first, entry.second);
    }
}
