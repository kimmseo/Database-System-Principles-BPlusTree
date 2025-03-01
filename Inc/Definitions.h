//
// Created by Minseo on 2/7/2025.
//

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <string>

const int DEFAULT_ORDER{20};

// Minimum order is necessarily 3
// Maximum may be adjusted
const int MIN_ORDER{DEFAULT_ORDER - 1};
const int MAX_ORDER{20};

// Size of the buffer used to get the arguments (1 or 2)
const int BUFFER_SIZE{256};

inline float safeStof(const std::string& str) {
    try {
        if (str.empty() || str == "NULL" || str == "N/A")
            return 0.0f;  // Handle missing/invalid values
        return std::stof(str);
    } catch (const std::exception& e) {
        std::cerr << "Error converting to float: \"" << str << "\" (" << e.what() << ")\n";
        return 0.0f;  // Return default value if conversion fails
    }
}

inline int safeStoi(const std::string& str) {
    try {
        // Empty strings are invalid, return 0 as default
        if (str.empty()) return 0;
        return std::stoi(str);
    } catch (const std::exception& e) {
        std::cerr << "Error converting to int: " << str << " (" << e.what() << ")\n";
        return 0;
    }
}

struct gameRecord {
    std::string GAME_DATE_EST;
    unsigned int TEAM_ID_home;
    unsigned short PTS_home;
    float FG_PCT_home;
    float FT_PCT_home;
    float FG3_PCT_home;
    unsigned short AST_home;
    unsigned short REB_home;
    bool HOME_TEAM_WINS;

    gameRecord()
        : GAME_DATE_EST(""),
          TEAM_ID_home(0),
          PTS_home(0),
          FG_PCT_home(0.0f),
          FT_PCT_home(0.0f),
          FG3_PCT_home(0.0f),
          AST_home(0),
          REB_home(0),
          HOME_TEAM_WINS(false) {}

    gameRecord(std::string date, std::string team_id_home, std::string points_home,
               std::string final_goal_percent, std::string free_throw_percent,
               std::string three_point_percent, std::string assists, std::string rebounds,
               std::string home_team_wins)
        : GAME_DATE_EST(std::move(date)),
          TEAM_ID_home(safeStoi(team_id_home)),
          PTS_home(static_cast<unsigned short>(safeStoi(points_home))),
          FG_PCT_home(safeStof(final_goal_percent)),
          FT_PCT_home(safeStof(free_throw_percent)),
          FG3_PCT_home(safeStof(three_point_percent)),
          AST_home(static_cast<unsigned short>(safeStoi(assists))),
          REB_home(static_cast<unsigned short>(safeStoi(rebounds))),
          HOME_TEAM_WINS(safeStoi(home_team_wins) != 0) {}  // Convert int to bool
};

using KeyType = float;
using ValueType = gameRecord;

inline std::ostream& operator<<(std::ostream& os, const gameRecord& record) {
    os << "Game Date: " << record.GAME_DATE_EST << ", Team ID: " << record.TEAM_ID_home
       << ", PTS: " << record.PTS_home << ", FG%: " << record.FG_PCT_home
       << ", FT%: " << record.FT_PCT_home << ", FG3%: " << record.FG3_PCT_home
       << ", AST: " << record.AST_home << ", REB: " << record.REB_home
       << ", Home Win: " << (record.HOME_TEAM_WINS ? "Yes" : "No");
    return os;
}

inline bool operator<(const gameRecord& lhs, const gameRecord& rhs) {
    return lhs.FG_PCT_home < rhs.FG_PCT_home;  // Use FG_PCT_home as the sorting key
}

#endif  // DEFINITIONS_H
