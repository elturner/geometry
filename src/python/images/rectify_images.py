#!/usr/bin/python

##
# @file     rectify_images.py
# @author   Nick Corso, edited by Eric Turner <elturner@eecs.berkeley.edu>
# @brief    This script calls the rectify_images executable, to generate
#           rectified images from the original fisheye imagery.
#
# @section DATE
#
#   Passed from Nicholas Corso to Eric Turner on April 19, 2015
#   Modified by Eric Turner started on April 20, 2015
#

### IMPORTS AND PATH ADDITIONS
import argparse
import os
import sys
import re

#---------------------------------------------------#
#------------------- CONSTANTS ---------------------#
#---------------------------------------------------#

# Append the source dir for python modules 
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local libraries
sys.path.append(os.path.join(SCRIPT_LOCATION), '..', 'files')
sys.path.append(os.path.join(SCRIPT_LOCATION), '..', 'config')
import dataset_filepaths
import backpackconfig

### Path to constant files to use
RECTIFY_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'bin', 'rectify_images'))

# TODO put config stuff in its own .xml config file

### CONSTANTS!!!
# CHANGING THE FOCAL LENGTH HERE WILL ALMOST CERTAINLY REQUIRE YOU TO 
# CREATE NEW MASK FILES!  IT IS YOUR RESPONSIBILITY TO DO THIS
FUP = 4
FDOWN = 4
FLEVEL = 3

# The output rectified image size, in pixels
# Changing this is okay.  Should be no problems
IMAGESIZE = [2048, 2448]

#---------------------------------------------------#
#------------------- CODE BODY ---------------------#
#---------------------------------------------------#

##
# The run function
# 
# This function will run the image rectification code on the specified
# dataset.
#
def run(datasetDir):

    # Read in the hardware config file
    configFile = dataset_filepaths.get_hardware_config_xml(datasetDir)
    conf = backpackconfig.Configuration(configFile, True, datasetDir)

    # Find all the active cameras in the file
    cameraList = conf.find_sensors_by_type("cameras")
    if len(cameraList) == 0 :
        print "No active cameras found in dataset " + datasetDir
        return 0;

    # get location of masks for any cameras
    MASK_DIR = dataset_filepaths.get_camera_masks_dir(datasetDir)

    # Then we loop over the cameraList generating the required things
    for cameraName in cameraList :

        print "==== RECTIFYING : " + cameraName + " ===="

        # TODO left off here

        # now we need to read find out what the input names should be 
        calibFile = conf.find_sensor_prop(cameraName, 
            "configFile/dalsa_fisheye_calibration_file", "cameras")
        calibFile = os.path.join(datasetDir, calibFile)
        metaDataFile = conf.find_sensor_prop(cameraName, 
            "configFile/dalsa_metadata_outfile", "cameras")
        metaDataFile = dataset_filepaths.get_color_metadata_file( \
            cameraName, metaDataFile)
        rToCommon = conf.find_sensor_prop(cameraName,
            "rToCommon", "cameras").split(",")
        tToCommon = conf.find_sensor_prop(cameraName,
            "tToCommon", "cameras").split(",")
        extrinStr =  " ".join((rToCommon + tToCommon))
        outputDirectory = conf.find_sensor_prop(cameraName, 
            "configFile/dalsa_output_directory", "cameras")
        outputDirectory = os.path.abspath(os.path.join(datasetDir, outputDirectory, '..', 'rectified'))

        # Adjust the maskdir
        maskDir = os.path.join(MASK_DIR, cameraName)

        # VODO The camera serial number
        camSerial = conf.find_sensor_prop(cameraName, 
            "serialNum", "cameras")
        camSerial = re.sub("[^0-9]", "", camSerial)

        ### UP CAMERA
        print "---Up Images"

        # Make vcam specific inputs
        outDir = os.path.join(outputDirectory, 'up')
        K = " ".join([str(max(IMAGESIZE)/FUP), "0", str(IMAGESIZE[1]/2), 
                      "0", str(max(IMAGESIZE)/FUP), str(IMAGESIZE[0]/2), 
                      "0","0","1"])
        rotation = "90 0 0"
        maskfile = os.path.join(maskDir, "up.jpg")
        serial = camSerial + "0"

        # Run the command
        command = [BINFILE,
            "-ic", calibFile,
            "-id", datasetDir,
            "-iv", maskfile,
            "-ie", extrinStr,
            "-ik", K,
            "-im",metaDataFile,
            "-ir",rotation,
            "-is",str(IMAGESIZE[0]) + " " + str(IMAGESIZE[1]),
            "-od",outDir,
            "-os",serial]
        command = " ".join(command)
        ret = os.system(command)
        if ret != 0 :
            print "Error Processing " + cameraName + " up images"
            return 2;

        ### LEVEL CAMERA
        print "---Level Images"

        # Make vcam specific inputs
        outDir = os.path.join(outputDirectory, 'level')
        K = " ".join([str(max(IMAGESIZE)/FLEVEL), "0", str(IMAGESIZE[1]/2), 
                      "0", str(max(IMAGESIZE)/FLEVEL), str(IMAGESIZE[0]/2), 
                      "0","0","1"])
        rotation = "0 0 0"
        maskfile = os.path.join(maskDir, "level.jpg")
        serial = camSerial + "1"

        # Run the command
        command = [BINFILE,
            "-ic", calibFile,
            "-id", datasetDir,
            "-iv", maskfile,
            "-ie", extrinStr,
            "-ik", K,
            "-im",metaDataFile,
            "-ir",rotation,
            "-is",str(IMAGESIZE[0]) + " " + str(IMAGESIZE[1]),
            "-od",outDir,
            "-os",serial]
        command = " ".join(command)
        ret = os.system(command)
        if ret != 0 :
            print "Error Processing " + cameraName + " level images"
            return 2;

        ### DOWN CAMERA
        print "---Down Images"

        # Make vcam specific inputs
        outDir = os.path.join(outputDirectory, 'down')
        K = " ".join([str(max(IMAGESIZE)/FDOWN), "0", str(IMAGESIZE[1]/2), 
                      "0", str(max(IMAGESIZE)/FDOWN), str(IMAGESIZE[0]/2), 
                      "0","0","1"])
        rotation = "-90 0 0"
        maskfile = os.path.join(maskDir, "down.jpg")
        serial = camSerial + "2"

        # Run the command
        command = [BINFILE,
            "-ic", calibFile,
            "-id", datasetDir,
            "-iv", maskfile,
            "-ie", extrinStr,
            "-ik", K,
            "-im",metaDataFile,
            "-ir",rotation,
            "-is",str(IMAGESIZE[0]) + " " + str(IMAGESIZE[1]),
            "-od",outDir,
            "-os",serial]
        command = " ".join(command)
        ret = os.system(command)
        if ret != 0 :
            print "Error Processing " + cameraName + " down images"
            return 2;

    return 0;

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():
    
    # check the command-line arguments
    if len(sys.argv) != 2:
        print ""
        print " Usage:"
        print ""
        print "\t", sys.argv[0],"<path_to_dataset>"
        print ""
        sys.exit(1)

    # run this script with the given dataset
    ret = run(os.path.abspath(sys.argv[1]))
    sys.exit(ret)

##
#
# Boilerplate code to run the main function
#
if __name__ == '__main__':
  main()
