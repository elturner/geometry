#!/usr/bin/python

##
# @file partition_pointcloud_levels.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Prepares arguments and runs partition_pointcloud_levels program
# 
# @section DESCRIPTION
#
# This script will call the pointcloud partitioning
# executable after verifying that all input files
# exist, output directories exist, and the given
# dataset is formatted appropriately.
#

# import python libraries
import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local code
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import dataset_filepaths

# store path to executable
PART_PC_LEVELS_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
		"..", "..", "bin",  "partition_pointcloud_levels"))
if sys.platform == 'win32' :
	PART_PC_LEVELS_EXE += '.exe'

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will partition the pointclouds for a given dataset
#
# This will run the pointcloud partitioning code, checking
# that all files exist and the output can be written.
#
# @param dataset_dir   Path to dataset directory
# @param madfile       Path to the localization output file
# @param xyzfiles      List of xyz files to use as input
#
# @return    On success, returns zero
#            On failure, returns non-zero.
#
def run(dataset_dir, madfile, xyzfiles):

	# check that executable exists
	if not os.path.exists(PART_PC_LEVELS_EXE):
		print "Error!  Could not find pointcloud", \
			"generation executable:", \
			PART_PC_LEVELS_EXE
		return -1

	# get input files for this dataset
	name = dataset_filepaths.get_file_body(madfile)
	xyz_levels = dataset_filepaths.get_pc_levels_prefix(dataset_dir, \
			name)
	xyz_hist = dataset_filepaths.get_pc_levels_hist(dataset_dir, name)

	# verify output directory exists
	pc_levels_dir = dataset_filepaths.get_pc_levels_dir(dataset_dir)
	if not os.path.exists(pc_levels_dir):
		os.makedirs(pc_levels_dir)

	# prepare arguments
	args = [PART_PC_LEVELS_EXE, '-o', xyz_levels, madfile, \
		xyz_hist] + xyzfiles

	# run the code
	ret = subprocess.call(args, executable=PART_PC_LEVELS_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, stdin=None, \
		shell=False)
	if ret != 0:
		print "Error! Pointcloud partitioning program returned",ret
		return -2

	# success 
	return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) < 2:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset> <madfile>", \
			"<xyzfile_1> <xyzfile_2> ..."
		print ""
		sys.exit(1)

	# run this script with the given arguments
	dataset_dir = sys.argv[1]
	madfile = sys.argv[2]
	xyzfiles = sys.argv[3:]
	ret = run(dataset_dir, madfile, xyzfiles)

	# success
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
