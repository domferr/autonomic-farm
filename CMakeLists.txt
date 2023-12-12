cmake_minimum_required(VERSION 3.27)
project(autonomic-farm)

set(CMAKE_CXX_STANDARD 20)

include_directories("${PROJECT_SOURCE_DIR}/include")
#file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/farm/src/*.cpp")

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/exe)

add_executable(farm "${PROJECT_SOURCE_DIR}/main.cpp" ${SOURCES})