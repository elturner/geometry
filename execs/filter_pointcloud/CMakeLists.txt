#
# CMake file for the localization code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
cmake_minimum_required(VERSION 2.6)
project(filter_pointcloud)

# Set the build type
# Set to RELEASE for release mode and DEBUG for debug mode
IF(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "Defaulting to building targets in RELEASE mode")
	set(CMAKE_BUILD_TYPE RELEASE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Enable C++11 features
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Set where to look for additional FindXXXX.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

# Setup where we want to build the output binaries
#
# This tells CMAKE where to instruct the compiler to build the
# output executables
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/)

# Add the source directories to the search path
# 
# This makes it so that our compiler can find all the header files
# in the src/cpp directory and the z_estimation includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/)

# Make the test sandbox executable
MESSAGE(STATUS "Including binary \"filter_pointcloud\"")
file(GLOB_RECURSE SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../execs/filter_pointcloud/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../execs/filter_pointcloud/Filter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/tictoc.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/cmd_args.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/conf/conf_reader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/writer/PointCloudWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/writer/OBJWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/writer/PTSWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/writer/XYZWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/reader/PointCloudReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/reader/OBJReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/reader/PTSReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/reader/XYZReader.cpp)

# Find liblas
#
# This is used for supporting outputing to .las formats
IF(LAS_SUPPORT)
	find_package(LibLAS REQUIRED)
	IF(NOT LIBLAS_FOUND)
		message( FATAL_ERROR "Unable to find LibLAS library" )
	ENDIF(NOT LIBLAS_FOUND)
	add_definitions( -DWITH_LAS_SUPPORT )
	LIST(APPEND SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/writer/LASWriter.cpp)
	LIST(APPEND SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/reader/LASReader.cpp)
ENDIF(LAS_SUPPORT)

add_executable(filter_pointcloud ${SANDBOX_SRC})

IF(LAS_SUPPORT)
	target_link_libraries(filter_pointcloud ${LIBLAS_LIBRARY})
ENDIF(LAS_SUPPORT)
