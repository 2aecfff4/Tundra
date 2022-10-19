cmake_minimum_required(VERSION 3.20)
project(fmt VERSION 8.0.1 LANGUAGES CXX)

#######################################################
# Files

set(PUBLIC_HDRS 
    include/fmt/args.h
    include/fmt/chrono.h
    include/fmt/color.h
    include/fmt/compile.h
    include/fmt/core.h
    include/fmt/format.h
    include/fmt/format-inl.h
    include/fmt/locale.h
    include/fmt/os.h
    include/fmt/ostream.h
    include/fmt/printf.h
    include/fmt/ranges.h
    include/fmt/xchar.h
)

set(SRC
    src/format.cc
    src/os.cc
)

add_library(fmt STATIC ${PUBLIC_HDRS} ${SRC})
target_include_directories(fmt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(TARGET fmt PROPERTY FOLDER "thrid_party")
