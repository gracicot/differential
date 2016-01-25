cmake_minimum_required(VERSION 2.6)
project(differential)

add_definitions("-std=c++14")

add_executable(differential differential.cpp)

install(TARGETS differential RUNTIME DESTINATION bin)
