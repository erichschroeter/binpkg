cmake_minimum_required(VERSION 3.12)
# Fix Catch macro warnings after upgrading to CMake 3.20+
cmake_policy(SET CMP0110 OLD)
project(binpkg LANGUAGES CXX VERSION 0.0.1)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

add_subdirectory(src)
add_subdirectory(test)

include(CTest)
include(CPack)
