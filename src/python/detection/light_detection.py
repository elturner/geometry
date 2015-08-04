#!/usr/bin/python

##
# @file   light_detection.py
# @author Eric Turner <elturner@indoorreality.com>
# @brief  The light detection python script
# 
# @section DESCRIPTION
#
# This script will call the programs in the light detection
# pipeline after verifying that all input files
# exist, output directories exist, and the given
# dataset is formatted appropriately.
#
# The method employed in this code is the same as developed by
# Satarupa Mukherjee in the UC Berkeley VIP Lab in 2014.
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
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import backpackconfig
import dataset_filepaths

import make_ceiling_screenshots

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will perform light detection for a dataset
#
# This will run the code, but will first check
# that all files exist and the output can be written.  
#
# @param dataset_dir   Path to dataset directory
#
def run(dataset_dir):

	# ensure abspath for input files
	dataset_dir = os.path.abspath(dataset_dir)

	#-------- Generate Screenshots ----------
	ret = make_ceiling_screenshots.run(dataset_dir)
	if ret != 0:
		print "Error!  Unable to make pointcloud ceiling images"
		return -1

	# TODO LEFT OFF HERE

	# success
	return 0

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
