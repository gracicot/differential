cmake_minimum_required(VERSION 2.6)
project(diffential)

add_definitions("-std=c++14")

add_executable(diffential differential.cpp)

install(TARGETS diffential RUNTIME DESTINATION bin)
