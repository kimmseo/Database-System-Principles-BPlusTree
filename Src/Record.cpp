//
// Created by Minseo on 2/7/2025.
//

#include <string>
#include <sstream>
#include "Definitions.h"
#include "Record.h"

Record::Record(ValueType aValue) : fValue(aValue) {}

ValueType Record::value() const {
    return fValue;
}

void Record::setValue(ValueType aValue) {
    fValue = aValue;
}

std::string Record::toString() const {
    std::ostringstream oss;
    oss << fValue;
    return oss.str();
}