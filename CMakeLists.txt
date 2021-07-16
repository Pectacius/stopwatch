cmake_minimum_required(VERSION 2.8.12)

project(libstopwatch LANGUAGES C)

#=======================================================================================================================
# General Setup
#=======================================================================================================================

# Prevent in-source builds as it pollutes the source directory
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
            FATAL_ERROR
            "In-source build prohibited. Please make a build directory and rerun CMake."
    )
endif ()

# Allow PAPI to be found via a find_package as it does not provide any CMake targets itself
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(PAPI REQUIRED)

add_library(stopwatch
        SHARED
        src/stopwatch.c
        include/stopwatch/stopwatch.h
        include/stopwatch/fstopwatch.F03
        src/str_table.c
        src/str_table.h
        )

# Consumers of this library are going to need its public headers but not the PAPI headers
target_include_directories(stopwatch
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

target_link_libraries(stopwatch
        PRIVATE
        PAPI::PAPI
        m
        )

# Build examples from the example directory
option(BUILD_C_EXAMPLES "Build C example programs" OFF)
option(BUILD_FORTRAN_EXAMPLES "Build Fortran example programs" OFF)
add_subdirectory("examples")

# Small testing
add_subdirectory("test")

#=======================================================================================================================
# Installation
#=======================================================================================================================
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Stopwatch)

install(TARGETS stopwatch
        EXPORT stopwatch-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(stopwatch PROPERTIES EXPORT_NAME Stopwatch)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT stopwatch-targets
        FILE
        StopwatchTargets.cmake
        NAMESPACE
        Stopwatch::
        DESTINATION
        ${INSTALL_CONFIGDIR}
        )

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/StopwatchConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/StopwatchConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
        )

install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/cmake/FindPAPI.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/StopwatchConfig.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
        )

#=======================================================================================================================
# Exporting from the build tree
#=======================================================================================================================
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindPAPI.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/FindPAPI.cmake
        COPYONLY)

export(EXPORT stopwatch-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/StopwatchTargets.cmake
        NAMESPACE Stopwatch::)

#=======================================================================================================================
# Register package in the User Package Registry
#=======================================================================================================================
export(PACKAGE Stopwatch)
