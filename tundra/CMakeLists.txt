cmake_minimum_required(VERSION 3.20)
project(tundra VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
)

set(PRIVATE_HDRS
    src/renderer/renderer.h

    src/app.h
    src/meshlet_mesh.h
)

set(SRC
    src/renderer/renderer.cpp

    src/app.cpp
    src/main.cpp
    src/meshlet_mesh.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
)

set(PRIVATE_DEPENDENCIES
    core
    math
    platform
    renderer
    rhi

    cxxopts
    glfw
    cgltf
    meshoptimizer
)

tndr_add_executable(tundra
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
