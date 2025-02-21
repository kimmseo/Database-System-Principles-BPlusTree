#include "Util.h"

using namespace std;

void Util::parseDate(const std::string& date, int& day, int& month, int& year) {
    char delimiter;
    std::stringstream ss(date);

    if (!(ss >> day >> delimiter >> month >> delimiter >> year) || delimiter != '/') {
        cout << "incorrect date format" << endl;
    }
}