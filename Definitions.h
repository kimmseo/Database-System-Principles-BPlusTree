//
// Created by Minseo on 2/7/2025.
//

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cstdint>
#include <cstdlib>

const int DEFAULT_ORDER{4};

// Minimum order is necessarily 3
// Maximum may be adjusted
const int MIN_ORDER{DEFAULT_ORDER - 1};
const int MAX_ORDER{20};

using KeyType = int64_t;
using ValueType = int64_t;

// Size of the buffer used to get the arguments (1 or 2)
const int BUFFER_SIZE{256};

#endif //DEFINITIONS_H
