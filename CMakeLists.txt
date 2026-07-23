cmake_minimum_required(VERSION 3.10)
project(shell C)

set(SOURCES
    main.c
    includes/utils.c
)

if(NOT WIN32)
    list(APPEND SOURCES includes/posix_input.c)
endif()

include_directories(includes)

add_executable(Cclicker ${SOURCES})

if(WIN32)
    if(MSVC)
        # Disable MSVC secure warnings and provide compatibility for GCC-style attributes and ssize_t
        target_compile_definitions(Cclicker PRIVATE _CRT_SECURE_NO_WARNINGS)
        # Force-include a small header that defines ssize_t and neutralizes __attribute__ on MSVC
        target_compile_options(Cclicker PRIVATE /FI"${CMAKE_SOURCE_DIR}\\includes\\msvc_compat.h")
    endif()
else()
    target_link_libraries(Cclicker m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Cclicker PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Cclicker PRIVATE /W4)
endif()
