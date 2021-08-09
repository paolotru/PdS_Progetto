cmake_minimum_required(VERSION 3.19)
project(our)

set(CMAKE_CXX_STANDARD 20)

add_executable(our main.cpp graph/GraphElements.cpp graph/GraphElements.h)