cmake_minimum_required(VERSION 3.5.0)
project(VisionOnEmbedded)
set(PJT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(3RDPARTY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")

add_definitions(-DNOMINMAX)
set(CMAKE_CXX_STANDARD 17)

include(cmake/predefines.cmake)
add_subdirectory(3rdparty)
find_package(Eigen3 REQUIRED)
add_subdirectory(apps)
add_subdirectory(modules)
 