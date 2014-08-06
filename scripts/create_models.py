#!/usr/bin/python

##
# @file create_models.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
#
# @section DESCRIPTION
#
# This script will generate all modeling products, such as pointclouds,
# floorplans, and 3D models.  This should be called after the localization
# is completed for a dataset.
#
# Usage:
#
#	python create_models.py <path_to_dataset> <localization_file>
#

import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
PYTHON_SRC_DIR = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
                                 '..', 'src','python'))
PYTHON_FILES_DIR    = os.path.join(PYTHON_SRC_DIR, 'files')
PYTHON_CONFIG_DIR   = os.path.join(PYTHON_SRC_DIR, 'config')
PYTHON_GEOMETRY_DIR = os.path.join(PYTHON_SRC_DIR, 'geometry')
PYTHON_LATEX_DIR    = os.path.join(PYTHON_SRC_DIR, 'latex')
sys.path.append(PYTHON_SRC_DIR)
sys.path.append(PYTHON_FILES_DIR)
import dataset_filepaths
sys.path.append(PYTHON_CONFIG_DIR)
import config
sys.path.append(PYTHON_GEOMETRY_DIR)
import pointcloud_gen
import partition_pointcloud_levels
import floorplan
sys.path.append(PYTHON_LATEX_DIR)
import generate_tex

###################################
##### PARSE COMMAND ARGUMENTS #####
###################################

# check that we were given a command-line argument
if len(sys.argv) != 3:
	print ""
	print " Usage:"
	print ""
	print "\t",sys.argv[0],"<path_to_dataset>","<localization_file>"
	print ""
	sys.exit(1)

# Get the dataset directory from command-line
DATASET_DIR = os.path.abspath(sys.argv[1])

# verify this is a valid folder
if not os.path.exists(DATASET_DIR):
	print "Error!  This is not a valid directory:",DATASET_DIR
	sys.exit(2)

# Get the output file from localization
LOCALIZATION_FILE = os.path.abspath(sys.argv[2])
if not os.path.exists(LOCALIZATION_FILE):
	print "Error! This is not a valid file:",LOCALIZATION_FILE
	sys.exit(3)

# get the human-friendly name of this dataset from the input file
DATASET_NAME = dataset_filepaths.get_file_body(LOCALIZATION_FILE)

####################
##### RUN CODE #####
####################

print ""
print "#### GENERATING MODELS FOR %s ####" % DATASET_NAME
print ""

# POINTCLOUD GENERATION
xyzfiles = pointcloud_gen.run(DATASET_DIR, LOCALIZATION_FILE, False)
if xyzfiles is None:
	print "Error! Pointcloud generation script failed"
	sys.exit(4)

# POINTCLOUD PARTITIONING
ret = partition_pointcloud_levels.run(DATASET_DIR, \
				LOCALIZATION_FILE, xyzfiles)
if ret != 0:
	print "Error! Pointcloud partitioning script returned",ret
	sys.exit(5)

# iterate through the different levels of the building
for level_xyz_file in dataset_filepaths.get_pc_levels_list(DATASET_DIR): 

	# generate floorplan for this level
	ret = floorplan.run(DATASET_DIR, LOCALIZATION_FILE, level_xyz_file)
	if ret != 0:
		print "Error! Could not generate floorplans:",ret
		sys.exit(6)

# prepare some documentation about this dataset
ret = generate_tex.run(DATASET_DIR, DATASET_NAME)
if ret != 0:
	print "Error! Unable to generate PDF documentation",ret
	sys.exit(7)
