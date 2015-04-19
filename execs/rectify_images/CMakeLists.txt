#
# CMake file for the image rectification code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
#	This project is called localization
cmake_minimum_required(VERSION 2.6)
project(rectify_images)

# Set the build type
set(CMAKE_BUILD_TYPE Release)

# Set compiler flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fpermissive")
endif()

# Set the directory to the root
set(GEOMETRY_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Setup where we want to build the output binaries
set(EXECUTABLE_OUTPUT_PATH ${GEOMETRY_ROOT}/bin/)

# Add the source directories to the search path
include_directories(${GEOMETRY_ROOT}/src/cpp/)
include_directories(${GEOMETRY_ROOT}/src/cpp/include)

# Find boost
FIND_PACKAGE( Boost COMPONENTS system filesystem thread REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# Find opencv
find_package( OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )

# Enable TICTOC to not suck for multithreading if you have C++11 
#MESSAGE(STATUS "COMPILING WITH C++11 CHRONO support...")
#MESSAGE(STATUS "	If this breaks it... comment next line in the CMakeLists.txt")
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -D _COMPILE_WITH_CHRONO)

# Make the executable
file(GLOB EXE_SRC 	${GEOMETRY_ROOT}/execs/rectify_images/src/main.cpp
					${GEOMETRY_ROOT}/execs/rectify_images/src/rectify_images.cpp
					${GEOMETRY_ROOT}/src/cpp/util/cmd_args.cpp
					${GEOMETRY_ROOT}/src/cpp/util/tictoc.cpp
					${GEOMETRY_ROOT}/src/cpp/util/progress_bar.cpp
					${GEOMETRY_ROOT}/src/cpp/io/data/color_image/color_image_metadata_reader.cpp
					${GEOMETRY_ROOT}/src/cpp/image/fisheye/ocam_functions.cpp)
add_executable(rectify_images ${EXE_SRC})

# Link libraries
TARGET_LINK_LIBRARIES( rectify_images ${Boost_LIBRARIES} )
TARGET_LINK_LIBRARIES( rectify_images ${OpenCV_LIBS} )

