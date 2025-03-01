//
// Created by Minseo on 2/7/2025.
//

#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <tuple>
#include <vector>
#include "Definitions.h"
#include "Printer.h"

class InternalNode;
class LeafNode;
class Node;

/// Main class providing the API for the B+ Tree
class BPlusTree {
  public:
    /// Sole constructor.  Accepts an optional order for the B+ Tree.
    /// The default order will provide a reasonable demonstration of the
    /// data structure and its operations.
    explicit BPlusTree(int aOrder = DEFAULT_ORDER);

    /// The type used in the API for inserting a new key-value pair
    /// into the tree.  The third item is the type of the Node into
    /// which the key will be inserted.
    using EntryType = std::tuple<KeyType, ValueType, LeafNode*>;

    /// Returns true if this B+ tree has no keys or values.
    bool isEmpty() const;

    /// Insert a key-value pair into this B+ tree.
    void insert(KeyType aKey, ValueType aValue);

    /// Remove a key and its value from this B+ tree.
    void remove(KeyType aKey);

    /// Print this B+ tree to stdout using a simple command-line
    /// ASCII graphic scheme.
    /// @param[in] aVerbose Determins whether printing should include addresses.
    void print(bool aVerbose = false);

    /// Print the bottom rank of this B+ tree, consisting of its leaves.
    /// This shows all the keys in the B+ tree in sorted order.
    /// @param[in] aVerbose Determins whether printing should include addresses.
    void printLeaves(bool aVerbose = false);

    /// Print the value associated with a given key, along with the address
    /// at which the tree stores that value.
    /// @param[in] aVerbose Determines whether printing should include addresses.
    void printValue(KeyType aKey, bool aVerbose = false);

    /// Print the path from the root to the leaf bearing key aKey.
    /// @param[in] aVerbose Determines whether printing should include addresses.
    void printPathTo(KeyType aKey, bool aVerbose = false);

    /// Print key, value, and address for each item in the range
    /// from aStart to aEnd, including both.
    void printRange(KeyType aStart, KeyType aEnd);

    /// Remove all elements from the B+ tree. You can then build
    /// it up again by inserting new elements into it.
    void destroyTree();

    /// Read elements to be inserted into the B+ tree from a text file.
    /// Each new element should consist of a single integer on a line by itself.
    /// This B+ tree treats each such input as both a new value and the key
    /// under which to store it.
    void readInputFromFile(std::string aFileName);

    /// Print the levels and number of nodes of the tree.
    /// Print the content of the root node
    /// from aStart to aEnd, including both.
    void printTreeInfo();

    void saveToDisk(const std::string& filename);
    void loadFromDisk(const std::string& filename);
    // Bulk load data from a CSV file into the B+ tree.
    // The columnID is the column number to use as the key.

    double bulkLoadFromCSV(const std::string& filename, int keyColumn);
    double normalInsertFromCSV(const std::string& filename, int keyColumn);
    double normalInsert(const std::vector<std::pair<KeyType, ValueType>>& data);

  private:
    void startNewTree(KeyType aKey, ValueType aValue);
    void insertIntoLeaf(KeyType aKey, ValueType aValue);
    void insertIntoParent(Node* aOldNode, KeyType aKey, Node* aNewNode);
    template <typename T>
    T* split(T* aNode);
    void removeFromLeaf(KeyType aKey);
    template <typename N>
    void coalesceOrRedistribute(N* aNode);
    template <typename N>
    void coalesce(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex);
    template <typename N>
    void redistribute(N* aNeighborNode, N* aNode, InternalNode* aParent, int aIndex);
    void adjustRoot();
    LeafNode* findLeafNode(KeyType aKey, bool aPrinting = false, bool aVerbose = false);
    void printValue(KeyType aKey, bool aPrintPath, bool aVerbose);
    std::vector<EntryType> range(KeyType aStart, KeyType aEnd);
    const int fOrder;
    Node* fRoot;
    Printer fPrinter;
    std::vector<std::pair<KeyType, ValueType>> loadDataForNormalInsert(const std::string& filename,
                                                                       int keyColumn);
};

#endif  // BPLUSTREE_H
