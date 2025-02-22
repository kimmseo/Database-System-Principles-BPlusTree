#ifndef UTIL_H
#define UTIL_H

#include <string>  // Include this to use std::string
#include <iostream>
#include <sstream>

class Util {
  public:
    static uint32_t parseDate(const std::string& date);
    static void unparseDate(uint32_t packedDate, int& day, int& month, int& year);
};

#endif