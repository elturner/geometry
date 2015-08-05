#!/usr/bin/python

##
# @file   make_ceiling_screenshots.py
# @author Eric Turner <elturner@indoorreality.com>
# @brief  Generates an image for each level's ceiling, from pointcloud data
# 
# @section DESCRIPTION
#
# Will crop the pointcloud to generate a slice at just the ceiling for
# each level.  These ceiling sub-pointclouds will be saved along with
# a .png image of each.
#
# Created August 2015
#

# import python libraries
import os
import sys
import shutil
import subprocess

#-------------------------------------
#---------- CONSTANTS ----------------
#-------------------------------------

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local code
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import dataset_filepaths
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from levels_io import Levels

# store path to executables
FILTER_PC_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
	"..", "..", "bin", "filter_pointcloud"))
if sys.platform == 'win32' :
	FILTER_PC_EXE += '.exe'
SCREENSHOT_PC_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
		"..", "..", "bin", "screenshot_pointcloud"))
if sys.platform == 'win32' :
	SCREENSHOT_PC_EXE += '.exe'
SPLIT_IMG_BY_FP_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
	"..", "..", "bin", "split_image_by_floorplan"))
if sys.platform == 'win32' :
	SPLIT_IMG_BY_FP_EXE += '.exe'

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will generate ceiling pointcloud screenshots
#
# This will run the code, but will first check
# that all files exist and the output can be written.  
#
# @param dataset_dir   Path to dataset directory
#
def run(dataset_dir):

	# ensure abspath for input files
	dataset_dir = os.path.abspath(dataset_dir)

	# ----- File Checks -------

	# check that executables exists
	if not os.path.exists(FILTER_PC_EXE):
		print "Error!  Could not find exectuable:", \
			FILTER_PC_EXE
		return -1
	if not os.path.exists(SCREENSHOT_PC_EXE):
		print "Error!  Could not find exectuable:", \
			SCREENSHOT_PC_EXE
		return -2
	if not os.path.exists(SPLIT_IMG_BY_FP_EXE):
		print "Error!  Could not find exectuable:", \
			SPLIT_IMG_BY_FP_EXE
		return -3

	# get necessary input pointcloud files for this dataset
	pcdir = dataset_filepaths.get_colored_pc_dir(dataset_dir)
	pcfiles = []
	for f in os.listdir(pcdir):
		(body, ext) = os.path.splitext(f)
		if ext == '.xyz':
			pcfiles.append(os.path.join(pcdir, f))
	if len(pcfiles) == 0:
		print "Error!  No colored pointclouds generated!"
		return -4

	# get necessary geometry input files
	levelsfile = dataset_filepaths.get_carving_levels_file(dataset_dir)
	fpfiles    = dataset_filepaths.get_carving_fp_files(dataset_dir)

	# parse the levels file
	levels = Levels()
	ret = levels.read(levelsfile)
	if ret != 0:
		print "Error!  Unable to parse building levels file"
		return -5

	# ensure the correct number of floorplans have been found
	if len(fpfiles) != levels.num_levels:
		print "Error!  Expected number of levels:",levels.num_levels
		print "        # of found floorplans:    ",len(fpfiles)
		return -6

	# ----- Processing -------

	# iterate over the levels of this model.  We want to generate
	# a ceiling pointcloud screenshot for each one
	for level_index in range(levels.num_levels):
	
		# make the level-wide screenshot
		ret = make_ceiling_screenshot_for_level( \
				dataset_dir, pcfiles, \
				levels, level_index, fpfiles[level_index])
		if ret != 0:
			print "Error!  Unable to generate ceiling " \
				+ "pointcloud screenshots for level #", \
					level_index
			return -7;

	# success
	return 0

