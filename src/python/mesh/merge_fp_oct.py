#!/usr/bin/python

##
# @file merge_fp_oct.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the merge_fp_oct program on a dataset
#
# @section DESCRIPTION
#
# Will call the merge_fp_oct program on a dataset, verifying that all
# input files exist, and that the output directories are valid.
#

# Import standard python libraries
import os
import sys
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import dataset_filepaths

# the following indicates the expected location of the merge_fp_oct
# executable file
MERGE_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'merge_fp_oct'))

# the following indicates the expected location of the settings xml
# file used for the merge_fp_oct program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'fp_optimizer', \
		'fp_optimizer_settings.xml'))

#-----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS -----------------
#-----------------------------------------------------

##
# The main function of this script, which is used to run merge_fp_oct
#
# This call will run the merge_fp_oct program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
	
	# determine the expected location of necessary files from
	# within the dataset
	carvemapfile  = dataset_filepaths.get_carvemap_file(dataset_dir)
	wedgefile     = dataset_filepaths.get_wedgefile(dataset_dir)
	chunklistfile = dataset_filepaths.get_chunklist(dataset_dir)
	in_octfile    = dataset_filepaths.get_octree(dataset_dir)
	out_octfile   = dataset_filepaths.get_refined_octree(dataset_dir)
	fpfiles       = dataset_filepaths.get_aligned_fp_files(dataset_dir)
	if fpfiles is None or len(fpfiles) == 0:
		print "Error! Could not find any aligned floorplans to use!"
		return -1	

	# prepare the command-line arguments for the merge_fp_oct code
	args = [MERGE_EXE, '-s', SETTINGS_XML, '-m', carvemapfile, \
			'-w', wedgefile, '-l', chunklistfile, \
			'-i', in_octfile, '-o', out_octfile] + fpfiles

	# run the merge_fp_oct
	ret = subprocess.call(args, executable=MERGE_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "merge_fp_oct program returned error",ret
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
