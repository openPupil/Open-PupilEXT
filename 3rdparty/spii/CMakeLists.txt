# Author: petter.strandmark@gmail.com (Petter Strandmark)
# set(CMAKE_CXX_STANDARD 11) requires at least CMake 3.1.
cmake_minimum_required(VERSION 3.1)
project(Spii C CXX)

# Some tests use std::uniform_real, which is a C++11 feature; other tests
# use generic lambdas, which is a C++14 feature. Ideally, we would declare
# this only for tests?
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Default locations to search for on various platforms.
list(APPEND SEARCH_LIBS /usr/lib)
list(APPEND SEARCH_LIBS /usr/local/lib)
list(APPEND SEARCH_LIBS /usr/local/homebrew/lib) # Mac OS X
list(APPEND SEARCH_LIBS /opt/local/lib)

list(APPEND SEARCH_HEADERS /usr/include)
list(APPEND SEARCH_HEADERS /usr/local/include)
list(APPEND SEARCH_HEADERS /usr/local/homebrew/include) # Mac OS X
list(APPEND SEARCH_HEADERS /opt/local/include)

enable_testing()
set(CTEST_OUTPUT_ON_FAILURE TRUE)

# Multithreading using OpenMP
option(OPENMP
       "Enable multi-threading (requires OpenMP)"
       ON)

# For Clang-based tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (OPENMP)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    message("-- Found OpenMP.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DUSE_OPENMP)
  else()
    message("-- Can't find OpenMP. Continuing without it.")
  endif()
else ()
  message("-- OpenMP is disabled. Continuing without it.")
endif ()

# sym-ildl is optional.
find_path(SYM_ILDL_INCLUDE NAMES lil_sparse_matrix.h )
if (EXISTS ${SYM_ILDL_INCLUDE})
	include_directories(${SYM_ILDL_INCLUDE})
	add_definitions("-DUSE_SYM_ILDL=1")
	message("-- Found sym-ildl at ${SYM_ILDL_INCLUDE}. Will use it if requested to.")
endif ()

#FIND_PATH(EIGEN_INCLUDE NAMES signature_of_eigen3_matrix_library PATHS /usr/include/eigen3 /usr/local/include/eigen3 /usr/lib/eigen3 thirdparty/Eigen)
#IF (NOT EIGEN_INCLUDE)
#	MESSAGE(FATAL_ERROR "Eigen not found.")
#ENDIF ()
#MESSAGE("-- Using Eigen at " ${EIGEN_INCLUDE})

include_directories(
  include
  SYSTEM thirdparty/Catch
  SYSTEM thirdparty/Eigen
  SYSTEM thirdparty/meschach
  SYSTEM thirdparty/ceres-solver-data
  )

file(GLOB SPII_HEADERS ${CMAKE_SOURCE_DIR}/include/spii/*.h)
file(GLOB SPII_THIRD_PARTY_HEADERS ${CMAKE_SOURCE_DIR}/include/spii-thirdparty/*.h)
install(FILES ${SPII_HEADERS} DESTINATION include/spii)
install(FILES ${SPII_THIRD_PARTY_HEADERS} DESTINATION include/spii-thirdparty)

# Library dependencies
set (SPII_LIBRARY_DEPENDENCIES)

# OpenMP needs to be included as a library on some platforms.
if (OPENMP_FOUND)
  if (NOT MSVC)
    list(APPEND SPII_LIBRARY_DEPENDENCIES gomp)
  endif (NOT MSVC)
endif ()

# Change the default build type from Debug to Release, while still
# supporting overriding the build type.
if (NOT CMAKE_BUILD_TYPE)
  message("-- No build type specified; defaulting to CMAKE_BUILD_TYPE=Release.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif ()

# C++11 support.
include(cmake/EnableCPP11.cmake)
include(cmake/CheckGenericLambdas.cmake)

#
# Clang and GCC settings
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
  # No warnings for C. The Meschach library contains really old code.
  # -fPIC for building a shared library.
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -w -fpic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -fpic")
  # -Wunused-function triggers incorrect warnings for variadic template
  # recursion base case with Clang 3.2.

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  	message("-- Debug mode enabled for Gcc/Clang; adding support for Gcov.")
  	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  endif ()

endif ()

#
# MSVC settings
#
if (MSVC)
  # Disable deprecation warning for standard functions.
  add_definitions("/wd4996")
  # To get rid of annoying min and max macros if windows.h
  # is included.
  add_definitions("-DNOMINMAX=1")
  # Macro redefinitions.
  add_definitions("/wd4005")

  # Workaround for ICE in Eigen.
  add_definitions("-DEIGEN_DONT_PARALLELIZE")
endif ()

#
# Emscripten settings
#
set(EXECUTABLE_EXTENSION "")
if (EMSCRIPTEN)
  set(EXECUTABLE_EXTENSION ".html")
  add_definitions("-DEMSCRIPTEN=1")
  # We want exceptions.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#
# Packaging
#
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_FILE_NAME "Spii")
include(CPack)

add_subdirectory(thirdparty/meschach)
add_subdirectory(data)
add_subdirectory(benchmarks)
add_subdirectory(examples)
add_subdirectory(large_tests)
add_subdirectory(source)
add_subdirectory(tests)
