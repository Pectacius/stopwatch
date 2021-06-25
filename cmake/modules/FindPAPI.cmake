# - Attempt to find PAPI
# Will automatically fail on non-UNIX systems as PAPI at the moment seems to be only available on UNIX systems.
#
# Based off the structure from:
#   https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Find-Libraries
#
# On success the following variables will be defined
# PAPI_FOUND - system has PAPI installed
# PAPI_INCLUDE_DIRS - the PAPI header include directories
# PAPI_LIBRARIES - the PAPI library to link with

if (UNIX)
    # The installation instructions from the official PAPI documentation:
    #   https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md
    # suggest that the environment variable <PAPI_DIR> should be set to where PAPI is installed, hence the directory
    # specified by <PAPI_DIR> will be searched first. Some other hard coded directories are also provided for fallback.
    # If the headers and libraries cannot be found in any of these directories, it is assumed that PAPI is not installed.

    message($ENV{PAPI_DIR})

    find_path(PAPI_INCLUDE_DIR
            NAMES   papi.h
            PATHS   $ENV{PAPI_DIR}/include
                    /usr/include
                    /usr/local/include
                    /opt/local/include
                    ${CMAKE_SOURCE_DIR}/include
            REQUIRED)

    find_library(PAPI_LIBRARY
            NAMES   papi
            PATHS   $ENV{PAPI_DIR}/lib
                    /usr/lib64
                    /usr/lib
                    /usr/local/lib
                    /opt/local/lib
                    ${CMAKE_SOURCE_DIR}/lib
            REQUIRED)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set PAPI_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(PAPI  DEFAULT_MSG
            PAPI_LIBRARY PAPI_INCLUDE_DIR)


    mark_as_advanced(PAPI_INCLUDE_DIR PAPI_LIBRARY)

    # Variables to define
    set(PAPI_LIBRARIES ${PAPI_LIBRARY})
    set(PAPI_INCLUDE_DIRS ${PAPI_INCLUDE_DIR})

else()
    message(FATAL_ERROR "PAPI is only available on UNIX platforms")
endif ()