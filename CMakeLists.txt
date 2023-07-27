# Description
# When using find_package() it will search in the CMAKE_MODULE_PATH to find the Find<package-name>.cmake file of a lib
# When using find_package(<package-name> REQUIRED) , it will throw an error, if the package whas not found.
# When a package is found several env variables will be set: <package-name>_FOUND, <package-name>_INCLUDE_DIR, <package-name>_LIBRARIES, <package-name>_LINK_DIRECTORIES
# Source: http://wiki.icub.org/wiki/CMake_and_FIND_PACKAGE
# When using find_package() in config mode, the <package-name>Config.cmake must be added together with find_package(<package-name> CONFIG REQUIRED)
# CMAKE_PREFIX_PATH --> is the base folder for the search engine
# CMAKE_MODULE_PATH --> for find <package-name>.cmake. Note cmake does not search in CMAKE_PREFIX_PATH for <package-name>.cmake
# <package-name>_DIR --> It will be searched there for the <package-name>config.cmake file
# The variables should be chained together with the following command set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/opencv")
# In the modern way it can also be used list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# A value can be set with set(OpenCV_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/")
# Messages can be printed out with message(STATUS "OpenCV library status:")

# The folders can be checked after compilation in the CMakeCache.txt in the build folder to check if anything is pointed correctly

# During build it is important to set the VCPKG_TARGET_TRIPLET env variable

cmake_minimum_required(VERSION 3.15)
project(PupilEXT)

# On linux with GCC and QT 5.15.2 setting standard here is not recognized and is overwritten by QT to gnu++14
#set(CMAKE_CXX_STANDARD 17)

# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# CUSTOM SETTINGS - PLEASE SPECIFY ACCORDING TO YOUR SYSTEM

# Build the spii lib manually or automatically by add_subdirectory()
# Please adjust with TRUE for automatic or FALSE for manually
set(SPII_BUILD_MODE_VAR TRUE)

# Provide the user name for your system, which is used to find the Qt5_DIR on Mac and Win
set(QT_VERSION 5.15.2)

# Provide the user name for your system, which is used to find the Qt5_DIR on mac
# Windows users can ignore this line
set(USER_NAME_MAC papillon)

# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------
# ------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.15.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

# Output of initial values -----------------------------------------------
message("----- Output of initial values -----")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "--------------------------------------------------------" )
# ------------------------------------------------------------------------

# The VCPKG_TARGET_TRIPLET should be commented out if you already set it during the cmake call in the settings.json
if(APPLE)
	set(PYLON_HOME "/Library/Frameworks/pylon.framework")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/Library/Frameworks/pylon.framework/Headers")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/Library/Frameworks/pylon.framework/Headers/GenICam")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/Library/Frameworks/pylon.framework")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake) # The find pylon file is in the cmake folder

    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/Users/${USER_NAME_MAC}/Qt/5.15.2/clang_64")
    set(Qt5_DIR "/Users/${USER_NAME_MAC}/Qt/${QT_VERSION}/clang_64/lib/cmake/Qt5")

    set(VCPKG_TARGET_TRIPLET x64-osx CACHE STRING "")
elseif(MSVC OR WIN32)
	set(PYLON_HOME "C:/basler-pylon-6/Development")
    set(PYLON_INCLUDE_DIR "C:/basler-pylon-6/Development/include")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake) # The find pylon file is in the cmake folder
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/${QT_VERSION}/msvc2019_64")

    set(VCPKG_TARGET_TRIPLET x64-windows-static-md CACHE STRING "")
else()
	set(PYLON_HOME "/opt/pylon")
    set(PYLON_INCLUDE_DIR "/opt/pylon/include")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/opt/pylon/include/pylon")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/opt/pylon/include/GenApi")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/opt/pylon/include/Base")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake) # The find pylon file is in the cmake folder
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/opt/Qt/${QT_VERSION}/gcc_64")

    set(VCPKG_TARGET_TRIPLET x64-linux CACHE STRING "")
endif()
# ----------------------------------------------------------------------------------------------

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share")
# set(CMAKE_TOOLCHAIN_FILE "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share") # Normally this is set with the cmake call


