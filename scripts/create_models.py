#!/usr/bin/python

##
# @file create_models.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Overarching script to generate pointclouds and floorplans
#
# @section DESCRIPTION
#
# This script will generate all modeling products, such as pointclouds,
# floorplans, and 3D models.  This should be called after the localization
# is completed for a dataset.
#
# Usage:
#
#    python create_models.py <path_to_dataset> <localization_file>
#

# import python libraries
import os
import sys
import shutil
import subprocess

#-------------------------------------------#
#----- LOCATIONS OF OTHER SCRIPT FILES -----#
#-------------------------------------------#

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
import surface_carve
sys.path.append(PYTHON_LATEX_DIR)
import generate_tex

#------------------------------------#
#----- FUNCTION IMPLEMENTATIONS -----#
#------------------------------------#

##
# The main function
#
# This function is called when the script is executed from
# the command-line.
#
def main():

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
    DATASET_NAME = dataset_filepaths.get_name_from_madfile( \
		    LOCALIZATION_FILE)

    # process the geometry for this dataset
    print ""
    print "#### GENERATING MODELS FOR %s ####" % DATASET_NAME
    print ""
    ret = run(DATASET_DIR, LOCALIZATION_FILE, DATASET_NAME)
    if ret != 0:
        print "[create_models] Error %d occurred" % ret
        sys.exit(4)

    # success, close gracefully
    sys.exit(0)

##
# This function runs the script, given appropriate arguments
#
# If calling this script from another python script, call this
# function with the appropriate arguments.  If calling this
# script from the command-line, then the main() function will
# be called, which in turn calls this function after parsing
# arguments.
#
# @param DATASET_DIR        The dataset directory to use
# @param LOCALIZATION_FILE  The .mad file to use
# @param DATASET_NAME       The name of this dataset
#
# @return   Returns zero on success, non-zero on failure.
#
def run(DATASET_DIR, LOCALIZATION_FILE, DATASET_NAME): 

    # COLORED POINTCLOUD GENERATION
    xyzfiles = pointcloud_gen.run(DATASET_DIR, LOCALIZATION_FILE, True)
    if xyzfiles is None:
        print "[create_models] Error! Color pointcloud gen script failed"
        return -1

    # POINTCLOUD GENERATION (has more points than colored pc's)
    xyzfiles = pointcloud_gen.run(DATASET_DIR, LOCALIZATION_FILE, False)
    if xyzfiles is None:
        print "[create_models] Error! Pointcloud generation script failed"
        return -2

    # POINTCLOUD PARTITIONING
    ret = partition_pointcloud_levels.run(DATASET_DIR, \
                    LOCALIZATION_FILE, xyzfiles)
    if ret != 0:
        print "[create_models] Error! Pointcloud partitioning " \
                + "script returned",ret
        return -3

    # iterate through the different levels of the building
    for level_xyz_file in dataset_filepaths.get_pc_levels_list(DATASET_DIR):

        # generate floorplan for this level
        ret = floorplan.run(DATASET_DIR, LOCALIZATION_FILE, level_xyz_file)
        if ret != 0:
            print "[create_models]  Error! Could not generate " \
                    "floorplans:",ret
            return -4

    # generate a detailed mesh using surface carving method.
    # this is saved for last since it takes the longest
    ret = surface_carve.run(DATASET_DIR, LOCALIZATION_FILE)
    if ret != 0:
        print "Error! Unable to generate surface carving"
        return -5
    
    # prepare some documentation about this dataset
    ret = generate_tex.run(DATASET_DIR, DATASET_NAME)
    if ret != 0:
        print "Error! Unable to generate PDF documentation",ret
        return -6

    # success
    return 0

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
    main()
