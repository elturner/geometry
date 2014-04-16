#!/usr/bin/python

##
# @file octsurf.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the octsurf program on a dataset
#
# @section DESCRIPTION
#
# Will call the octsurf program on a dataset, verifying that all
# input files exist, and that the output directories are valid.
#

# Import standard python libraries
import os
import sys
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import dataset_filepaths

# the following indicates the expected location of the octsurf
# executable file
OCTSURF_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'octsurf'))

# the following indicates the expected location of the settings xml
# file used for the octsurf program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'procarve', \
		'procarve_settings.xml'))

##
# The main function of this script, which is used to run octsurf
#
# This call will run the octsurf program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
	
	# determine the expected location of necessary files from
	# within the dataset
	outfile   = dataset_filepaths.get_carved_obj_file(dataset_dir)
	octree    = dataset_filepaths.get_octree(dataset_dir)

	# prepare the command-line arguments for the chunker code
	args = [OCTSURF_EXE, '-o', outfile, '-s', SETTINGS_XML] + [octree]

	# run the octsurf code
	ret = subprocess.call(args, executable=OCTSURF_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "octsurf program returned error",ret
		return -1
	
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
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
