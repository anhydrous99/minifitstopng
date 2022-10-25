#[=================================[.rst:

FindASCDM
---------

Finds the ASCDM library.

Import Targets
^^^^^^^^^^^^^^

This module procides the following import targets, if found:

``CIAO::ASCDM``
  The ASCDM library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``ASCDM_FOUND``
  True if the system has the ASCDM library
``ASCDM_INCLUDE_DIRS``
  Include directories needed to use ASCDM.
``ASCDM_LIBRARIES``
  Libraries needed to link to ASCDM.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

#]=================================]

if ("$ENV{ASCDS_INSTALL}" STREQUAL "")
    message(FATAL_ERROR "ASCDM wasn't found.")
endif ()

find_path(ASCDM_INCLUDE_DIR
        NAMES ascdm.h
        HINTS "$ENV{ASCDS_INSTALL}/include")

find_library(ASCDM_LIBRARY
        NAMES ascdm
        HINTS "$ENV{ASCDS_INSTALL}/lib")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASCDM
        FOUND_VAR ASCDM_FOUND
        REQUIRED_VARS
        ASCDM_LIBRARY
        ASCDM_INCLUDE_DIR)

if (ASCDM_FOUND)
    set(ASCDM_LIBRARIES ${ASCDM_LIBRARY})
    set(ASCDM_INCLUDE_DIRS ${ASCDM_INCLUDE_DIR})
endif ()

if (ASCDM_FOUND AND NOT TARGET CIAO::ASCDM)
    add_library(CIAO::ASCDM UNKNOWN IMPORTED)
    set_target_properties(CIAO::ASCDM PROPERTIES
            IMPORTED_LOCATION "${ASCDM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ASCDM_INCLUDE_DIR}")
endif ()
