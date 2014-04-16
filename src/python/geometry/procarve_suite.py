#!/usr/bin/python

##
# @file procarve_suite.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls all the programs to generate a procarve
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

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local libraries
import wedge_gen
import chunker
import procarve
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import dataset_filepaths


##
# This function runs this script
#
# @param dataset_dir  The path to the dataset to process
# @param path_file    The path to the 3D path file to use
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

	# run the octsurf program to generate mesh
	OCTSURF_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
			'..', '..', '..', 'bin', 'octsurf'))
	ret = subprocess.call([OCTSURF_EXE], executable=OCTSURF_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "octsurf program returned error",ret
		return -4

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
