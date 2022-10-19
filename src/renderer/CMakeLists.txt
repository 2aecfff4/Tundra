cmake_minimum_required(VERSION 3.20)
project(renderer VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/renderer/frame_graph/resources/barrier.h
    include/renderer/frame_graph/resources/base_resource.h
    include/renderer/frame_graph/resources/buffer.h
    include/renderer/frame_graph/resources/enums.h
    include/renderer/frame_graph/resources/handle.h
    include/renderer/frame_graph/resources/texture.h

    include/renderer/frame_graph/builder.h
    include/renderer/frame_graph/enums.h
    include/renderer/frame_graph/frame_graph.h
    include/renderer/frame_graph/present_pass.h
    include/renderer/frame_graph/registry.h
    include/renderer/frame_graph/render_pass.h
)

set(PRIVATE_HDRS
)

set(SRC
    src/frame_graph/resources/barrier.cpp
    src/frame_graph/resources/base_resource.cpp
    src/frame_graph/resources/buffer.cpp
    src/frame_graph/resources/handle.cpp
    src/frame_graph/resources/texture.cpp

    src/frame_graph/builder.cpp
    src/frame_graph/frame_graph.cpp
    src/frame_graph/present_pass.cpp
    src/frame_graph/registry.cpp
    src/frame_graph/render_pass.cpp

    src/renderer_module.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
    core
    globals
    rhi
)

set(PRIVATE_DEPENDENCIES
)

tndr_add_module(renderer
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
