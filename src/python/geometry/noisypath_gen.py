#!/usr/bin/python

##
# @file noisypath_gen.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the noisypath_gen program on a dataset
#
# @section DESCRIPTION
#
# Will call the noisypath_gen program on a dataset, verifying that all
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

# the following indicates the expected location of the noisypath_gen
# executable file
NOISYPATH_GEN_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'generate_noisypath'))

##
# The main function of this script, which is used to run noisypath_gen
#
# This call will run the noisypath_gen program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
# @param madfile      The mad file to convert
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, madfile):

        # get output file location
        noisypathfile = dataset_filepaths.get_noisypath_file(dataset_dir)

	# prepare the command-line arguments for the noisypath gen code
	args = [NOISYPATH_GEN_EXE, '--degrees', '--rot_sigma', '2.0', \
	                           '--lin_sigma', '0.05', \
	                           os.path.abspath(madfile), noisypathfile]

	# run the noisypath_gen code
	ret = subprocess.call(args, executable=NOISYPATH_GEN_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "noisypath_gen program returned error",ret
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
	if len(sys.argv) != 3:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset> <madfile>"
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
