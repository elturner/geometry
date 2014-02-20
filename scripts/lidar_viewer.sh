#!/bin/bash

##
# @file lidar_viewer.sh
# @author Eric Turner <elturner@eecs.berkeley.edu>
#
# @section DESCRIPTION
# 
# This script will preprocess and show pointclouds using 
# Vrui's lidar viewer
#

#############################################################

# program paths
PREPROCESSOR_PROG="/home/elturner/Documents/research/programs/LidarViewer-2.12/bin/LidarPreprocessor"
VIEWER_PROG="/home/elturner/Documents/research/programs/LidarViewer-2.12/bin/LidarViewer"
ILLUMINATOR_PROG="/home/elturner/Documents/research/programs/LidarViewer-2.12/bin/LidarIlluminator"

#############################################################

# check arguments
if [ $# -ne 1 ]
then
	echo " Usage:"
	echo ""
	echo "     $0 <xyz_file>"
	exit
fi

# get input xyz file
xyz_file=$1

# get processed octree path
oct_file="${xyz_file}.oct"

# check if this file has already been preprocessed
if [ -e ${oct_file} ]
then
	# run the viewer
	echo "viewing existing pointcloud..."
	echo ${oct_file}
	${VIEWER_PROG} ${oct_file}
	exit
fi

# call the preprocessor function
echo "processing pointcloud..."
${PREPROCESSOR_PROG} -np 1024 -ooc 1000 -o ${oct_file} -ASCIIRGB 0 1 2 3 4 5 ${xyz_file}

# run the viewer
echo "viewing pointcloud..."
${VIEWER_PROG} ${oct_file}
