[Google Docs Project Report](https://docs.google.com/document/d/1WtS6wBsTzRH7-qIsAmv6fLDHr4i85h7oSXdD7ya9iFw/edit?usp=sharing)


# Build Instructions

## **1️⃣ Prerequisites**
Ensure you have the following installed:
- **C++20 Compiler** (GCC, Clang, or MSVC)
- **CMake (v3.30 or later)**

```sh
cmake -S . -B build   # Generate build files in "build" directory
cmake --build build   # Compile the project

./build/Database_System_Principles_Project_1 # run the executable

# This set of instructions was built and compiled using Visual Studios.
# Run this if above doesn't work
cmake ..
cmake --build build
cd build
Debug\Database_System_Principles_Project_1.exe 

# For clion app, just click run on main.exe to get it to work.

# Task 1
Each record was stored as such.
    std::string GAME_DATE_EST;  // Date the game was held
    unsigned int TEAM_ID_home;  // Team ID of hometeam
    unsigned short PTS_home;    // Points scored by hometeam
    float FG_PCT_home;          // Final goal percentage
    float FT_PCT_home;          // Free throw percentage
    float FG3_PCT_home;         // 3-point final goal percentage
    unsigned short AST_home;    // Assists scored
    unsigned short REB_home;    // Rebounds
    bool HOME_TEAM_WINS;        // Boolean for win(1) or loss(0)


# Task 2
Run the code to start sorting then bulk and normal loading.
It will show the time taken to upload 
input 'm' into terminal to check the parameter n of B+ tree, number of nodes of B+ tree, number of levels of B+ tree and content of root node.

# Task 3
To get details for task 3, input 'r 0.6 0.9' into the terminal.
