# - Attempt to find PAPI
# Will automatically fail on non-UNIX systems as PAPI at the moment seems to be only available on UNIX systems.
#
# Based off the structure from:
#   https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Find-Libraries
#
# On success the following variables will be defined
#   PAPI_FOUND - system has PAPI installed
#   PAPI_INCLUDE_DIRS - the PAPI header include directories
#   PAPI_LIBRARIES - the PAPI library to link with
#
# It will also define the following imported targets
#   PAPI::PAPI
#=======================================================================================================================

if (UNIX)
    # The installation instructions from the official PAPI documentation:
    #   https://bitbucket.org/icl/papi/wiki/Downloading-and-Installing-PAPI.md
    # suggest that the environment variable <PAPI_DIR> should be set to where PAPI is installed. However environment
    # variables are not recommended hence the cached variable <PAPI_DIR> (note the difference between the environment
    # variable) is used to find the PAPI install location. Providing this variable is not required as some hard coded
    # paths where possible PAPI installation could be are listed.

    find_path(PAPI_INCLUDE_DIR
            NAMES papi.h
            PATHS $ENV{PAPI_DIR}/include
            /usr/include
            /usr/local/include
            /opt/local/include
            ${CMAKE_SOURCE_DIR}/include
            REQUIRED)

    find_library(PAPI_LIBRARY
            NAMES papi
            PATHS $ENV{PAPI_DIR}/lib
            /usr/lib64
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            ${CMAKE_SOURCE_DIR}/lib
            REQUIRED)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set PAPI_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(
            PAPI
            DEFAULT_MSG
            PAPI_LIBRARY
            PAPI_INCLUDE_DIR
    )

    mark_as_advanced(PAPI_INCLUDE_DIR PAPI_LIBRARY)

    # Make PAPI an imported target and also include its public header papi.h
    if (PAPI_FOUND AND NOT TARGET PAPI::PAPI)
        add_library(PAPI::PAPI SHARED IMPORTED)
        set_target_properties(PAPI::PAPI PROPERTIES IMPORTED_LOCATION ${PAPI_LIBRARY})
        target_include_directories(PAPI::PAPI INTERFACE ${PAPI_INCLUDE_DIR})
    endif ()

else ()
    message(FATAL_ERROR "PAPI is only available on UNIX platforms")
endif ()