//
// Created by Minseo on 3/1/2025.
//

#ifndef CSV_H
#define CSV_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

// Defines
typedef std::vector<std::string> CSVRow;
typedef CSVRow::const_iterator CSVRowIterator;
typedef std::vector<CSVRow> CSVDatabase;
typedef CSVDatabase::const_iterator CSVDatabaseIterator;

bool readCSV(const char* filePath, CSVDatabase& database, int columnID, int columnIndex,
             int numberOfCharsToIndex);
void readCSV(std::istream& input, CSVDatabase& database, int columnID, int columnIndex,
             int numberOfCharsToIndex);

inline bool readCSV(const char* filePath, CSVDatabase& database, int columnID, int columnIndex,
                    int numberOfCharsToIndex) {
    // Read the file path in filePath
    std::fstream file(filePath, std::ios::in);
    // Verify if valid file path
    if (!file.is_open()) {
        std::cout << "Function 'readCSV': " << filePath << " not found\n!";
        return false;
    }
    // Read file data
    readCSV(file, database, columnID, columnIndex, numberOfCharsToIndex);
    if (database.size() > 0) {
        return true;
    }
    std::cout << "Function 'readCSV': the table has no attributes.\n";
    return false;
}

inline void readCSV(std::istream& input, CSVDatabase& database, int columnID, int columnIndex,
                    int numberOfCharsToIndex) {
    std::string line;
    database.clear();
    int counter = 0;

    try {
        // Read every line from file
        while (std::getline(input, line)) {
            // Read current line from file
            // string -> stringstream, required for getline
            std::stringstream csvStream(line);
            CSVRow row;
            std::string column;
            // Read each column in getline
            while (getline(csvStream, column, ',')) {
                column = column.substr(0, numberOfCharsToIndex);
                row.push_back(column);
                counter++;
            }
            counter = 0;
            // Insert current CSVRow in database
            if (row.size() > 0) {
                database.push_back(row);
            }
        }
    } catch (std::exception& e) {
        std::cout << "Function 'readCSV': error reading this column in input\n";
    }
}

inline void display(const CSVDatabase& database) {
    // Verify if file contains data
    if (!database.size()) {
        return;
    }
    // Get the first row
    CSVDatabaseIterator row = database.begin();
    // Get each row
    int i = 0;
    for (; row != database.end(); ++row) {
        // Verify if row contains data
        if (!row->size()) {
            continue;
        }
        // Get the first column
        CSVRowIterator column = row->begin();
        // Print first column, drop ','
        std::cout << i++ << "->" << *(column++);
        // Print rest of the columns
        for (; column != row->end(); ++column) {
            std::cout << ", " << *column;
        }
        std::cout << "\n";
    }
}

#endif  // CSV_H
