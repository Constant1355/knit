cmake_minimum_required(VERSION 3.10)
project(coordinate_sensors_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})

add_library(${PROJECT_NAME} mpu9250.cpp yesense.cpp rm3100.cpp mpu9250_mag.cpp)
target_link_libraries(${PROJECT_NAME} pthread)