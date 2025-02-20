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

# Clang-format custom target
find_program(CLANG_FORMAT NAMES clang-format)

if (CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -style=file -i ${SRC_DIR}/*.cpp ${INC_DIR}/*.h
        COMMENT "Formatting source code with clang-format"
    )
    add_dependencies(Database_System_Principles_Project_1 format)
else()
    message(WARNING "clang-format not found! Skipping code formatting.")
endif()