cmake_minimum_required(VERSION 3.10)
project(websocket_sink_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)

find_package(websocketpp REQUIRED)
include_directories(${WEBSOCKETPP_DIR})

add_library(${PROJECT_NAME} views.cpp)
target_link_libraries(${PROJECT_NAME} pthread stm32_source_actor pb_sink_actor)

add_executable(subprotocol_server subprotocol_server.cpp)
target_link_libraries(subprotocol_server pthread )