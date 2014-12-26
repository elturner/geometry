#!/usr/bin/python

##
# @file pointcloud_gen.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief The pointcloud generation python script
# 
# @section DESCRIPTION
#
# This script will call the pointcloud generation
# executable after verifying that all input files
# exist, output directories exist, and the given
# dataset is formatted appropriately.
#

# import python libraries
import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local code
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import dataset_filepaths

# store path to executable
POINTCLOUD_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
		"..", "..", "bin",  "pointcloud_gen"))
if sys.platform == 'win32' :
	POINTCLOUD_EXE += '.exe'

# the following dictionary contains the white-list of all laser sensors
# to use for pointcloud generation
laser_whitelist = ['H1214157', 'H1311822', 'H1004314', 'H1004315',
	'H1316223', 'H1316225']
camera_whitelist = ['left_camera', 'right_camera', 'back_camera']

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will generate a pointcloud for the given dataset
#
# This will run the pointcloud generation code, but will first check
# that all files exist and the output can be written.  It will also
# check what sensors are available in the dataset, and only use
# existing sensor data.
#
# @param dataset_dir   Path to dataset directory
# @param pathfile      Path to 3D .mad path file
# @param use_cameras   If true, will color with camera imagery
#
# @return    On success, returns path to output xyz files.
#            On failure, returns None.
#
def run(dataset_dir, pathfile, use_cameras=True):

	# check that executable exists
	if not os.path.exists(POINTCLOUD_EXE):
		print "Error!  Could not find pointcloud", \
			"generation executable:", \
			POINTCLOUD_EXE
		return None

	# get configuration files for this dataset
	timesync_xml = dataset_filepaths.get_timesync_xml(dataset_dir)
	config_xml = dataset_filepaths.get_hardware_config_xml(dataset_dir)

	# parse the hardware configuration file
	sensor_types, sensor_cfn, sensor_names = \
			config.parse_backpack_xml(config_xml)
	if sensor_types is None or sensor_cfn is None \
			or len(sensor_types) != len(sensor_cfn):
		return None # could not parse the xml file

	# find the laser and camera files to use as input
	geomfiles = []
	geomnames = []
	cam_metas  = []
	cam_calibs = []
	cam_dirs   = []
	for si in range(len(sensor_types)):	
		
		# check for laser scanners
		if sensor_types[si] == 'laser':

			# check if this sensor is in the whitelist
			if sensor_names[si] not in laser_whitelist:
				continue # ignore this one

			# parse the settings file for this laser
			urg_settings = config.parse_settings_xml( \
					os.path.normpath(os.path.join( \
					dataset_dir, sensor_cfn[si])))
			if urg_settings is None:
				print "Error! Could not parse laser", \
					sensor_cfn[si]
				return None # unable to parse settings

			# get the laser data file (relative to dataset)
			urg_filename = urg_settings["urg_datafile"]

			# add to our list
			geomfiles.append(urg_filename)
			geomnames.append(sensor_names[si])

		# check for cameras
		if sensor_types[si] == 'camera':

			# check if this camear is in the whitelist
			if sensor_names[si] not in camera_whitelist:
				continue # ignore this one

			# parse the settings file for camera
			cam_settings = config.parse_settings_xml( \
					os.path.normpath(os.path.join( \
					dataset_dir, sensor_cfn[si])))
			if cam_settings is None:
				print "Error! Could not parse cam", \
					sensor_cfn[si]
				return None # unable to parse settings

			# get the color metadata file (relative to dataset)
			cam_metas.append( \
				dataset_filepaths.get_color_metadata_file( \
				sensor_names[si], \
				cam_settings["dalsa_metadata_outfile"]))

			# get calibration file (relative to dataset)
			cam_calibs.append(cam_settings[ \
				"dalsa_fisheye_calibration_file"])
	
			# get image directory for this camera (relative)
			cam_dirs.append( \
				dataset_filepaths.get_color_image_dir( \
				cam_settings["dalsa_output_directory"]))


	# prepare output directory for pointclouds
	name = dataset_filepaths.get_file_body(pathfile)
	if use_cameras:
		pc_output_dir = dataset_filepaths.get_colored_pc_dir( \
				dataset_dir)
	else:
		pc_output_dir = dataset_filepaths.get_pointcloud_dir( \
				dataset_dir)
	if not os.path.exists(pc_output_dir):
		os.makedirs(pc_output_dir)

	# choose appropriate parameters based on input
        if use_cameras:
            range_limit = '10'
        else:
            range_limit = '30'

	# now that we have a list of geometry sensors to use, we
	# want to make pointcloud files for each sensor
	output_files = []
	for si in range(len(geomfiles)):
		
		# specify output file for this scanner
		xyzfile = os.path.join(pc_output_dir, name + '_' \
			+ geomnames[si] + '.xyz')

		# Prepare arguments for pointcloud generation program.
		# This will generate pointclouds in units of millimeters 
		# and remove points farther than 10 meters from the
		# sensor
		args = [POINTCLOUD_EXE, '-c', config_xml, \
				'-t', timesync_xml, \
				'-o', xyzfile, \
				'-p', os.path.abspath(pathfile), \
				'-u', '1000', '-r', range_limit, \
				'-l', geomnames[si], geomfiles[si]]

		# add camera information if we want to color
		if use_cameras:
			args += ['--remove_noncolored_points', \
				'--time_buffer', '2.0', '0.5']
			for ci in range(len(cam_metas)):
				args += ['-f', cam_metas[ci], \
					cam_calibs[ci], cam_dirs[ci]]

		# run pointcloud generation program
                ret = subprocess.call(args, executable=POINTCLOUD_EXE, \
			cwd=dataset_dir, stdout=None, stderr=None, \
			stdin=None, shell=False)
		if ret != 0:
			print "Error! Pointcloud generation program", \
				"returned",ret
			return None

		# record where we wrote the output
		output_files.append(xyzfile)
	
	# return the final list of files
	return output_files

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 3 and len(sys.argv) != 4:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>", \
			"<madfile>","[-c]"
		print ""
		print " Where:"
		print ""
		print "\t-c\tIf specified, will generate color pointcloud."
		print "\t  \tMust be last argument if specified."
		print ""
		sys.exit(1)

	# get values
	dataset_dir = sys.argv[1]
	path_file = sys.argv[2]
	use_color = False
	if len(sys.argv) == 4 and sys.argv[3] == '-c':
		use_color = True

	# run this script with the given arguments
	ret = run(sys.argv[1],sys.argv[2],use_color)
	
	# display results
	print ""
	if ret is None:
		print "Error occurred during pointcloud generation!"
		sys.exit(2)
	elif len(ret) == 0:
		print "No pointclouds exported."
	elif len(ret) == 1:
		print "One pointcloud exported."
	else:
		print "%d pointclouds exported." % len(ret)
	
	# success
	sys.exit(0)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
