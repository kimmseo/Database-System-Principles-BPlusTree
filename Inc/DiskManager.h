#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <fstream>
#include <string>
#include <cstdint>

static const int BLOCK_SIZE = 4096;  // or system’s page size

struct NodeBlock {
    int nodeID;            // unique ID assigned during BFS
    bool isLeaf;           // 1 if leaf, 0 if internal
    int size;              // # of keys
    int parentID;          // ID of parent node, -1 if none
    int nextLeafID;        // if leaf, store fNext’s ID or -1
    int leftChildID;       // if internal, store fLeftChild’s ID or -1

    // For internal node:
    float keys[50];        
    int childIDs[50];      

    // For leaf node:
    float leafKeys[50];   


    // constructor
    NodeBlock() {
        nodeID = -1;
        isLeaf = false;
        size = 0;
        parentID = -1;
        nextLeafID = -1;
        leftChildID = -1;
        // zero out arrays if you want
    }
};

class DiskManager {
public:
    DiskManager(const std::string &filename);

    // read a NodeBlock from disk
    bool readBlock(int blockID, NodeBlock &outBlock);

    // write a NodeBlock to disk
    bool writeBlock(int blockID, const NodeBlock &inBlock);

    // get a new block ID
    int allocateBlockID();

private:
    std::fstream file;
    int nextBlockID;
};

#endif
