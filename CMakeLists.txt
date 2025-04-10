# Copyright © 2024 Zen Shawn. All rights reserved.
# Created at 11:39:22, December 25, 2024

cmake_minimum_required(VERSION 3.21)

option(BUILD_SHARED_LIBS "build shared libs" ON)

set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "build type, Release/Debug/MinSizeRel/RelWithDebInfo")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(Coin3DUtils VERSION 0.1.0 DESCRIPTION "Introduction about coin-examples")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# find_package(SoQt CONFIG REQUIRED)
# find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Widgets OpenGL)
find_package(Coin CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(spdlog CONFIG REQUIRED)

if(MSVC)
  add_compile_options(/utf-8 /W2)
endif()

add_subdirectory(srcs)