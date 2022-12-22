cmake_minimum_required(VERSION 3.18)
project(SemestralnaPracaPOS C)

set(CMAKE_C_STANDARD 99)

add_executable(SemestralnaPracaPOS Client.c Client.h Server.c Server.h)

target_link_libraries(SemestralnaPracaPOS pthread m)
