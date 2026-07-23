cmake_minimum_required(VERSION 3.10)
project(shell C)

set(SOURCES
    main.c
    includes/utils.c
)

include_directories(includes)

add_executable(Cclicker ${SOURCES})

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(WAYLAND QUIET wayland-client)
    pkg_check_modules(X11 QUIET x11)

    if(DEFINED ENV{XDG_SESSION_TYPE} AND "$ENV{XDG_SESSION_TYPE}" STREQUAL "wayland")
        set(SELECTED_BACKEND WAYLAND)
    elseif(DEFINED ENV{WAYLAND_DISPLAY})
        set(SELECTED_BACKEND WAYLAND)
    elseif(DEFINED ENV{XDG_SESSION_TYPE} AND "$ENV{XDG_SESSION_TYPE}" STREQUAL "x11")
        set(SELECTED_BACKEND X11)
    elseif(DEFINED ENV{DISPLAY})
        set(SELECTED_BACKEND X11)
    elseif(WAYLAND_FOUND)
        set(SELECTED_BACKEND WAYLAND)
    elseif(X11_FOUND)
        set(SELECTED_BACKEND X11)
    endif()

    if(SELECTED_BACKEND STREQUAL "WAYLAND")
        if(NOT WAYLAND_FOUND)
            message(FATAL_ERROR "System is using Wayland, but wayland-client pkg-config was not found")
        endif()
        target_compile_definitions(Cclicker PRIVATE USE_WAYLAND)
        target_include_directories(Cclicker PRIVATE ${WAYLAND_INCLUDE_DIRS})
        target_compile_options(Cclicker PRIVATE ${WAYLAND_CFLAGS_OTHER})
        target_link_libraries(Cclicker PRIVATE ${WAYLAND_LIBRARIES})
    elseif(SELECTED_BACKEND STREQUAL "X11")
        if(NOT X11_FOUND)
            message(FATAL_ERROR "System is using X11, but x11 pkg-config was not found")
        endif()
        target_compile_definitions(Cclicker PRIVATE USE_X11)
        target_include_directories(Cclicker PRIVATE ${X11_INCLUDE_DIRS})
        target_compile_options(Cclicker PRIVATE ${X11_CFLAGS_OTHER})
        target_link_libraries(Cclicker PRIVATE ${X11_LIBRARIES})
    else()
        message(FATAL_ERROR "Cannot determine whether the system uses Wayland or X11; set XDG_SESSION_TYPE, WAYLAND_DISPLAY, or DISPLAY")
    endif()
endif()

if(WIN32)
    if(MSVC)
        # Disable MSVC secure warnings and provide compatibility for GCC-style attributes and ssize_t
        target_compile_definitions(Cclicker PRIVATE _CRT_SECURE_NO_WARNINGS)
        # Force-include a small header that defines ssize_t and neutralizes __attribute__ on MSVC
        target_compile_options(Cclicker PRIVATE /FI"${CMAKE_SOURCE_DIR}\\includes\\msvc_compat.h")
    endif()
else()
    target_link_libraries(Cclicker PRIVATE m)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Cclicker PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Cclicker PRIVATE /W4)
endif()