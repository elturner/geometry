#
# CMake file for the image rectification code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
#	This project is called localization
cmake_minimum_required(VERSION 2.6)
project(simplify_fp)

# Set the build type
set(CMAKE_BUILD_TYPE Release)

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

# Make the executable
file(GLOB EXE_SRC 	${GEOMETRY_ROOT}/execs/simplify_fp/main.cpp
					${GEOMETRY_ROOT}/execs/simplify_fp/Polygon.cpp
					${GEOMETRY_ROOT}/execs/simplify_fp/Point2D.cpp
					${GEOMETRY_ROOT}/src/cpp/util/cmd_args.cpp
					${GEOMETRY_ROOT}/src/cpp/util/tictoc.cpp)
add_executable(simplify_fp ${EXE_SRC})

# Link libraries
TARGET_LINK_LIBRARIES( simplify_fp ${Boost_LIBRARIES} )






