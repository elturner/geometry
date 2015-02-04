#!/usr/bin/python

##
# @file oct2fp.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the oct2dq and floorplan_gen programs on a dataset
#
# @section DESCRIPTION
#
# Will call the oct2dq and floorplan_gen programs on a dataset, 
# verifying that all input files exist, and that the output 
# directories are valid.
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

# the following indicates the expected location of the
# executable files
OCT2DQ_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'bin', 'oct2dq'))
FLOORPLAN_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'bin', 'floorplan_gen'))

# the following indicates the expected location of the settings xml
# file used for the oct2dq program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'config', 'oct2dq', \
        'oct2dq_settings.xml'))

##
# The main function of this script, which is used to run oct2fp
#
# This call will run the oct2dq and floorplan_gen programs, 
# verifying inputs and outputs.
#
# @param dataset_dir  The path to the dataset to process
# @param madfile      The path to the .mad file of the dataset
# @param debug        If true, will run code through gdb
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, madfile, debug=False):

    # check that directories exist
    output_dir = dataset_filepaths.get_carving_fp_dir(dataset_dir)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # determine the expected location of necessary files from
    # within the dataset
    config_xml = dataset_filepaths.get_hardware_config_xml(dataset_dir)
    fssfiles   = dataset_filepaths.get_all_fss_files(dataset_dir)
    pathfile   = dataset_filepaths.get_noisypath_file(dataset_dir)
    octfile    = dataset_filepaths.get_octree(dataset_dir)
    levelsfile = dataset_filepaths.get_carving_levels_file(dataset_dir)
    dqfile     = dataset_filepaths.get_carving_dq_file(dataset_dir)
    fpfile     = dataset_filepaths.get_carving_fp_file(dataset_dir)

    # verify input is good
    if fssfiles is None:
        print "Error! Unable to determine fss files to use"
        return -1

    # prepare the command-line arguments for the oct2dq code and run it
    args = [OCT2DQ_EXE, '-c', config_xml, '-s', SETTINGS_XML, \
        pathfile, octfile, levelsfile, dqfile] + fssfiles
    ret = callproc(OCT2DQ_EXE, args, dataset_dir, debug)
    if ret != 0:
        print "oct2dq program returned error",ret
        return -2
    
    # run the floorplan generation code
    args = [FLOORPLAN_EXE, dqfile, os.path.abspath(madfile), config_xml, \
            fpfile, '-s', '-1']
    ret = callproc(FLOORPLAN_EXE, args, dataset_dir, debug)
    if ret != 0:
        print "floorplan_gen program returned error",ret
	return -3

    # success
    return 0

##
# Calls a subprocess with the given set of arguments
#
# @param exe    The executable to run
# @param args   The arguments to give to the executable
# @param cwd    The directory to run in
# @param debug  Whether to run the process in gdb or not
#
# @return    Returns return-code of subprocess
#
def callproc(exe, args, cwd, debug):

    # check if we're debugging
    if debug:
        exe = 'gdb'
        args = ['gdb', '--args'] + args

    # call the process
    ret = subprocess.call(args, executable=exe, \
        cwd=cwd, stdout=None, stderr=None, stdin=None, shell=False)
    return ret


##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

    # check if we are debugging
    debug = (len(sys.argv) == 4 and sys.argv[3] == '--debug')

    # check command-line arguments
    if not debug and len(sys.argv) != 3:
        print ""
        print " Usage:"
        print ""
        print "\t",sys.argv[0],"<path_to_dataset> <madfile> [--debug]"
        print ""
        sys.exit(1)

    # run this script with the given arguments
    ret = run(sys.argv[1], sys.argv[2], debug)
    sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
    main()