##
# Creates ceiling pointcloud screenshot for the given level
#
# @param dataset_dir  The root dataset directory
# @param pcfiles      The pointcloud files to use
# @param levels       The levels structure for this model
# @param level_index  Which level to process
# @param fpfile       The floorplan file for this level
#
# @return    Returns zero on success, non-zero on failure.
#
def make_ceiling_screenshot_for_level( \
		dataset_dir, pcfiles, levels, level_index, fpfile):
	
	# prepare output paths
	ceil_pc = dataset_filepaths.get_light_detection_level_ceiling_pc( \
			dataset_dir, level_index)
	ceil_image = \
	   dataset_filepaths.get_light_detection_level_ceiling_image( \
			dataset_dir, level_index)
	ceil_coordmap = \
	   dataset_filepaths.get_light_detection_level_ceiling_coordmap( \
			dataset_dir, level_index)
	ceil_timemap = \
	   dataset_filepaths.get_light_detection_level_ceiling_timemap( \
			dataset_dir, level_index)
	ceil_rooms = \
	   dataset_filepaths.get_light_detection_level_ceiling_rooms( \
	   		dataset_dir, level_index)
	
	# verify that output locations exist
	if not os.path.exists(ceil_rooms):
		os.makedirs(ceil_rooms)

	# generate a filtering script to retrieve just the ceiling of
	# the pointcloud for the specified level
	#
	# This script will only keep the cross section from the midpoint
	# of the current level elevation to the boundary of the next level
	crop_minz = (0.5 * (levels.floor_heights[level_index] \
				+ levels.ceiling_heights[level_index]))
        crop_minz *= 1000 # convert from meters to millimeters
	filterscript = "PartitionPlane 0 0 1 0 0 %f; Kill INVALID; " \
					% crop_minz
	if level_index < (levels.num_levels-1):
		# since this is not the top level, we also need
		# to crop everything above it
		crop_maxz = levels.split_heights[level_index]
                crop_maxz *= 1000 # convert from meters to millimeters
		filterscript += ("PartitionPlane 0 0 -1 0 0 %f; " \
				+ "Kill INVALID; ") % crop_maxz
	
	# we want to mirror the ceiling pointcloud so the underside
	# is facing up.  This way the lights will show up in a top-down
	# screenshot
	filterscript += "Scale 1 1 -1"

	# next, create a new pointcloud file for just the ceiling under
	# the above set of filtering operations
	args = [ FILTER_PC_EXE, \
			"-o", ceil_pc, \
			"-x", filterscript]
	for f in pcfiles:
		args += ["-i", f]
	ret = call_exe(FILTER_PC_EXE, args, dataset_dir)
	if ret != 0:
		# an error occurred
		print "Error! Could not create ceiling pointcloud"
		return -1

	# now that the pointcloud exists, we can take a screenshot
	# of it
	args = [ SCREENSHOT_PC_EXE, \
			"-i", ceil_pc, \
			"-o", ceil_image, ceil_coordmap, ceil_timemap, \
			"-u", "0.001", \
			"-r", "0.01", \
			"-b", "0", "0", "0", \
			"--ignore_uncolored" ]
	ret = call_exe(SCREENSHOT_PC_EXE, args, dataset_dir)
	if ret != 0:
		# an error occurred
		print "Error!  Could not produce screenshot of ceiling"
		return -2

	# now that we have a screenshot for the entire level, also
	# make images for each room individually
	args = [ SPLIT_IMG_BY_FP_EXE, \
			"-ii", ceil_image, \
			"-ic", ceil_coordmap, \
			"-it", ceil_timemap, \
			"-if", fpfile, \
			"-o",  ceil_rooms ]
	ret = call_exe(SPLIT_IMG_BY_FP_EXE, args, dataset_dir)
	if ret != 0:
		# an error occurred
		print "Error!  Could not produce room-specific ceilings"
		return -3

	return 0

##
# Runs the specified process with the given arguments
#
# @param exe    The exectutable to run, should already be listed in args
# @param args   The arguments to give
# @param rundir The directory to run in
# @param debug  Whether or not to run in 'debug' mode
#
# @return       Returns the return code of this subprocess
#
def call_exe(exe, args, rundir, debug=False):
	
	# check for debug
	exe_to_run = exe
	if debug:
		exe_to_run = 'lldb'
		args = [exe_to_run, '--'] + args
	ret = subprocess.call(args, executable=exe_to_run, \
			cwd=rundir, stdout=None, stderr=None, \
			stdin=None, shell=False)
	if ret != 0:
		print "Error! %s call failed with code %d" \
				% (exe, ret)
	return ret

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 2:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1])
	
	# display results
	print ""
	if ret != 0:
		sys.exit(1)

	# success
	sys.exit(0)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
