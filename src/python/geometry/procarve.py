#!/usr/bin/python

##
# @file procarve.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the procarve program on a dataset
#
# @section DESCRIPTION
#
# Will call the procarve program on a dataset, verifying that all
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

# the following indicates the expected location of the procarve
# executable file
PROCARVE_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'procarve'))

# the following indicates the expected location of the settings xml
# file used for the procarve program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'procarve', \
		'procarve_settings.xml'))

##
# The main function of this script, which is used to run procarve
#
# This call will run the procarve program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
	
	# determine the expected location of necessary files from
	# within the dataset
	cmfile    = dataset_filepaths.get_carvemap_file(dataset_dir)
	wedgefile = dataset_filepaths.get_wedgefile(dataset_dir)
	chunklist = dataset_filepaths.get_chunklist(dataset_dir)
	octree    = dataset_filepaths.get_octree(dataset_dir)
	fpfiles   = dataset_filepaths.get_all_floorplan_files(dataset_dir)

	# prepare the command-line arguments for the procarve code
	args = [PROCARVE_EXE, '-m', cmfile, '-w', wedgefile, \
		'-l', chunklist, '-o', octree, \
		'-s', SETTINGS_XML] + fpfiles

	# run the procarve code
	ret = subprocess.call(args, executable=PROCARVE_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "procarve program returned error",ret
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
