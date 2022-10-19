cmake_minimum_required(VERSION 3.20)
project(Tundra VERSION 1.0.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-source built.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(utils)

include(GenerateExportHeader)
set(CMAKE_INSTALL_UCRT_LIBRARIES ON)
include(InstallRequiredSystemLibraries)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TUNDRA_MONOLITHIC_BUILD "Set to true if you want to build using static libraries." OFF)
option(TUNDRA_ENABLE_VULKAN_SUPPORT "Set to true if you want to enable Vulkan support." ON)
option(TUNDRA_ENABLE_TESTS "Set to true if you want to enable tests." ON)
option(TUNDRA_ENABLE_PROFILER "Set to true if you want to enable profiler." ON)

option(TUNDRA_ENABLE_ADDRESS_SANITIZER "Enable asan. This adds -fsanitize=address flag. This is incompatible with thread sanitizer" OFF)
option(TUNDRA_ENABLE_THREAD_SANITIZER "Enable thread sanitizer. This adds -fsanitize=thread flag. This is incompatible with address sanitizer" OFF)

message(STATUS "TUNDRA: Detected C compiler: ${CMAKE_C_COMPILER_ID} version: ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "TUNDRA: Detected C++ compiler: ${CMAKE_CXX_COMPILER_ID} version: ${CMAKE_CXX_COMPILER_VERSION}")

if(TUNDRA_ENABLE_LTO)
    include(CheckIPOSupported)

    check_ipo_supported(RESULT supported)

    if(supported)
        message(STATUS "LUMINOUS: LTO is enabled!")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if(TUNDRA_ENABLE_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=address")

    if(TUNDRA_ENABLE_THREAD_SANITIZER)
        message(FATAL_ERROR "ASan is incompatible with thread sanitizer!")
    endif(TUNDRA_ENABLE_THREAD_SANITIZER)
endif(TUNDRA_ENABLE_ADDRESS_SANITIZER)

if(TUNDRA_ENABLE_THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -fsanitize=thread")

    if(TUNDRA_ENABLE_ADDRESS_SANITIZER)
        message(FATAL_ERROR "Thread sanitizer is incompatible with ASan!")
    endif(TUNDRA_ENABLE_ADDRESS_SANITIZER)
endif(TUNDRA_ENABLE_THREAD_SANITIZER)

if(UNIX AND CMAKE_GENERATOR STREQUAL "Ninja")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color")
    endif()

    if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color")
    endif()
endif()

if(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    string(REGEX REPLACE "/Z[a-z]+" "/ZI" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REGEX REPLACE "/Z[a-z]+" "/ZI" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
    add_definitions("/Zc:__cplusplus")

    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
    # set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /ZI")
endif()

# add_subdirectory(shaders)
add_subdirectory(src)
add_subdirectory(thrid_party)
add_subdirectory(tundra)
