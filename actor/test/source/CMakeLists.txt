cmake_minimum_required(VERSION 3.10)
project(source_test_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)


add_executable(spi_read spi_read.cpp)
target_link_libraries(spi_read stm32_source_actor)

add_executable(spi_single_read spi_single_read.cpp)
target_link_libraries(spi_single_read stm32_source_actor)
