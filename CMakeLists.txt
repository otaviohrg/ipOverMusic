cmake_minimum_required(VERSION 2.8)

project(ipOverMusic)

FIND_PACKAGE (Threads REQUIRED)
set(CMAKE_CXX_FLAGS "-lpulse")

include_directories("${PROJECT_SOURCE_DIR}/lib/")
file(GLOB SOURCES "src/*.c")
add_library(ipOverMusic SHARED ${SOURCES})