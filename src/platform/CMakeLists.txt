cmake_minimum_required(VERSION 3.20)
project(platform VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/platform/window_native_handle.h
)

set(PRIVATE_HDRS
)

set(SRC
    src/platform_module.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
    core
)

set(PRIVATE_DEPENDENCIES
)

tndr_add_module(platform
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
