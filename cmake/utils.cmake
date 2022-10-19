# From https://stackoverflow.com/a/31987079
function(tndr_set_catalog_tree SOURCE)
    foreach(FILE ${SOURCE})
        # Get the directory of the source file
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

        # Remove common directory prefix to make the group
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

        # Make sure we are using windows slashes
        string(REPLACE "/" "\\" GROUP "${GROUP}")

        # Group into "Source Files" and "Header Files"
        if("${FILE}" MATCHES ".*\\.cpp")
            set(GROUP "${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.h")
            set(GROUP "${GROUP}")
        endif()

        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction()

function(tndr_set_compile_options TARGET_NAME)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        target_compile_options(${TARGET_NAME} PRIVATE
            /permissive-
            /W4
            /w14640
            /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            /w14263 # 'function': member function does not override any base class virtual member function
            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
            /w14287 # 'operator': unsigned/negative constant mismatch
            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
            /w14296 # 'operator': expression is always 'boolean_value'
            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545 # expression before comma evaluates to a function which is missing an argument list
            /w14546 # function call before comma missing argument list
            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 # expression has no effect; expected expression with side-effect
            /w14619 # pragma warning: there is no warning number 'number'
            /w14640 # Enable warning on thread un-safe static member initialization
            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905 # wide string literal cast to 'LPSTR'
            /w14906 # string literal cast to 'LPWSTR'
            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
            /arch:AVX
            -Xclang -Wno-gnu-zero-variadic-macro-arguments
            -Xclang -Wall
            -Xclang -Wpedantic
            -Xclang -Wno-signed-enum-bitfield
            -Xclang -Wno-documentation-unknown-command
            -Xclang -Wno-missing-prototypes
            -Xclang -Wno-documentation
            -Xclang -Wno-ctad-maybe-unsupported
            -Xclang -Wno-c++98-compat
            -Xclang -Wno-c++98-compat-pedantic
            -Xclang -Wsequence-point
            -Xclang -Wunreachable-code
            -Xclang -Wshadow
            -Xclang -Wdelete-non-virtual-dtor
            -Xclang -Wnull-dereference
            -Xclang -Wdouble-promotion
            -Xclang -Wold-style-cast
            -Xclang -Waddress
            -Xclang -Warray-bounds
            -Xclang -Wchar-subscripts
            -Xclang -Wformat
            -Xclang -Winit-self
            -Xclang -Wlogical-not-parentheses
            -Xclang -Wuninitialized
            -Xclang -Wmemset-transposed-args
            -Xclang -Wnarrowing
            -Xclang -Wreturn-type
            -Xclang -Wnonnull
            -Xclang -Wsequence-point
            -Xclang -Wsign-compare
            -Xclang -Wsizeof-pointer-div
            -Xclang -Wsizeof-pointer-memaccess
            -Xclang -Wswitch
            -Xclang -Wtautological-compare
            -Xclang -Wtrigraphs
            -Xclang -Wunknown-pragmas
            -Xclang -Wunused-function
            -Xclang -Wunused-label
            -Xclang -Wunused-value
            -Xclang -Wunused-variable
            -Xclang -Wvolatile-register-var
            -Xclang -Wno-zero-as-null-pointer-constant
            -Xclang -Wno-extra-semi-stmt
            -Xclang -Wconversion
        )

    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(${TARGET_NAME} PRIVATE
            /permissive-
            /W4
            /w14640
            /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            /w14263 # 'function': member function does not override any base class virtual member function
            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
            /w14287 # 'operator': unsigned/negative constant mismatch
            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside the for-loop scope
            /w14296 # 'operator': expression is always 'boolean_value'
            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545 # expression before comma evaluates to a function which is missing an argument list
            /w14546 # function call before comma missing argument list
            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 # expression has no effect; expected expression with side-effect
            /w14619 # pragma warning: there is no warning number 'number'
            /w14640 # Enable warning on thread un-safe static member initialization
            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905 # wide string literal cast to 'LPSTR'
            /w14906 # string literal cast to 'LPWSTR'
            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
            /wd5030 # attribute 'asdf' is not recognized
            /wd5105 # macro expansion producing 'defined' has undefined behavior
            /wd4251 # 'type' needs to have dll-interface to be used by clients of 'type'
            /wd4530 # C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
            /wd4324 # Structure was padded due to alignment specifier
            /wd26812 # The enum type "x" is unscoped. Prefer "enum class" over "enum"
            /arch:AVX
        )

    # string(REGEX REPLACE "/W[0-9]+" "/W4" CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS})
    # string(REGEX REPLACE "/W[0-9]+" "/W4" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS})
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wpedantic
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wmisleading-indentation # (only in GCC >= 6.0) warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
            -Wduplicated-branches # (only in GCC >= 7.0) warn if if / else branches have duplicated code
            -Wlogical-op # (only in GCC) warn about logical operations being used where bitwise were probably wanted
            -Wnull-dereference # (only in GCC >= 6.0) warn if a null dereference is detected
            -Wuseless-cast # (only in GCC >= 4.8) warn if you perform a cast to the same type
            -Wall
            -Wextra
            -Wpedantic
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wsequence-point
            -Wunreachable-code
            -Wdelete-non-virtual-dtor
            -Wnull-dereference
            -Wdouble-promotion
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wrestrict
            -Wold-style-cast
            -Waddress
            -Warray-bounds=1
            -Wchar-subscripts
            -Wformat
            -Wint-in-bool-context
            -Winit-self
            -Wlogical-not-parentheses
            -Wuninitialized
            -Wmaybe-uninitialized
            -Wmemset-elt-size
            -Wmemset-transposed-args
            -Wmisleading-indentation
            -Wnarrowing
            -Wreturn-type
            -Wnonnull
            -Wsequence-point
            -Wsign-compare
            -Wsizeof-pointer-div
            -Wsizeof-pointer-memaccess
            -Wswitch
            -Wtautological-compare
            -Wtrigraphs
            -Wunknown-pragmas
            -Wunused-function
            -Wunused-label
            -Wunused-value
            -Wunused-variable
            -Wvolatile-register-var
            -Wno-zero-as-null-pointer-constant
            -Wno-extra-semi-stmt
            -Wconversion
            -Wno-attributes
        )

    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wpedantic
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wformat=2
            -Wnarrowing
            -Wdelete-non-virtual-dtor
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wsequence-point
            -Wunreachable-code
            -Wnull-dereference
            -Wdouble-promotion
            -Waddress
            -Warray-bounds
            -Wchar-subscripts
            -Winit-self
            -Wlogical-not-parentheses
            -Wuninitialized
            -Wmemset-transposed-args
            -Wreturn-type
            -Wnonnull
            -Wsequence-point
            -Wsign-compare
            -Wsizeof-pointer-div
            -Wsizeof-pointer-memaccess
            -Wswitch
            -Wtautological-compare
            -Wtrigraphs
            -Wunknown-pragmas
            -Wunused-function
            -Wunused-label
            -Wunused-value
            -Wunused-variable
            -Wvolatile-register-var
            -Wno-zero-as-null-pointer-constant
            -Wno-extra-semi-stmt
            -Wno-unknown-attributes
        )
    endif()
