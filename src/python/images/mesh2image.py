#!/usr/bin/python

##
# @file     mesh2image.py
# @author   Nick Corso, edited by Eric Turner <elturner@eecs.berkeley.edu>
# @brief    This script calls the mesh2image executable, to generate
#           depth maps and normal maps for rectified images
#
# @section DATE
#
#   Passed from Nicholas Corso to Eric Turner on April 19, 2015
#   Modified by Eric Turner started on June 3, 2015
#

# imports
import backpackconfig
import dataset_filepaths
import sys
from os import *
import glob

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# store path to executable
MESH2IMAGE_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
        "..", "..", "bin",  "mesh2image"))
if sys.platform == 'win32' :
    MESH2IMAGE_EXE += '.exe'

#---------------------------------------------------#
#------------------- CODE BODY ---------------------#
#---------------------------------------------------#

##
# Depth and normal images run function
#
# Given a dataset, will generate depth and normal maps for each 'level'
# rectified image for each available camera.
#
def run(datasetDir, datasetName):

    # The first thing we need to do is import the backpack i
    # configuration file
    # so that we can deduce the locations of all the files names
    configFile = dataset_filepaths.get_hardware_config_xml(datasetDir)
    conf = backpackconfig.Configuration(configFile, True, datasetDir)
    meshfile = dataset_filepaths.get_full_mesh_file(datasetDir)

    # Then we need to get a list of all active cameras for this dataset
    cameras = conf.find_sensors_by_type("cameras")
    if len(cameraList) == 0 :
        print "No active cameras found in dataset " + datasetDir
        return 0;

    # For each camera create the triplet of inputs
    code_args = []
    for camera in cameras :

        # get the folder for this camera in the dataset
        datadir = conf.find_sensor_prop(camera, 
            "configFile/dalsa_output_directory", 
            "cameras")    
        imageDir = path.normpath(path.join(datasetDir,
            datadir,'..'))

        # Then find the mcd file for this camera
        mcdFiles = glob.glob(path.join(imageDir, \
            'rectified', 'level', '*.mcd'))
        if(len(mcdFiles) == 0) :
            print "No .mcd files found in " \
                + path.join(imageDir,'rectified', 'level')
            return 1
        if(len(mcdFiles) > 1) :
            print (str(len(mcdFiles)) + " .mcd files found in " +
                path.join(imageDir,'rectified','level'))
            return 2

        # Store the final mcd file
        mcdFile = mcdFiles[0]

        # Then locate the pose file for the camera
        poseFiles = glob.glob(path.join(datasetDir,
            'localization',datasetName,
            'cameraposes',camera+"_poses.txt"))
        if(len(poseFiles) == 0) :
            print ("No camera pose file for " + camera + " found in " 
                 + path.join(settings['datasetDir'],
                    'localization',datasetName,
                    'cameraposes'))
            return 1

        # Store the pose file
        poseFile = poseFiles[0]
        
        # Create the output directory name
        outDir = path.join(datasetDir,
            'imagemaps',camera)
        
        # Store as a tuple
        code_args.append(tuple([mcdFile, poseFile, outDir, camera]))
        
    # Create the arguments for the C++ code
    args = [MESH2IMAGE_EXE,
        "-dir",   datasetDir,
        "-model", meshfile,
        "-depth", '12']
    for argset in code_args :
        args += ["-i"]
        args += argset[:]

    # Run the code
    ret = system(" ".join(args))
    if ret != 0:
        print "Depth mapping returned error : " + str(ret)

    # Return success
    return ret

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():
    
    # check the command-line arguments
    if len(sys.argv) != 3:
        print ""
        print " Usage:"
        print ""
        print "\t", sys.argv[0],"<path_to_dataset>","<dataset_name>"
        print ""
        sys.exit(1)

    # run this script with the given dataset
    ret = run(os.path.abspath(sys.argv[1]), sys.argv[2])
    sys.exit(ret)

##
#
# Boilerplate code to run the main function
#
if __name__ == '__main__':
  main()
