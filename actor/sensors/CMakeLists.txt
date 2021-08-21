cmake_minimum_required(VERSION 3.10)
project(sensors_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

add_subdirectory(clock)
add_subdirectory(ld06)
add_subdirectory(coordinate)

