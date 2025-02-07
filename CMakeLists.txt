cmake_minimum_required(VERSION 3.30)
project(Database_System_Principles_Project_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(Database_System_Principles_Project_1
        Definitions.h
        Node.h
        Node.cpp
        Exceptions.h
        InternalNode.cpp
        InternalNode.h
        LeafNode.cpp
        LeafNode.h
        Record.cpp
        Record.h
        Exceptions.cpp
        BPlusTree.cpp
        BPlusTree.h
        Printer.cpp
        Printer.h
        main.cpp
)
