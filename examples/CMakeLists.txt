cmake_minimum_required(VERSION 3.5)

project(Hot_Demo)

add_executable(demo demo.cpp)

add_library(shared SHARED shared.cpp)