# Setting the module paths to the <package-name>config.cmake files for each lib -------------------
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/opencv")
set(TBB_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/tbb")
set(Eigen3_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/eigen3")
set(Ceres_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/ceres")
set(glog_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/share/glog")
# TODO: It seems that boost has no config file in vcpkg. The dirs need to be set manually on mac
# ------------------------------------------------------------------------------------------------

# Using the find package commands ----------------------------------------------------------------
find_package(OpenCV CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Widgets Concurrent SerialPort Charts Svg PrintSupport Network Xml REQUIRED) # OpenGL
find_package(Pylon REQUIRED) # The "FindPylon.cmake" is in the project folder in cmake/

#find_package(Boost CONFIG REQUIRED)

# Check if everthing was found correctly for each used package
if(OPENCV_FOUND)
message(STATUS "----- OpenCV was found with the following status: -----")
message(STATUS "Version: ${OpenCV_VERSION}")
message(STATUS "Libraries: ${OpenCV_LIBS}")
message(STATUS "Include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Dir path: ${OpenCV_DIR}")
message(STATUS "Lib path: ${OpenCV_LIB_DIR}")
message(STATUS "--------------------------------------------------------" )
else()
message(STATUS "----- OpenCV packageconfig was not found ! -----")
endif()

if(TBB_FOUND)
message(STATUS "----- TBB was found -----")
else()
message(STATUS "----- TBB packageconfig was not found ! -----")
endif()

if(EIGEN3_FOUND)
message(STATUS "----- EIGEN3 was found -----")
else()
message(STATUS "----- EIGEN3 packageconfig was not found ! -----")
endif()

if(Ceres_FOUND)
message(STATUS "----- Ceres was found -----")
else()
message(STATUS "----- Ceres packageconfig was not found ! -----")
endif()

if(Boost_FOUND)
message(STATUS "----- Boost was found -----")
else()
message(STATUS "----- Boost packageconfig was not found ! -----")
endif()

if(${Qt5Core_FOUND})
message(STATUS "----- QTCore was found -----")
else()
message(STATUS "----- QTCore packageconfig was not found ! -----")
endif()

# -------------------------------------------------------
set(Boost_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/")
set(TBB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/")
set(EIGEN_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/")
set(CERES_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/build/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/")
# -------------------------------------------------------

# Adjustments for spii -------------------------------------------------------
set(spii_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/3rdparty/spii/include/")
set(spii_LIB_DIR lib)
set(SPII_INSTALL_DIR "3rdparty/spii/build")
set(spii_LIB_NAME 3rdparty/spii)

if(SPII_BUILD_MODE_VAR)
# Autimatic build for spii
    message(STATUS "--------------------- SPII automatic mode ---------------------")
    # Check install status of the library
    find_library(spii_LIBRARY_RELEASE
    spii
    PATHS ${CMAKE_BINARY_DIR}/lib/)

    find_library(meschach_LIBRARY_RELEASE
    meschach
    PATHS ${CMAKE_BINARY_DIR}/lib/)

        if(spii_LIBRARY_RELEASE AND meschach_LIBRARY_RELEASE)
            message(STATUS "--------------------- SPII found (automatic)---------------------")
            set(spii_LIBRARIES ${spii_LIBRARY_RELEASE} ${meschach_LIBRARY_RELEASE})
            message(STATUS "Libraries: ${spii_LIBRARIES}")
            set(SPII_BUILD_FOUND TRUE)
        else(spii_LIBRARY_RELEASE AND meschach_LIBRARY_RELEASE)
            message(STATUS "--------------------- SPII not found will be installed (automatic) ---------------------")
            add_subdirectory(${spii_LIB_NAME})
            link_directories(${CMAKE_BINARY_DIR}/lib)
            set(spii_LIBRARIES "")
        endif()
else()
# Manual build for spii
message(STATUS "--------------------- SPII manual mode ---------------------")
    find_library (spii_LIBRARY_RELEASE
            spii
            PATHS ${spii_LIB_NAME}/lib/)
    find_library (meschach_LIBRARY_RELEASE
            meschach
            PATHS ${spii_LIB_NAME}/lib/)
    if (spii_LIBRARY_RELEASE AND meschach_LIBRARY_RELEASE)
        set(spii_LIBRARIES ${spii_LIBRARY_RELEASE} ${meschach_LIBRARY_RELEASE})
        set(SPII_BUILD_FOUND TRUE)
        message(STATUS "--------------------- SPII was found ---------------------")
        message(STATUS "Libraries: ${spii_LIBRARIES}")
        message(STATUS "--------------------------------------------------------" )
    else()
        set(spii_LIBRARIES "")
        set(SPII_BUILD_FOUND TRUE)
        message(STATUS "-------------- WARNING SPII NOT FOUND: PLEASE INSTALL SPII LIBS ! ---------------------")
        message(STATUS "-------------- WARNING SPII NOT FOUND: PLEASE INSTALL SPII LIBS ! ---------------------")
        message(STATUS "-------------- WARNING SPII NOT FOUND: PLEASE INSTALL SPII LIBS ! ---------------------")
        message(STATUS "-------------- WARNING SPII NOT FOUND: PLEASE INSTALL SPII LIBS ! ---------------------")
        message(STATUS "-------------- WARNING SPII NOT FOUND: PLEASE INSTALL SPII LIBS ! ---------------------")
    endif()
endif()
# -------------------------------------------------------

# Output of initial values -----------------------------------------------
message("----- Output of ENDING values -----")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "--------------------------------------------------------" )
# ------------------------------------------------------------------------

include_directories(${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Concurrent_INCLUDE_DIRS}
        ${Qt5SerialPort_INCLUDE_DIRS}
        ${Qt5Charts_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
        ${Qt5PrintSupport_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
        ${TBB_INCLUDE_DIR}
        ${spii_INCLUDE_DIR}
        ${EIGEN_INCLUDE_DIR}
        ${CERES_INCLUDE_DIRS}
        ${PYLON_INCLUDE_DIR}
        "singleeyefitter")

add_subdirectory(src)

add_subdirectory(singleeyefitter)

# -------------------------------------------------------

message(STATUS "")
message(STATUS "--- Include directories ---" )
message(STATUS " QT5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}")
message(STATUS " Qt5Concurrent_INCLUDES: ${Qt5Concurrent_INCLUDE_DIRS}")
message(STATUS " Qt5SerialPort_INCLUDES: ${Qt5SerialPort_INCLUDE_DIRS}")
message(STATUS " Qt5Charts_INCLUDES: ${Qt5Charts_INCLUDE_DIRS}")
message(STATUS " Qt5Svg_INCLUDES: ${Qt5Svg_INCLUDE_DIRS}")
message(STATUS " Qt5PrintSupport_INCLUDES: ${Qt5PrintSupport_INCLUDE_DIRS}")
message(STATUS " Qt5Network_INCLUDES: ${Qt5Network_INCLUDE_DIRS}")
message(STATUS " Qt5Xml_INCLUDES: ${Qt5Xml_INCLUDE_DIRS}")

message(STATUS " OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS " Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIR}")
message(STATUS " tbb_INCLUDE_DIRS: ${TBB_INCLUDE_DIR}")
message(STATUS " spii_INCLUDE_DIRS: ${spii_INCLUDE_DIR}")
message(STATUS " EIGEN_INCLUDE_DIR: ${EIGEN_INCLUDE_DIR}")
message(STATUS " CERES_INCLUDE_DIRS: ${CERES_INCLUDE_DIRS}")
message(STATUS " Pylon_INCLUDE_DIRS: ${PYLON_INCLUDE_DIR}")
message(STATUS "---------------------------" )
message(STATUS "")

# Code snippet from https://stackoverflow.com/questions/43697154/detect-current-cmake-version-using-cmake
if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    message("Please consider to switch to CMake 3.21.0")
    message("CMAKE_VERSION: ${CMAKE_VERSION}")
endif()

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#
#SET(CMAKE_CXX_FLAGS "-g -O0")
#SET(CMAKE_C_FLAGS "-g -O0")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3 -Wall -pedantic")