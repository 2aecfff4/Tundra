cmake_minimum_required(VERSION 3.20)
project(globals VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/globals/globals.h
)

set(PRIVATE_HDRS
)

set(SRC
    src/globals_module.cpp
    src/globals.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
)

set(PRIVATE_DEPENDENCIES
    core
)

tndr_add_module(globals
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
)
