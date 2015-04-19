#!/usr/bin/python

### IMPORTS AND PATH ADDITIONS
import argparse
import os
import sys
import re

# Append the source dir for python modules 
SCRIPT_LOCATION = os.path.dirname(__file__)
PYTHON_SRC_DIR = os.path.abspath(
		os.path.join(SCRIPT_LOCATION, '..', 'src', 'python'))
MASK_DIR = os.path.abspath(os.path.join(SCRIPT_LOCATION, '..', 'calib', 'camera', 'masks'))
sys.path.append(PYTHON_SRC_DIR)
 
import backpackconfig

### CONSTANTS!!!
# CHANGING THE FOCAL LENGTH HERE WILL ALMOST CERTAINLY REQUIRE YOU TO 
# CREATE NEW MASK FILES!  IT IS YOUR RESPONSIBILITY TO DO THIS
FUP = 4
FDOWN = 4
FLEVEL = 3

# Changing this is okay.  Should be no problems
IMAGESIZE = [2048, 2448]


### FUNCTIONS
#
# main function
#
def main() :

	# The first thing we need to do is append the python sorce so this
	# can be called from anywhere
	SCRIPT_LOCATION = os.path.dirname(__file__)
	
	BIN_DIR = os.path.join(SCRIPT_LOCATION, '..', 'bin')
	BINFILE = os.path.join(BIN_DIR, 'rectify_images')

	# Copy the dataset directory
	if len(sys.argv) < 2 :
		print "First argument must be the dataset directory"
		return 1;
	datasetDir = sys.argv[1]

	# Read in the config file
	configFile = os.path.join(datasetDir, "config", "backpack_config.xml")
	conf = backpackconfig.Configuration(configFile, True, datasetDir)

	# Find all the active cameras in the file
	cameraList = conf.find_sensors_by_type("cameras")
	if len(cameraList) == 0 :
		print "No active cameras found in dataset " + datasetDir
		return 0;

	# Then we loop over the cameraList generating the required things
	for cameraName in cameraList :

		print "==== RECTIFYING : " + cameraName + " ===="

		# now we need to read find out what the input names should be 
		calibFile = conf.find_sensor_prop(cameraName, 
			"configFile/dalsa_fisheye_calibration_file", "cameras")
		calibFile = os.path.join(datasetDir, calibFile)
		metaDataFile = conf.find_sensor_prop(cameraName, 
			"configFile/dalsa_metadata_outfile", "cameras")
		metaDataFile = os.path.join(datasetDir,
			os.path.dirname(metaDataFile),
			"color_" + cameraName + "_metadata.txt")
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

# 
# Boilerplate code to run the main function
#
if __name__ == '__main__':
  main()
