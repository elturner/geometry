#!/usr/bin/python

##
# @file run_procarve.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls all programs to generate detailed surface reconstruction
#
# @section DESCRIPTION
#
# This metascript will call the python scripts in charge
# of the individual programs in the procarve pipeline suite
#

# import libraries
import os
import sys
import subprocess

# Get the location of this file and other source files
SCRIPT_LOCATION = os.path.dirname(__file__)
PYTHON_SRC_LOC = os.path.normpath(os.path.join(SCRIPT_LOCATION, \
				'..', 'src', 'python'))

# import local libraries
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'geometry'))
import wedge_gen
import chunker
import procarve
import octsurf
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'mesh'))
import fp_optimizer
import merge_fp_oct
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'files'))
import dataset_filepaths

#-----------------------------------------------------------------
#------------ RUNS SCRIPTS FOR INDIVIDUAL PROGRAMS ---------------
#-----------------------------------------------------------------

##
# This function runs this script
#
# @param dataset_dir  The path to the dataset to process
# @param path_file    The path to the noisypath file to use
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, path_file):

	# verify that carving directory exists, which will contain all
	# intermediary and output files for this code
	carvedir = dataset_filepaths.get_carving_dir(dataset_dir)
	if not os.path.exists(carvedir):
		os.makedirs(carvedir)
	
	# run the wedge generation program on input scans
	ret = wedge_gen.run(dataset_dir, path_file)
	if ret != 0:
		return -1; # an error occurred

	# run the chunker program on the resulting wedge file
	ret = chunker.run(dataset_dir)
	if ret != 0:
		return -2 # an error occurred

	# run the procarve program on the output chunks
	ret = procarve.run(dataset_dir)
	if ret != 0:
		return -3 # an error occurred

	# run the fp optimization program on resulting octree
	ret = fp_optimizer.run(dataset_dir)
	if ret != 0:
		return -4 # an error occurred

	# merge the floorplans and carving into one file
	ret = merge_fp_oct.run(dataset_dir)
	if ret != 0:
		return -5 # an error ocurred

	# run the octsurf program to generate mesh
	ret = octsurf.run(dataset_dir)
	if ret != 0:
		return -6 # an error occurred

	# success
	return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 3:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>",\
			"<localization_file>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1], sys.argv[2])
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
