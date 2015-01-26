#
# CMake file for the localization code
#

# Set up meta information about this cmake build.
#	We require at least version 2.6 of CMake
#	This project is called localization
cmake_minimum_required(VERSION 2.6)
project(Filter_URG_Scans)

# Set the build type
set(CMAKE_BUILD_TYPE Release)

# Set the directory to the root
set(MAGNETO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Setup where we want to build the output binaries
set(EXECUTABLE_OUTPUT_PATH ${MAGNETO_ROOT}/bin/)

# Add the source directories to the search path
include_directories(${MAGNETO_ROOT}/src/cpp/)

# Collect a list of all of the .cpp files in the project
file(GLOB COMMON_SRC ${MAGNETO_ROOT}/src/cpp/util/tictoc.cpp
	${MAGNETO_ROOT}/src/cpp/util/progress_bar.cpp
	${MAGNETO_ROOT}/src/cpp/util/cmd_args.cpp
	${MAGNETO_ROOT}/src/cpp/io/data/urg/urg_data_reader.cpp
	${MAGNETO_ROOT}/src/cpp/io/data/fss/fss_io.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/tinystr.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/tinyxml.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/tinyxmlerror.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/tinyxmlparser.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/tinyxmltools.cpp
	${MAGNETO_ROOT}/src/cpp/xmlreader/xmlsettings.cpp
	${MAGNETO_ROOT}/src/cpp/timestamp/sync_xml.cpp)

# Make the executable
file(GLOB LOCAL_SRC ${MAGNETO_ROOT}/execs/filter_urg_scans/*.cpp)
add_executable(filter_urg_scans ${LOCAL_SRC} ${COMMON_SRC})

