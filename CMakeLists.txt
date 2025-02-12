cmake_minimum_required(VERSION 3.30)
project(Database_System_Principles_Project_1)

#C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define source and include directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/Src")
set(INC_DIR "${CMAKE_SOURCE_DIR}/Inc")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")

include_directories(${INC_DIR})

add_executable(Database_System_Principles_Project_1 ${SRC_FILES})
