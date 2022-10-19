cmake_minimum_required(VERSION 3.20)
project(vulkan_headers VERSION 1.3.218 LANGUAGES CXX)

add_library(vulkan_headers INTERFACE)
target_include_directories(vulkan_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
