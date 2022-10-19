cmake_minimum_required(VERSION 3.20)
project(robin_hood_hashing VERSION 3.11.5 LANGUAGES CXX)

add_library(robin_hood_hashing INTERFACE)
target_include_directories(robin_hood_hashing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
