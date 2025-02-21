#ifndef UTIL_H
#define UTIL_H

#include <string>  // Include this to use std::string
#include <iostream>
#include <sstream>

class Util {
  public:
    static void parseDate(const std::string& date, int& day, int& month, int& year);
};

#endif