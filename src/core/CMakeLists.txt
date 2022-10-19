cmake_minimum_required(VERSION 3.20)
project(core VERSION 1.0.0 LANGUAGES CXX)

# ######################################################
# Files
set(PUBLIC_HDRS
    include/core/memory/allocator.h
    include/core/memory/linear_allocator.h
    include/core/memory/pointer_math.h

    include/core/module/module.h
    include/core/module/module_manager.h

    include/core/std/containers/array.h
    include/core/std/containers/cstring.h
    include/core/std/containers/deque.h
    include/core/std/containers/hash_map.h
    include/core/std/containers/hash_set.h
    include/core/std/containers/string.h

    include/core/std/sync/lock.h
    include/core/std/sync/rw_lock.h

    include/core/std/traits/declval.h
    include/core/std/traits/is_callable.h

    include/core/std/assert.h
    include/core/std/defer.h
    include/core/std/expected.h
    include/core/std/function.h
    include/core/std/hash.h
    include/core/std/option.h
    include/core/std/panic.h
    include/core/std/reference_wrapper.h
    include/core/std/shared_ptr.h
    include/core/std/span.h
    include/core/std/timer.h
    include/core/std/tuple.h
    include/core/std/type_list.h
    include/core/std/unique_ptr.h
    include/core/std/utils.h
    include/core/std/variant.h

    include/core/utils/endianness.h
    include/core/utils/enum_flags.h
    include/core/utils/libloader.h

    include/core/build.h
    include/core/compiler_setup.h
    include/core/core.h
    include/core/logger.h
    include/core/macros.h
    include/core/profiler.h
    include/core/project_dirs.h
    include/core/typedefs.h
)

set(PRIVATE_HDRS
    src/project_dirs/windows.h
)

set(SRC
    src/memory/allocator.cpp
    src/memory/linear_allocator.cpp

    src/module/module.cpp
    src/module/module_manager.cpp

    src/project_dirs/project_dirs.cpp
    src/project_dirs/windows.cpp

    src/std/containers/cstring.cpp

    src/std/assert.cpp
    src/std/panic.cpp
    src/std/timer.cpp

    src/utils/libloader.cpp

    src/core_module.cpp
    src/logger.cpp
    src/profiler.cpp

    core.natvis
)

# ######################################################
# Dependencies
set(PUBLIC_DEPENDENCIES
    fmt
    robin_hood_hashing
)

set(PRIVATE_DEPENDENCIES
)

tndr_add_module(core
    SOURCES ${SRC} ${PUBLIC_HDRS} ${PRIVATE_HDRS}
    PRIVATE_DEPENDENCIES ${PRIVATE_DEPENDENCIES}
    PUBLIC_DEPENDENCIES ${PUBLIC_DEPENDENCIES}
)
