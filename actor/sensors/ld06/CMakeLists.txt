cmake_minimum_required(VERSION 3.10)
project(ld06_sensors_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

add_library(${PROJECT_NAME} cloud.cpp pkt.cpp)
target_link_libraries(${PROJECT_NAME} pthread)