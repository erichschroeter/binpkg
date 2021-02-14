cmake_minimum_required(VERSION 3.12)
project(binpkg LANGUAGES CXX VERSION 0.0.1)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

add_subdirectory(src)
add_subdirectory(test)

include(CTest)