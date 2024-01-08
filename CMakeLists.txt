cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}, Platform: ${CMAKE_SYSTEM_NAME}")
include_directories(third_party/doctest utility)

FILE(GLOB_RECURSE SRC_LIST ${CMAKE_SOURCE_DIR}/test/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
