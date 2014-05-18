#!/usr/bin/python

##
# @file wedge_gen.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the wedge_gen program on a dataset
#
# @section DESCRIPTION
#
# Will call the wedge_gen program on a dataset, verifying that all
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

# the following indicates the expected location of the scan_wedge_gen
# executable file
WEDGE_GEN_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'wedge_gen'))

# the following indicates the expected location of the settings xml
# file used for the scan_wedge_gen program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'procarve', \
		'procarve_settings.xml'))

##
# The main function of this script, which is used to run wedge_gen
#
# This call will run the wedge_gen program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
# @param path_file    The path to the 3D path file to use
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, path_file):
	
	# determine the expected location of necessary files from
	# within the dataset
	config_xml = dataset_filepaths.get_hardware_config_xml(dataset_dir)
	timesync_xml = dataset_filepaths.get_timesync_xml(dataset_dir)
	cmfile = dataset_filepaths.get_carvemap_file(dataset_dir)
	wedgefile = dataset_filepaths.get_wedgefile(dataset_dir)
	fssfiles = dataset_filepaths.get_all_fss_files(dataset_dir)

	# verify input is good
	if fssfiles is None:
		print "Error! Unable to determine fss files to use"
		return -1

	# prepare the command-line arguments for the wedge_gen code
	args = [WEDGE_GEN_EXE, '-c', config_xml, '-m', cmfile, \
		'-w', wedgefile, '-p', os.path.abspath(path_file), \
		'-s', SETTINGS_XML, '-t', timesync_xml] + fssfiles

	# run the wedge_gen code
	ret = subprocess.call(args, executable=WEDGE_GEN_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "wedge_gen program returned error",ret
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
