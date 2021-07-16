if (BUILD_C_EXAMPLES)
    add_subdirectory("examples_c")
endif ()

if (BUILD_FORTRAN_EXAMPLES)
    enable_language(Fortran)
    add_subdirectory("examples_fortran")
endif ()
