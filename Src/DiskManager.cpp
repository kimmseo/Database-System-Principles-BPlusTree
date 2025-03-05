#include "DiskManager.h"
#include <cstring> // for memset

DiskManager::DiskManager(const std::string &filename) : nextBlockID(0) {
    // open or create
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if(!file.is_open()) {
        // create new file
        file.clear();
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
}

bool DiskManager::readBlock(int blockID, NodeBlock &outBlock) {
    file.seekg(blockID * BLOCK_SIZE, std::ios::beg);
    if(!file.good()) return false;

    file.read(reinterpret_cast<char*>(&outBlock), sizeof(NodeBlock));
    return file.good();
}

bool DiskManager::writeBlock(int blockID, const NodeBlock &inBlock) {
    file.seekp(blockID * BLOCK_SIZE, std::ios::beg);
    if(!file.good()) return false;

    file.write(reinterpret_cast<const char*>(&inBlock), sizeof(NodeBlock));
    file.flush();
    return file.good();
}

int DiskManager::allocateBlockID() {
    // naive approach: just return nextBlockID++
    // or you can track free blocks, etc.
    return nextBlockID++;
}
