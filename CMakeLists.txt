cmake_minimum_required(VERSION 3.10)

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

#add_subdirectory("src")

# Build examples from the example directory
option(BUILD_C_EXAMPLES "Build C example programs" OFF)
option(BUILD_FORTRAN_EXAMPLES "Build Fortran example programs" OFF)
add_subdirectory("examples")

# Small testing
add_subdirectory("test")

# ======================================================================================================================
# Stopwatch Library
# ======================================================================================================================

find_package(PAPI REQUIRED)

add_library(stopwatch
        STATIC
        src/stopwatch.c
        src/str_table.c
        src/str_table.h
        src/call_tree.c
        src/call_tree.h
        ${CMAKE_SOURCE_DIR}/include/stopwatch/stopwatch.h
        ${CMAKE_SOURCE_DIR}/include/stopwatch/fstopwatch.F03
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
        #call_tree
        #str_table
        )

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

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindPAPI.cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/StopwatchConfig.cmake
        DESTINATION ${INSTALL_CONFIGDIR}
        )
