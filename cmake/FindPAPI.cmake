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
    # suggest that the environment variable <PAPI_DIR> should be set to where PAPI is installed. This variable is used
    # to find where PAPI is installed. Default locations are also searched if the environment variable is not set.

    # First use pkg-config to find PAPI. If that fails use the ENV hint to find PAPI. Set it to quiet as pkg-config is
    # not required, hence an extra check to see if it is actually found.
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        message(STATUS "Looking for PAPI via pkg-config")
        pkg_search_module(PAPI IMPORTED_TARGET papi)
    endif()

    # If PAPI_FOUND is defined here, then it must have been found with pkg-config
    if (PAPI_FOUND)
        message(STATUS "Found PAPI via pkg-config")
        find_package_handle_standard_args(PAPI REQUIRED_VARS pkgcfg_lib_PAPI_papi PAPI_CFLAGS PAPI_LDFLAGS VERSION_VAR PAPI_VERSION)

        # Hack: pkg-config returns PAPI_LDFLAGS as a semicolon-separated list.
        # This is incompatible with the space-separated list needed by set_target_properties,
        # so use string replacement
        string(REPLACE ";" " " PAPI_LDFLAGS_SPACE ${PAPI_LDFLAGS})

        include(FindPackageHandleStandardArgs)

        if (NOT TARGET PAPI::PAPI)
            add_library(PAPI::PAPI STATIC IMPORTED)
            set_target_properties(
                    PAPI::PAPI
                    PROPERTIES
                    IMPORTED_LOCATION ${pkgcfg_lib_PAPI_papi} # Use the internal library found by pkgconfig
                    COMPILE_OPTIONS ${PAPI_CFLAGS}
                    LINK_FLAGS ${PAPI_LDFLAGS_SPACE})

        endif ()
    else ()
        message(STATUS "Looking for PAPI via find_path")
        find_path(PAPI_INCLUDE_DIR
                NAMES papi.h
                HINTS ENV PAPI_DIR
                PATH_SUFFIXES include
                REQUIRED)

        find_library(PAPI_LIBRARY
                NAMES libpapi.a papi
                HINTS ENV PAPI_DIR
                PATH_SUFFIXES lib
                REQUIRED)

        message(STATUS "Found PAPI via find_path")

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
           add_library(PAPI::PAPI STATIC IMPORTED)
           set_target_properties(
                   PAPI::PAPI
                   PROPERTIES
                   IMPORTED_LOCATION ${PAPI_LIBRARY}
                   INTERFACE_INCLUDE_DIRECTORIES ${PAPI_INCLUDE_DIR})
       endif ()
    endif()


else ()
    message(FATAL_ERROR "PAPI is only available on UNIX platforms")
endif ()
