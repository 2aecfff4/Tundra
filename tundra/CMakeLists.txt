cmake_minimum_required(VERSION 3.20)
project(tundra VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
)

set(PRIVATE_HDRS
    src/renderer/common/culling/instance_culling_and_lod.h
    src/renderer/common/culling/meshlet_culling.h

    src/renderer/hardware/hardware_rasterizer.h

    src/renderer/software/passes/gpu_rasterize_debug_pass.h
    src/renderer/software/passes/gpu_rasterizer_init.h
    src/renderer/software/passes/gpu_rasterizer.h
    src/renderer/software/software_rasterizer.h

    src/renderer/config.h
    src/renderer/helpers.h
    src/renderer/render_input_output.h
    src/renderer/renderer.h
    src/renderer/ubo.h

    src/app.h
    src/meshlet_mesh.h
    src/pipelines.h
    src/shader.h
)

set(SRC
    src/renderer/common/culling/instance_culling_and_lod.cpp
    src/renderer/common/culling/meshlet_culling.cpp

    src/renderer/hardware/hardware_rasterizer.cpp

    src/renderer/software/passes/gpu_rasterize_debug_pass.cpp
    src/renderer/software/passes/gpu_rasterizer_init.cpp
    src/renderer/software/passes/gpu_rasterizer.cpp
    src/renderer/software/software_rasterizer.cpp

    src/renderer/renderer.cpp

    src/app.cpp
    src/main.cpp
    src/meshlet_mesh.cpp
    src/pipelines.cpp
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
