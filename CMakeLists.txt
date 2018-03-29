cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 11)

project(fab)

include_directories(include)
include_directories(dependencies/json/single_include)
set(SOURCES
src/main.cpp
src/fab/Compiler.cpp
src/fab/GccCompiler.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} stdc++fs)