cmake_minimum_required(VERSION 3.31)
project(OSGroup)

set(CMAKE_CXX_STANDARD 20)

add_executable(OSGroup
        main.cpp
        header/BucketCell.h
        header/Index_table.h
        header/MurmurHash3.h
        header/OSFilter.h
        BucketCell.cpp
        Index_table.cpp
        MurmurHash3.cpp
        OSFilter.cpp
)
