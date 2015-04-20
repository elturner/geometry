#
# Driver functions to run the image generation code
#
import backpackconfig
import sys
from os import *
import glob

#
# depth and normal images run function
#
def run(settings) :

	# The first thing we need to do is import the backpack configuration file
	# so that we can deduce the locations of all the files names
	confFile = path.join(settings['datasetDir'],
		"config","backpack_config.xml")
	conf = backpackconfig.Configuration(confFile, True, settings['datasetDir']);

	# Then we need to get a list of all active cameras for this dataset
	cameras = conf.find_sensors_by_type("cameras")

	# For each camera create the triplet of inputs
	code_args = []
	for camera in cameras :

		# Create the input mcd and output directory location
		datadir = conf.find_sensor_prop(camera, 
			"configFile/dalsa_output_directory", 
			"cameras")	
		imageDir = path.normpath(path.join(settings['datasetDir'],
			datadir,'..'))

		# Then find the mcd file for this camera
		mcdFiles = glob.glob(path.join(imageDir,'rectified/level/*.mcd'))
		if(len(mcdFiles) == 0) :
			print "No .mcd files found in " + path.join(imageDir,'rectified/level')
			return 1;
		if(len(mcdFiles) > 1) :
			print (str(len(mcdFiles)) + " .mcd files found in " +
				path.join(imageDir,'rectified/level'))
			return 2;

		# Store the final mcd file
		mcdFile = mcdFiles[0]

		# Then locate the pose file for the camera
		poseFiles = glob.glob(path.join(settings['datasetDir'],
			'localization',settings['datasetName'],
			'cameraposes',camera+"_poses.txt"))
		if(len(poseFiles) == 0) :
			print ("No camera pose file for " + camera + " found in " 
				 + path.join(settings['datasetDir'],
					'localization',settings['datasetName'],
					'cameraposes'))
			return 1;

		# Store the pose file
		poseFile = poseFiles[0]

		# Create the output directory name
		outDir = path.join(settings['datasetDir'],
			'imagemaps',camera)

		# Store as a tuple
		code_args.append(tuple([mcdFile, poseFile, outDir, camera]))

	# Creat the binary file name
	binFile = path.join(settings['binaryDir'],'depth_maps')

	# Create the arguments for the C++ code
	args = [binFile,
		"-dir", settings['datasetDir'],
		"-model", settings['modelFile'],
		"-depth", str(settings['depth'])]
	for argset in code_args :
		args += ["-i"]
		args += argset[:]
	if settings['thread'] != None :
		args += ['-threads',str(settings['thread'])]
	if settings['downsample'] != None :
		args += ['-ds',str(settings['downsample'])]

	# Run the code
	ret = system(" ".join(args))
	if ret != 0:
		print "Depth mapping returned error : " + str(ret)

	# Return success
	return ret

