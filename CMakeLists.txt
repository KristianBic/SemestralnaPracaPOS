cmake_minimum_required(VERSION 3.18)
project(SemestralnaPracaPOS C)

set(CMAKE_C_STANDARD 99)

add_executable(SemestralnaPracaPOS server.c)

target_link_libraries(SemestralnaPracaPOS pthread m)