endfunction()

function(tndr_add_module NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;INCLUDES;PUBLIC_DEPENDENCIES;PRIVATE_DEPENDENCIES;INCLUDE_DIRS;PRIVATE_COMPILER_FLAGS" ${ARGN})

    tndr_set_catalog_tree("${ARGS_SOURCES}")

    if(TUNDRA_MONOLITHIC_BUILD)
        add_library(${NAME} STATIC ${ARGS_SOURCES})
    else()
        add_library(${NAME} SHARED ${ARGS_SOURCES})
    endif()

    target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${NAME} PUBLIC ${ARGS_PUBLIC_DEPENDENCIES})
    target_link_libraries(${NAME} PRIVATE ${ARGS_PRIVATE_DEPENDENCIES})
    target_include_directories(${NAME} PRIVATE ${ARGS_INCLUDE_DIRS})
    target_include_directories(${NAME} PUBLIC ${CMAKE_BINARY_DIR}/exports/${NAME})
    target_compile_options(${NAME} PRIVATE ${ARGS_PRIVATE_COMPILER_FLAGS})
    target_compile_features(${NAME} PRIVATE cxx_std_20)

    if(NOT TUNDRA_MONOLITHIC_BUILD)
        if(UNIX)
            target_compile_options(${NAME} PRIVATE -fPIC)
        endif()
    endif()

    if(UNIX)
        find_package(Threads REQUIRED)
        target_link_libraries(${NAME} PUBLIC Threads::Threads -latomic -g)
    endif()

    target_compile_definitions(${NAME} PRIVATE
        $<IF:$<BOOL:${TUNDRA_MONOLITHIC_BUILD}>, TUNDRA_MONOLITHIC_BUILD=1, TUNDRA_MONOLITHIC_BUILD=0>
    )

    tndr_set_compile_options(${NAME})

    string(TOUPPER ${NAME} NAME_UPPER)
    generate_export_header(${NAME}
        EXPORT_MACRO_NAME ${NAME_UPPER}_API
        EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/${NAME}/${NAME}/${NAME}_export.h
    )

    install(TARGETS ${NAME} LIBRARY)

    set_property(TARGET ${NAME} PROPERTY FOLDER "modules")
endfunction()

function(tndr_add_executable NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;INCLUDES;PUBLIC_DEPENDENCIES;PRIVATE_DEPENDENCIES;INCLUDE_DIRS;PRIVATE_COMPILER_FLAGS" ${ARGN})

    tndr_set_catalog_tree("${ARGS_SOURCES}")

    # if(WIN32)
    # add_executable(${NAME} WIN32 ${ARGS_SOURCES})
    # elseif()
    # add_executable(${NAME} ${ARGS_SOURCES})
    # endif()
    add_executable(${NAME} ${ARGS_SOURCES})

    target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${NAME} PUBLIC ${ARGS_PUBLIC_DEPENDENCIES})
    target_link_libraries(${NAME} PRIVATE ${ARGS_PRIVATE_DEPENDENCIES})
    target_include_directories(${NAME} PRIVATE ${ARGS_INCLUDE_DIRS})
    target_compile_options(${NAME} PRIVATE ${ARGS_PRIVATE_COMPILER_FLAGS})

    if(WIN32)
        target_compile_definitions(${NAME} PRIVATE -DUNICODE -D_UNICODE)
    endif(WIN32)

    if(UNIX)
        find_package(Threads REQUIRED)
        target_link_libraries(${NAME} PUBLIC Threads::Threads -latomic -g)
    endif()

    target_compile_definitions(${NAME} PRIVATE
        $<IF:$<BOOL:${TUNDRA_MONOLITHIC_BUILD}>, TUNDRA_MONOLITHIC_BUILD=1, TUNDRA_MONOLITHIC_BUILD=0>
    )

    tndr_set_compile_options(${NAME})

    install(TARGETS ${NAME} RUNTIME)

    set_property(TARGET ${NAME} PROPERTY FOLDER "executables")
endfunction()
