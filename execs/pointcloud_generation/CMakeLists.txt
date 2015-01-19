#
# CMake file for the localization code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
cmake_minimum_required(VERSION 2.6)
project(pointcloud_gen)

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
include_directories(/usr/local/include/eigen3/)
include_directories(/usr/include/eigen3/)


# Find opencv
#
# This is used for both the kdtree and the threading code
find_package( OpenCV REQUIRED )
include_directories(${OpenCV_LIBS})

# Find liblas
#
# This is used for supporting outputing to .las formats
find_package(LibLAS REQUIRED)
#IF(LIBLAS_FOUND)
#	MESSAGE(STATUS "Compiling with .las file support.")
#	add_definitions(-DLIBLAS_SUPPORTED)
#	include_directories(${LIBLAS_INCLUDE_DIR})
#ELSE(LIBLAS_FOUND)
#	MESSAGE(STATUS "Compiling without .las file support.")
#ENDIF(LIBLAS_FOUND)

# Make the test sandbox executable
MESSAGE(STATUS "Including binary \"pointcloud_gen\"")
file(GLOB_RECURSE SANDBOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../execs/pointcloud_generation/main.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/tictoc.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/cmd_args.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/rotLib.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/progress_bar.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/util/range_list.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/config/*.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/xmlreader/*.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/data/urg/urg_data_reader.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/data/d_imager/d_imager_data_reader.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/data/fss/fss_io.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/data/color_image/color_image_metadata_reader.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/pointcloud/*.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/io/carve/noisypath_io.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/timestamp/*.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/image/*.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/geometry/system_path.cpp
							  ${CMAKE_CURRENT_SOURCE_DIR}/../../src/cpp/geometry/transform.cpp)
add_executable(pointcloud_gen ${SANDBOX_SRC})
target_link_libraries(pointcloud_gen ${OpenCV_LIBS})
IF(LIBLAS_FOUND)
	target_link_libraries(pointcloud_gen ${LIBLAS_LIBRARY})
ENDIF(LIBLAS_FOUND)