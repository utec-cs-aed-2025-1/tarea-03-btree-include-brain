cmake_minimum_required(VERSION 3.29)
project(tarea_03_btree_include_brain)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(tarea_03_btree_include_brain
    main.cpp)

add_executable(p debug_build.cpp)
