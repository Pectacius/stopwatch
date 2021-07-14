# ======================================================================================================================
# Building test suites
# ======================================================================================================================
option(ADDRESS_SANITIZE "Enable Address Sanitizer" OFF)
include(CTest)

if (BUILD_TESTING)
    add_executable(initializing_unittests "stopwatch_initializing_tests.c")
    target_link_libraries(initializing_unittests PRIVATE stopwatch)

    add_executable(measurement_unittests "stopwatch_measurement_tests.c")
    target_link_libraries(measurement_unittests PRIVATE stopwatch)

    add_executable(print_table_unittests "print_table_tests.c")
    target_link_libraries(print_table_unittests PRIVATE stopwatch)

    if (ADDRESS_SANITIZE)
        target_compile_options(print_table_unittests PRIVATE -fsanitize=address)
        target_link_options(print_table_unittests PRIVATE -fsanitize=address)
    endif ()

    add_test(stopwatch_initializing_tests stopwatch_initializing_unittests)
    add_test(stopwatch_measurement_tests stopwatch_measurement_unittests)
    add_test(print_table_tests print_table_unittests)
endif ()