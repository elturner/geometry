#
# CMake file for the xyz2dq code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
cmake_minimum_required(VERSION 2.6)
project(xyz2dq)

# Set the build type
# Set to RELEASE for release mode and DEBUG for debug mode
IF(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "Defaulting to building targets in RELEASE mode")
	set(CMAKE_BUILD_TYPE RELEASE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Setup where we want to build the output binaries
#
# This tells CMAKE where to instruct the compiler to build the
# output executables
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/)

# Add the source directories to the search path
# 
# This makes it so that our compiler can find all the header files
# in the src/cpp directory and the z_estimation includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

# Make the test sandbox executable
MESSAGE(STATUS "Including binary \"xyz2dq\"")
file(GLOB_RECURSE XYZ2DQ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(xyz2dq ${XYZ2DQ_SRC})