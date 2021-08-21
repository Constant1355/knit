cmake_minimum_required(VERSION 3.10)
project(test_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

add_subdirectory(source)
add_subdirectory(sensors)
add_subdirectory(sink)

add_executable(tube tube.cpp)
target_link_libraries(tube pthread)