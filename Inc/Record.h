//
// Created by Minseo on 2/7/2025.
//

#ifndef RECORD_H
#define RECORD_H

#include "Definitions.h"

class Record {
  public:
    explicit Record(ValueType aValue);
    ValueType value() const;
    void setValue(ValueType aValue);
    std::string toString() const;

  private:
    Record() : fValue() {}
    ValueType fValue;
};

#endif  // RECORD_H
