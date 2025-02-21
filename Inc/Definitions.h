//
// Created by Minseo on 2/7/2025.
//

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <cstdint>
#include <cstdlib>
#include <utility>

const int DEFAULT_ORDER{4};

// Minimum order is necessarily 3
// Maximum may be adjusted
const int MIN_ORDER{DEFAULT_ORDER - 1};
const int MAX_ORDER{20};

using KeyType = int64_t;
using ValueType = int64_t;

// Size of the buffer used to get the arguments (1 or 2)
const int BUFFER_SIZE{256};

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

    // Corrected Constructor
    gameRecord(std::string date, unsigned int team_id_home, unsigned short points_home,
               float final_goal_percent, float free_throw_percent, float three_point_percent,
               unsigned short assists, unsigned short rebounds, bool home_team_wins)
        : GAME_DATE_EST(std::move(date)),
          TEAM_ID_home(team_id_home),
          PTS_home(points_home),
          FG_PCT_home(final_goal_percent),
          FT_PCT_home(free_throw_percent),
          FG3_PCT_home(three_point_percent),
          AST_home(assists),
          REB_home(rebounds),
          HOME_TEAM_WINS(home_team_wins) {}
};

#endif  // DEFINITIONS_H
