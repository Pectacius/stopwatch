
add_executable(matmul papi_matmul.c)
target_link_libraries(matmul stopwatch)


add_executable(matmul_dist matmul_dist.c)
target_link_libraries(matmul_dist stopwatch)