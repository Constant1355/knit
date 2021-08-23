cmake_minimum_required(VERSION 3.10)
project(pb_sink_actor)
 
add_definitions(-std=c++11)
add_definitions(-O3)
add_definitions(-g)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS spi_message.proto)

add_library(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${PROJECT_NAME} ${Protobuf_LIBRARIES})
