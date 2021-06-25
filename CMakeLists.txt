# GEM is setting this as minimum version
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

project(libstopwatch)

find_package(PAPI REQUIRED)

add_library(libstopwatch STATIC src/stopwatch.c include/stopwatch/stopwatch.h)

target_include_directories(libstopwatch PUBLIC ${CMAKE_SOURCE_DIR}/include ${PAPI_INCLUDE_DIRS})
target_link_libraries(libstopwatch PRIVATE ${PAPI_LIBRARIES})


add_executable(matmul src/papi_matmul.c)
target_include_directories(matmul PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(matmul libstopwatch)


add_executable(matmul_dist src/matmul_dist.c)
target_include_directories(matmul_dist PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(matmul_dist libstopwatch)