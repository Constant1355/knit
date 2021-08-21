cmake_minimum_required(VERSION 3.10)
project(sensors_test_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})

add_executable(ld06_read ld06_read.cpp)
target_link_libraries(ld06_read stm32_source_actor ld06_sensors_actor)

add_executable(imu_read imu_read.cpp)
target_link_libraries(imu_read stm32_source_actor coordinate_sensors_actor)

add_executable(mag_read mag_read.cpp)
target_link_libraries(mag_read stm32_source_actor coordinate_sensors_actor)

add_executable(ds3231_read ds3231_read.cpp)
target_link_libraries(ds3231_read stm32_source_actor clock_sensors_actor)

add_executable(all_read all_read.cpp)
target_link_libraries(all_read stm32_source_actor clock_sensors_actor coordinate_sensors_actor ld06_sensors_actor)
