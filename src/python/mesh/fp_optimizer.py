#!/usr/bin/python

##
# @file fp_optimizer.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the fp_optimizer program on a dataset
#
# @section DESCRIPTION
#
# Will call the fp_optimizer program on a dataset, verifying that all
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

# the following indicates the expected location of the fp_optimizer
# executable file
FP_OPT_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'fp_optimizer'))

# the following indicates the expected location of the settings xml
# file used for the fp_optimizer program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'fp_optimizer', \
		'fp_optimizer_settings.xml'))

#-----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS -----------------
#-----------------------------------------------------

##
# The main function of this script, which is used to run fp_optimizer
#
# This call will run the fp_optimizer program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
	
	# determine the expected location of necessary files from
	# within the dataset
	octfile = dataset_filepaths.get_octree(dataset_dir)
	in_fpfiles = dataset_filepaths.get_all_floorplan_files(dataset_dir)
	if in_fpfiles is None or len(in_fpfiles) == 0:
		print "Error! Could not find any floorplan files to use!"
		return -1

	# prepare output directory
	aligned_fp_dir = dataset_filepaths.get_aligned_fp_dir(dataset_dir)
	if not os.path.exists(aligned_fp_dir):
		os.makedirs(aligned_fp_dir)

	# determine output file for each input floorplan
	out_fpfiles = []
	for f_in in in_fpfiles:
		# file will be given same name in the aligned directory
		(head, tail) = os.path.split(f_in)
		f_out = os.path.join(aligned_fp_dir, tail)
		out_fpfiles.append(f_out)

	# prepare the command-line arguments for the fp_optimizer code
	args = [FP_OPT_EXE, '-s', SETTINGS_XML, '-o', octfile]
	for i in range(len(in_fpfiles)):
		args += ['-f', in_fpfiles[i], out_fpfiles[i]]

	# run the fp_optimizer
	ret = subprocess.call(args, executable=FP_OPT_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "fp_optimizer program returned error",ret
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
