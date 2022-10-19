cmake_minimum_required(VERSION 3.20)
project(cgltf LANGUAGES C VERSION 1.12)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/cgltf/cgltf_write.h
    include/cgltf/cgltf.h
)

set(SRC
    src/cgltf.c
)

add_library(cgltf STATIC ${SRC} ${PUBLIC_HDRS})
target_include_directories(cgltf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET cgltf PROPERTY FOLDER "thrid_party")
