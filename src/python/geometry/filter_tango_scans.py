#!/usr/bin/python

##
# @file    filter_tango_scans.py
# @author  Eric Turner <elturner@eecs.berkeley.edu>
# @brief   Calls the filter_tango_scans program on a dataset
#
# @section DESCRIPTION
#
# Will call the filter_tango_scans program on a dataset, verifying that all
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
import backpackconfig
import dataset_filepaths

# the following indicates the expected location of the filter_tango_scans
# executable file
FILTER_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'bin', 'filter_tango_scans'))

# This constants defines the hardware name for the sensors
# to be filtered.
#
# This is the label used in the backpack hardware xml config file
TANGO_TYPE = 'tof_cameras'

# The xml-file-path to find the output data files for each
# tango scanner
TANGO_DAT_XPATH = 'configFile/tof_datafile'

##
# The main function of this script, which is used to run filter_tango_scans
#
# This call will run the filter_tango_scans program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
    
    # determine the expected location of necessary files from
    # within the dataset
    conffile  = dataset_filepaths.get_hardware_config_xml(dataset_dir)

    # read the xml configuration file
    conf = backpackconfig.Configuration(conffile, True, dataset_dir)
        
    # iterate through the available TANGO scanners
    tangoSensors = conf.find_sensors_by_type(TANGO_TYPE)
    for tango in tangoSensors :

        # get the output dat file for this sensor        
        datfile = os.path.abspath(os.path.join(dataset_dir, \
                conf.find_sensor_prop(tango, TANGO_DAT_XPATH, TANGO_TYPE)))

        # get the fss file that will be generated for this sensor
        fssfile = dataset_filepaths.get_fss_file(datfile)

        # prepare the command-line arguments for the filter_tango_scans code
	#
	# This code can also optionally export a .mad file, but we don't
	# need that here.
        args = [FILTER_EXE, datfile, fssfile]

        # run the filter_tango_scans code
        ret = subprocess.call(args, executable=FILTER_EXE, \
            cwd=dataset_dir, stdout=None, stderr=None, \
            stdin=None, shell=False)
        if ret != 0:
            print "filter_tango_scans program returned error",ret
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
