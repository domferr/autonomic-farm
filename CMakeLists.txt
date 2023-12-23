cmake_minimum_required(VERSION 3.27)
project(autonomicfarm)

set(CMAKE_CXX_STANDARD 20)

include_directories("${PROJECT_SOURCE_DIR}/include")
#file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/farm/src/*.cpp")

set(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/exe)

add_executable(autonomicfarm "${PROJECT_SOURCE_DIR}/main.cpp"
        include/ProgramArgs.hpp)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    add_subdirectory(tests)
endif()
