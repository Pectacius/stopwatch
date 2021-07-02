
add_executable(matmul matmul.c)
target_link_libraries(matmul stopwatch)
target_compile_options(matmul PUBLIC -O0)


add_executable(matmul_dist matmul_dist.c)
target_link_libraries(matmul_dist stopwatch)
target_compile_options(matmul_dist PUBLIC -O0)