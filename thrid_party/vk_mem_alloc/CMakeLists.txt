cmake_minimum_required(VERSION 3.20)
project(vk_mem_alloc LANGUAGES CXX VERSION 3.0.1)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/vk_mem_alloc/vk_mem_alloc.h
)

set(SRC
    src/vk_mem_alloc.cpp
    vk_mem_alloc.natvis
)

tndr_set_catalog_tree("${PUBLIC_HDRS}")
tndr_set_catalog_tree("${SRC}")

add_library(vk_mem_alloc STATIC ${SRC} ${PUBLIC_HDRS})
target_link_libraries(vk_mem_alloc PRIVATE vulkan_headers)
target_include_directories(vk_mem_alloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT LUMINOUS_MONOLITHIC_BUILD)
    if(UNIX)
        target_compile_options(vk_mem_alloc PRIVATE -fPIC)
    endif()
endif()

set_property(TARGET vk_mem_alloc PROPERTY FOLDER "thrid_party")
