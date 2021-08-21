cmake_minimum_required(VERSION 3.10)
project(sink_test_actor)
 
add_definitions(-std=c++17)
add_definitions(-O3)
add_definitions(-g)


add_executable(dump dump.cpp)
target_link_libraries(dump stm32_source_actor dump_sink_actor)

add_executable(creat_db creat_db.cpp)
target_link_libraries(creat_db dump_sink_actor)

add_executable(db_insert db_insert.cpp)
target_link_libraries(db_insert dump_sink_actor)
