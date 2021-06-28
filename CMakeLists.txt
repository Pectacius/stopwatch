# GEM is setting this as minimum version
cmake_minimum_required(VERSION 2.8.12)

project(libstopwatch)

# Prevent in-source builds as it pollutes the source directory
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(
            FATAL_ERROR
            "In-source build prohibited. Please make a build directory and rerun CMake."
    )
endif ()

# Allow PAPI to be found via a find_package as it does not provide any CMake targets itself
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(PAPI REQUIRED)

add_library(libstopwatch SHARED src/stopwatch.c include/stopwatch/stopwatch.h)

target_include_directories(libstopwatch PUBLIC ${CMAKE_SOURCE_DIR}/include ${PAPI_INCLUDE_DIRS})
target_link_libraries(libstopwatch PRIVATE ${PAPI_LIBRARIES})

# Build examples from the example directory
option(BUILD_EXAMPLES "Build example programs" OFF)

if (BUILD_EXAMPLES)
    add_subdirectory("examples")
endif ()
