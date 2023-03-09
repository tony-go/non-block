cmake_minimum_required(VERSION 3.16)
project(example)

set(CMAKE_CXX_STANDARD 20)

add_executable(example main.cpp)

target_compile_features(example PRIVATE cxx_std_20)

