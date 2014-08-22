#!/usr/bin/python

##
# @file surface_carve.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the surface_carve program on a dataset
#
# @section DESCRIPTION
#
# Will call the surface_carve program on a dataset, verifying that all
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

# the following indicates the expected location of the surface_carve
# executable file
SURFACE_CARVE_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'bin', 'surface_carve'))

# the following represents the location of the hardware configuration
# file to use for the surface carve code.  Note that this code was
# developed before we standardized to xml config files, so it needs
# its own config file.
CONFIG_FILE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
                '..', '..', '..', 'config', 'surface_carve', \
                'magnetoTransform_20140511_CAD.bcfg'))

#-----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS -----------------
#-----------------------------------------------------

##
# The main function of this script, which is used to run surface_carve
#
# This call will run the surface_carve program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
# @param madfile      The localization file to use
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, madfile):
    
    # determine the expected location of necessary files from
    # within the dataset
    voxfile = dataset_filepaths.get_surface_carve_vox(dataset_dir, madfile)
    plyfile = dataset_filepaths.get_surface_carve_ply(dataset_dir, madfile)

    # prepare output directory
    sc_dir = dataset_filepaths.get_surface_carve_dir(dataset_dir)
    if not os.path.exists(sc_dir):
        os.makedirs(sc_dir)

    # prepare the command-line arguments for the surface_carve code
    args = [SURFACE_CARVE_EXE, '-r', '0.05', '-p', '-f', '-m', '7', '-a', \
            CONFIG_FILE, voxfile, plyfile, madfile] \
            + dataset_filepaths.get_all_pointcloud_files(dataset_dir)

    # run the surface_carve
    ret = subprocess.call(args, executable=SURFACE_CARVE_EXE, \
        cwd=dataset_dir, stdout=None, stderr=None, \
        stdin=None, shell=False)
    if ret != 0:
        print "surface_carve program returned error",ret
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
    if len(sys.argv) != 3:
        print ""
        print " Usage:"
        print ""
        print "\t",sys.argv[0],"<path_to_dataset>","<madfile>"
        print ""
        sys.exit(1)

    # run this script with the given arguments
    ret = run(sys.argv[1],sys.argv[2])
    sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
    main()
