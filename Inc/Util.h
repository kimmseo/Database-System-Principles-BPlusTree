#ifndef UTIL_H
#define UTIL_H

#include "CSV.h"

#include <string>  // Include this to use std::string
#include <iostream>
#include <sstream>
#include <algorithm>
#include <array>
#include <iterator>
#include "Definitions.h"

#include <BPlusTree.h>
#include <vector>

class Util {
  public:
    static int parseDate(const std::string& date);
    static void unparseDate(int packedDate, int& day, int& month, int& year);
};

#endif