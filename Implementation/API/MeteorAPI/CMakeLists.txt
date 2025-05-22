cmake_minimum_required(VERSION 3.10)
project(MeteorAPI)
add_library(MeteorAPI INTERFACE)
target_include_directories(MeteorAPI
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
