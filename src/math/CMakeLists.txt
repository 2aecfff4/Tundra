cmake_minimum_required(VERSION 3.20)
project(math VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/math/constants.h
    include/math/log2_indices.h
    include/math/math_hash.h
    include/math/math_utils.h
    include/math/matrix4.h
    include/math/quat.h
    include/math/range.h
    include/math/transform.h
    include/math/vector2.h
    include/math/vector3.h
    include/math/vector4.h
)

set(PRIVATE_HDRS
)

set(SRC
    src/math_module.cpp
    src/transform.cpp
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
    core
)

set(PRIVATE_DEPENDENCIES
)

tndr_add_module(math
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
