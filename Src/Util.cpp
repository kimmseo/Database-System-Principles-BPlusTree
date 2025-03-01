#include "Util.h"

int Util::parseDate(const std::string& date) {
    char delimiter;
    int day, month, year;
    std::stringstream ss(date);

    if (ss >> day >> delimiter >> month >> delimiter >> year && delimiter == '/') {
        if (day < 1 || day > 31 || month < 1 || month > 12 || year < 0 || year > 9999) {
            throw std::invalid_argument("Date values out of range!");
        }

        // Pack the date into a 24-bit:
        // Bits: [15 bits (year)] [ 4 bits (month)] [5 bits (days)]
        int packedDate = 0;
        packedDate |= (static_cast<int>(year) << 9);  // Shift year to the left by 9 bits
        packedDate |= (month << 5);                        // Shift month to the left by 5 bits
        packedDate |= day;                                 // Store day in the lowest 5 bits

        return packedDate;
    } else {
        throw std::invalid_argument("Invalid date format!");
    }
}

void Util::unparseDate(int packedDate, int& day, int& month, int& year) {
    day = packedDate & 0x1F;            // Extract 5 bits for day
    month = (packedDate >> 5) & 0xF;    // Extract 4 bits for month
    year = (packedDate >> 9) & 0x7FFF;  // Extract 15 bits for year
}
