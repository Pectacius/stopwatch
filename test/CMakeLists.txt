# ======================================================================================================================
# Building test suites
# ======================================================================================================================
include(CTest)

if (BUILD_TESTING)
    add_executable(initializing_unittests "stopwatch_initializing_tests.c")
    target_link_libraries(initializing_unittests PRIVATE stopwatch)

    add_executable(measurement_unittests "stopwatch_measurement_tests.c")
    target_link_libraries(measurement_unittests PRIVATE stopwatch)

    add_executable(print_table_unittests "print_table_tests.c")
    # Must support at least 3.10 so address sanitize is appended to the end of target_link_libraries
    # ASan is supported by all modern compilers under the sum i.e. gcc/clang/xcode/msvc
    target_compile_options(print_table_unittests PRIVATE -fsanitize=address)
    target_link_libraries(print_table_unittests PRIVATE stopwatch -fsanitize=address)

    add_executable(call_tree_unittests "call_tree_tests.c" "${CMAKE_SOURCE_DIR}/src/call_tree.c")
    target_include_directories(call_tree_unittests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_compile_options(call_tree_unittests PRIVATE -fsanitize=address)
    target_link_libraries(call_tree_unittests PRIVATE -fsanitize=address)

    add_test(stopwatch_initializing_tests stopwatch_initializing_unittests)
    add_test(stopwatch_measurement_tests stopwatch_measurement_unittests)
    add_test(print_table_tests print_table_unittests)
    add_test(call_tree_tests call_tree_unittests)
endif ()