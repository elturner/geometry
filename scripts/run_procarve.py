#!/usr/bin/python

##
# @file run_procarve.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls all programs to generate detailed surface reconstruction
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
import argparse

# Get the location of this file and other source files
SCRIPT_LOCATION = os.path.dirname(__file__)
PYTHON_SRC_LOC = os.path.normpath(os.path.join(SCRIPT_LOCATION, \
                '..', 'src', 'python'))

# this variable specifies the number of stages in this script
NUM_STAGES = 9

# import local libraries
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'geometry'))
import filter_urg_scans
import noisypath_gen
import wedge_gen
import chunker
import procarve
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'mesh'))
import oct2fp
import fp_optimizer
import merge_fp_oct
import octsurf
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'files'))
import dataset_filepaths

#-----------------------------------------------------------------
#------------ RUNS SCRIPTS FOR INDIVIDUAL PROGRAMS ---------------
#-----------------------------------------------------------------

##
# This function runs this script
#
# @param dataset_dir  The path to the dataset to process
# @param madfile      The path to the mad file to use
# @param stages       The stages of this script to run.
#                     Stages are indexed from 1, and go to NUM_STAGES
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, madfile, stages):

    # verify that carving directory exists, which will contain all
    # intermediary and output files for this code
    carvedir = dataset_filepaths.get_carving_dir(dataset_dir)
    if not os.path.exists(carvedir):
        os.makedirs(carvedir)

    # filter the raw range scans to include statistical information
    # about the scanner intrinsics
    if 1 in stages:
        print ""
        print "STAGE 1 - GENERATING SCAN STATISTICS"
        print ""
        ret = filter_urg_scans.run(dataset_dir)
        if ret != 0:
            return -1; # an error occurred
    
    # convert the mad file into a noisypath file
    if 2 in stages:
        print ""
        print "STAGE 2 - GENERATING PATH STATISTICS"
        print ""
        ret = noisypath_gen.run(dataset_dir, madfile)
        if ret != 0:
            return -2 # an error occurred

    # run the wedge generation program on input scans
    if 3 in stages:
        print ""
        print "STAGE 3 - GENERATING WEDGES"
        print ""
        path_file = dataset_filepaths.get_noisypath_file(dataset_dir)
        ret = wedge_gen.run(dataset_dir, path_file)
        if ret != 0:
            return -3; # an error occurred

    # run the chunker program on the resulting wedge file
    if 4 in stages:
        print ""
        print "STAGE 4 - RUNNING CHUNKER"
        print ""
        ret = chunker.run(dataset_dir)
        if ret != 0:
            return -4 # an error occurred

    # run the procarve program on the output chunks
    if 5 in stages:
        print ""
        print "STAGE 5 - CARVING"
        print ""
        ret = procarve.run(dataset_dir)
        if ret != 0:
            return -5 # an error occurred

    # generate wall samples and floorplan from carving
    if 6 in stages:
        print ""
        print "STAGE 6 - GENERATING FLOORPLAN"
        print ""
        ret = oct2fp.run(dataset_dir, madfile);
        if ret != 0:
            return -6 # an error occurred
        
    # run the fp optimization program on resulting octree
    if 7 in stages:
        print ""
        print "STAGE 7 - ALIGNING FLOORPLAN TO CARVING"
        print ""
        ret = fp_optimizer.run(dataset_dir)
        if ret != 0:
            return -7 # an error occurred

    # merge the floorplans and carving into one file
    if 8 in stages:
        print ""
        print "STAGE 8 - MERGING CARVING AND FLOORPLAN"
        print ""
        ret = merge_fp_oct.run(dataset_dir)
        if ret != 0:
            return -8 # an error ocurred

    # run the octsurf program to generate mesh
    if 9 in stages:
        print ""
        print "STAGE 9 - GENERATING SURFACE"
        print ""
        ret = octsurf.run(dataset_dir)
        if ret != 0:
            return -9 # an error occurred

    # success
    return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

    # check command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("dataset_dir", metavar="DataDir", type=str, \
            help="Relative path to dataset directory")
    parser.add_argument("madfile", metavar="MadFile", type=str, \
            help="Relative path to 3D .mad localization file")
    parser.add_argument("-b","--begin_stage", required=False, nargs=1, \
            default=([1]), type=int, \
            help="Start processing at this stage ([1," \
            + str(NUM_STAGES) + "] inclusive)")
    parser.add_argument("-e","--end_stage", required=False, nargs=1, \
            default=([NUM_STAGES]), type=int, \
            help="End processing at this stage ([1," \
            + str(NUM_STAGES) + "] inclusive)")
    args = parser.parse_args()

    # retrieve arguments
    dataset_dir = args.dataset_dir[0]
    madfile     = args.madfile[0]
    begin_stage = max(1, min(NUM_STAGES, args.begin_stage[0]))
    end_stage   = min(NUM_STAGES, max(1, args.end_stage[0]))
    stages = range(begin_stage, 1+end_stage)

    # run this script with the given arguments
    ret = run(sys.argv[1], sys.argv[2], stages)
    sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
    main()
