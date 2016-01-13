cmake_minimum_required(VERSION 2.6)
project(diffential)

add_executable(diffential main.cpp)

install(TARGETS diffential RUNTIME DESTINATION bin)
