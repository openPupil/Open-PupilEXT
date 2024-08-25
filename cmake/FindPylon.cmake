# This file was reused from the following GitHub repository: https://github.com/lagadic/visp/blob/master/cmake/FindPylon.cmake


#############################################################################
#
# ViSP, open source Visual Servoing Platform software.
# Copyright (C) 2005 - 2019 by Inria. All rights reserved.
#
# This software is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# See the file LICENSE.txt at the root directory of this source
# distribution for additional information about the GNU GPL.
#
# For using ViSP with software that can not be combined with the GNU
# GPL, please contact Inria about acquiring a ViSP Professional
# Edition License.
#
# See http://visp.inria.fr for more information.
#
# This software was developed at:
# Inria Rennes - Bretagne Atlantique
# Campus Universitaire de Beaulieu
# 35042 Rennes Cedex
# France
#
# If you have questions regarding the use of this file, please contact
# Inria at visp@inria.fr
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# Description:
# Try to find Basler Pylon library API
#
# PYLON_FOUND
# PYLON_INCLUDE_DIRS
# PYLON_LIBRARIES
# PYLON_VERSION
#
# Authors:
# Wenfeng CAI
# Fabien Spindler : compat with OSX and Windows
#
#############################################################################


if(APPLE)
    find_path(PYLON_INCLUDE_DIR pylon/PylonIncludes.h)
    find_path(PYLON_BASE_INCLUDE_DIR
            NAMES Base/GCTypes.h
            HINTS ${PYLON_INCLUDE_DIR}/Headers
            PATH_SUFFIXES GenICam)

    find_library(PYLON_LIBRARIES pylon)

    find_program(PYLON_CONFIG pylon-config
            HINTS ${PYLON_INCLUDE_DIR}
            PATH_SUFFIXES Versions/A/Resources/Tools)

    if(PYLON_INCLUDE_DIR)
        list(APPEND PYLON_INCLUDE_DIRS ${PYLON_INCLUDE_DIR})
    endif()
    if(PYLON_BASE_INCLUDE_DIR)
        list(APPEND PYLON_INCLUDE_DIRS ${PYLON_BASE_INCLUDE_DIR})
    endif()

    if(PYLON_CONFIG)
        execute_process(COMMAND ${PYLON_CONFIG} "--version"
                OUTPUT_VARIABLE PYLON_VERSION_TMP
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REPLACE "-n" "" PYLON_VERSION ${PYLON_VERSION_TMP})
    endif()

    if(PYLON_INCLUDE_DIRS AND PYLON_LIBRARIES)
        set(PYLON_FOUND TRUE)
    endif()

    mark_as_advanced(PYLON_BASE_INCLUDE_DIR)

elseif(MSVC)

    find_path(PYLON_INCLUDE_DIR pylon/PylonIncludes.h
            PATHS "${PYLON_HOME}/include")

    set(PYLON_LIB_SEARCH_PATH "${PYLON_HOME}/lib/x64")

    find_library(PYLON_BASE_LIBRARY
            NAMES PylonBase_v6_2.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})
    find_library(PYLON_GCBASE_LIBRARY
            NAMES GCBase_MD_VC141_v3_1_Basler_pylon.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})
    find_library(PYLON_GENAPI_LIBRARY
            NAMES GenApi_MD_VC141_v3_1_Basler_pylon.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})
    find_library(PYLON_UTILITY_LIBRARY
            NAMES PylonUtility_v6_2.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})
    find_library(PYLON_GUI_LIBRARY
            NAMES PylonGUI_v6_2.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})
    find_library(PYLON_C_LIBRARY
            NAMES PylonC.lib
            PATHS ${PYLON_LIB_SEARCH_PATH})

    if(PYLON_INCLUDE_DIR)
        list(APPEND PYLON_INCLUDE_DIRS ${PYLON_INCLUDE_DIR})
    endif()
    if(PYLON_BASE_LIBRARY AND PYLON_GCBASE_LIBRARY)
        list(APPEND PYLON_LIBRARIES ${PYLON_BASE_LIBRARY})
        list(APPEND PYLON_LIBRARIES ${PYLON_GCBASE_LIBRARY})
        list(APPEND PYLON_LIBRARIES ${PYLON_GENAPI_LIBRARY})
        list(APPEND PYLON_LIBRARIES ${PYLON_UTILITY_LIBRARY})
        list(APPEND PYLON_LIBRARIES ${PYLON_GUI_LIBRARY})
        list(APPEND PYLON_LIBRARIES ${PYLON_C_LIBRARY})
    endif()

    if(PYLON_INCLUDE_DIRS AND PYLON_LIBRARIES)
        set(PYLON_FOUND TRUE)
    endif()

    mark_as_advanced(
            PYLON_BASE_LIBRARY
            PYLON_GCBASE_LIBRARY
            PYLON_GENAPI_LIBRARY
            PYLON_UTILITY_LIBRARY
            PYLON_GUI_LIBRARY
            PYLON_C_LIBRARY
    )

elseif(UNIX)
    set(PYLON_ROOT_SEARCH_PATH /opt/pylon5)
    set(PYLON_ROOT_SEARCH_PATH ${PYLON_ROOT_SEARCH_PATH} /opt/pylon)
    # For more possible versions, just add more paths below.
    # list(APPEND PYLON_ROOT_SEARCH_PATH "/somepath/include")

    find_program(PYLON_CONFIG pylon-config
            PATHS ${PYLON_ROOT}
            PATHS ${PYLON_ROOT}
            PATHS ${PYLON_ROOT_SEARCH_PATH}
            PATH_SUFFIXES bin)

    if(PYLON_CONFIG)
        execute_process(COMMAND ${PYLON_CONFIG} "--version"
                OUTPUT_VARIABLE PYLON_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${PYLON_CONFIG} "--libs" "--libs-rpath"
                OUTPUT_VARIABLE PYLON_LIBRARIES
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${PYLON_CONFIG} "--cflags-only-I"
                OUTPUT_VARIABLE PYLON_INC_TMP
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REPLACE "-I" "" PYLON_INCLUDE_DIRS ${PYLON_INC_TMP})

        set(PYLON_FOUND TRUE)
    endif()
endif()

mark_as_advanced(
        PYLON_INCLUDE_DIR
        PYLON_INCLUDE_DIRS
        PYLON_LIBRARIES
        PYLON_CONFIG
)