#!/usr/bin/python

##
# @file   octsurf.py
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
    
    # make sure the mesh directory exists
    meshdir    = dataset_filepaths.get_mesh_dir(dataset_dir)
    if not os.path.exists(meshdir):
        os.makedirs(meshdir)

    # determine the expected location of necessary files from
    # within the dataset
    octree     = dataset_filepaths.get_refined_octree(dataset_dir)
    objectmesh = dataset_filepaths.get_object_mesh_file(dataset_dir)
    roommesh   = dataset_filepaths.get_room_mesh_file(dataset_dir)
    fullmesh   = dataset_filepaths.get_full_mesh_file(dataset_dir)

    # run octsurf code to generate mesh of objects
    args = [OCTSURF_EXE, '-o', objectmesh, '--objects', '--dense', \
            '-s', SETTINGS_XML, octree]
    ret = callproc(OCTSURF_EXE, args, dataset_dir, False)
    if ret != 0:
        print "octsurf program returned error %d on object meshing" % ret
        return -1
    
    # run octsurf code to generate mesh of room
    args = [OCTSURF_EXE, '-o', roommesh, '--room', '--planar', \
            '-s', SETTINGS_XML, octree]
    ret = callproc(OCTSURF_EXE, args, dataset_dir, False)
    if ret != 0:
        print "octsurf program returned error %d on room meshing" % ret
        return -2 

    # generate full mesh
    args = [OCTSURF_EXE, '-o', fullmesh, '-s', SETTINGS_XML, octree]
    ret = callproc(OCTSURF_EXE, args, dataset_dir, False)
    if ret != 0:
        print "octsurf program returned error %d on full meshing" % ret
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
