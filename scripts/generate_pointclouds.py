#!/usr/bin/python
#
#	generate_pointclouds.py
#
#	This script handles the generation of pointclouds only for the quicksilver
#	data system
#

#	
# 	IMPORTS
#

# system imports
import sys
import os
import re
import string
import argparse
import subprocess

# Import our python files
SCRIPT_LOCATION = os.path.dirname(__file__)
PYTHON_SRC_DIR = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
                                 '..', 'src','python'))
PYTHON_CONFIG_DIR   = os.path.join(PYTHON_SRC_DIR, 'config')
PYTHON_FILES_DIR    = os.path.join(PYTHON_SRC_DIR, 'files')
sys.path.append(PYTHON_SRC_DIR)
sys.path.append(PYTHON_CONFIG_DIR)
sys.path.append(PYTHON_FILES_DIR)
import backpackconfig
from pint import UnitRegistry
import dataset_filepaths

#
# 	CONSTANTS
#

# Filepaths
TIMEFILE_PATH = os.path.join('time','time_sync.xml')
CONFIGFILE_PATH = os.path.join('config','backpack_config.xml')

# Sensor type tags
CAMERA_TYPE = 'cameras'
FLIR_TYPE = 'flirs'
URG_TYPE = 'lasers'
TOF_TYPE = 'tof_cameras'

# XPATH Paths
URG_DAT_XPATH = 'configFile/urg_datafile'
DALSA_CALIB_XPATH = 'configFile/dalsa_fisheye_calibration_file'
DALSA_METADATA_XPATH = 'configFile/dalsa_metadata_outfile'
DALSA_OUTPUTDIR_XPATH = 'configFile/dalsa_output_directory'
DALSA_MASKFILE_XPATH = 'configFile/dalsa_fisheye_mask_file'
FLIR_CALIB_XPATH = 'configFile/flir_rectilinear_calibration_file'
FLIR_METADATA_XPATH = 'configFile/flir_metadata_outfile'
FLIR_OUTPUTDIR_XPATH = 'configFile/flir_output_directory'
TOF_DAT_XPATH = 'configFile/tof_datafile'

# Locations of executables
POINTCLOUD_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
	"bin",  "pointcloud_gen"))

#
#	SENSOR WHITELISTS
#
URG_WHITELIST = \
	['H1214157', 'H1311822', 'H1004314', 'H1004315', 'H1316223','H1316225']




#
#	Main function
#
def main() :
	
	# Parse the args from the command line
	args = handle_args()

	# Ensure that the given input files actually exist
	check_files_exist(args)

	# Read in the hardware config file so that we have access to the list of 
	# active sensors that were used in this dataset
	conf = backpackconfig.Configuration( \
		os.path.join(args.dataset_directory[0],CONFIGFILE_PATH), \
		True, \
		args.dataset_directory[0])
	args.scanner_list = collect_scanners(args.scanner_list, conf)

	# if there are any time-of-flight (or any depth) cameras present
	# then we should include them in the geometry
	tof_list          = collect_tof(conf)
	all_scanners = args.scanner_list + tof_list

	# Resolve the camera names that will be used in the colorizing the point 
	# cloud
	args.camera_list = collect_cameras(args.color_by, args.camera_list, conf)

	# Ensure that the output folder exists
	args.output_directory = create_output_folder(args)

	# Create common filenames
	timeFile = os.path.join(args.dataset_directory[0], TIMEFILE_PATH)
	configFile = os.path.join(args.dataset_directory[0], CONFIGFILE_PATH)
	pathFileBase = os.path.splitext(os.path.basename(args.path_file[0]))[0]

	# Handle the units
	args.units = handle_units(args.units[0])

	# Lastly run the executable for each scanner
	for scanner in all_scanners :

		print ""
		print "##### Generating Pointcloud for " + scanner + ". Colored by " \
			+ args.color_by + ". #####"
		print ""

		# Here is where the input arguments will be created
		cargs = [POINTCLOUD_EXE]

		# Tack on the time flag
		cargs.append('-t')
		cargs.append(timeFile)

		# Tack on the hardware config file
		cargs.append('-c')
		cargs.append(configFile)

		# Tack on the path
		cargs.append('-p')
		cargs.append(args.path_file[0])

		# give the geometry scanner info
		if scanner in args.scanner_list:

			# Tack on the laser scanner info
			scannerDataFile = conf.find_sensor_prop(scanner, \
				URG_DAT_XPATH, \
				URG_TYPE)
			scannerDataFile = os.path.join(args.dataset_directory[0], \
				scannerDataFile)
			cargs.append('-l')
			cargs.append(scanner)
			cargs.append(scannerDataFile)

		elif scanner in tof_list:

			# this is a tof sensor, which should have
			# a .fss file
			tofDatFile = conf.find_sensor_prop(scanner, \
				TOF_DAT_XPATH, \
				TOF_TYPE)
			tofFssFile = os.path.join( \
				args.dataset_directory[0], \
				dataset_filepaths.get_fss_file( \
				tofDatFile))
			cargs.append('--fss')
			cargs.append(tofFssFile)

		# Handle tacking on the colorization flag
		if args.color_by == 'cameras' :
			for camera in args.camera_list :

				# Give the information for the camera
				cmetadata = conf.find_sensor_prop(camera, \
					DALSA_METADATA_XPATH, \
					CAMERA_TYPE)
				cmetadata = os.path.dirname(cmetadata)
				cmetadata = os.path.join(args.dataset_directory[0], \
					cmetadata, \
					'color_'+camera+'_metadata.txt')
				calibFile = conf.find_sensor_prop(camera, \
					DALSA_CALIB_XPATH, \
					CAMERA_TYPE)
				calibFile = os.path.join(args.dataset_directory[0], \
					calibFile)
				imgDir = conf.find_sensor_prop(camera, \
					DALSA_OUTPUTDIR_XPATH, \
					CAMERA_TYPE)
				imgDir = os.path.normpath(os.path.join( \
					args.dataset_directory[0], imgDir, '..', 'color'))
				cargs.append('-f')
				cargs.append(cmetadata)
				cargs.append(calibFile)
				cargs.append(imgDir)

				#Check if a mask exists and if so register it for the camera
				maskFile = conf.find_sensor_prop(camera, \
					DALSA_MASKFILE_XPATH, \
					CAMERA_TYPE)
				if maskFile != None :
					maskFile = os.path.join(args.dataset_directory[0], \
						maskFile)
					cargs.append('-m')
					cargs.append(camera)
					cargs.append(maskFile)

		elif args.color_by == 'ir_cameras' :
			for camera in args.camera_list :

				# Give the information for the camera
				cmetadata = conf.find_sensor_prop(camera, \
					FLIR_METADATA_XPATH, \
					FLIR_TYPE)
				cmetadata = os.path.dirname(cmetadata)
				cmetadata = os.path.join(args.dataset_directory[0], \
					cmetadata, \
					'normalized_'+camera+'_metadata.txt')
				calibFile = conf.find_sensor_prop(camera, \
					FLIR_CALIB_XPATH, \
					FLIR_TYPE)
				calibFile = os.path.join(args.dataset_directory[0], \
					calibFile)
				imgDir = conf.find_sensor_prop(camera, \
					FLIR_OUTPUTDIR_XPATH, \
					FLIR_TYPE)
				imgDir = os.path.normpath(os.path.join( \
					args.dataset_directory[0], imgDir, '..', 'normalized'))
				cargs.append('-q')
				cargs.append(cmetadata)
				cargs.append(calibFile)
				cargs.append(imgDir)
				
		elif args.color_by == 'time' :
			cargs.append('--color_by_time')
		elif args.color_by == 'height' :
			cargs.append('--color_by_height')

		# Tack on the units flag
		cargs.append('-u')
		cargs.append(str(args.units))

		# Tack on the range limit flag
		if args.range_limit != None :
			cargs.append('-r')
			cargs.append(str(args.range_limit[0]))

		# Tack on the time buffer settings
		if not args.disable_time_buffer :
			cargs.append('--time_buffer')
			cargs.append(str(args.time_buffer[0]))
			cargs.append(str(args.time_buffer[1]))

		# if we are removing uncolored points tack them on
		if not args.keep_noncolored_points :
			cargs.append('--remove_noncolored_points')
		else :
			cargs.append('--default_color')
			cargs.append(str(args.default_color[0]))
			cargs.append(str(args.default_color[1]))
			cargs.append(str(args.default_color[2]))

		# Make the output file
		outFileName = os.path.join(args.output_directory,
			pathFileBase + "_" + scanner + "." + args.output_filetype)
		cargs.append("-o")
		cargs.append(outFileName)
		
		# Run the pointcloud code
		ret = os.system(" ".join(cargs))
		if ret != 0 :
			exit(1)


#
#	Function to handle the units string
#
def handle_units(units):

	# Check if all numeric. If it is numeric then we just pass it back
	# because we are already done
	try :
		float(units)
		return units
	except:
		pass

	# Check to see if it only contains a-z
	units = units.lower()
	isAlpha = True
	for c in units :
		if c not in string.lowercase:
			isAlpha = False
	if not isAlpha :
		raise Exception("units string must be either convertable to float " + \
			"or purely alphabetic.")

	# Strip an s on the end
	if units[-1] == 's' :
		units = units[0:-1]

	# Get the units registry
	ureg = UnitRegistry(os.path.join(PYTHON_SRC_DIR, 'pint', \
		'default_en.txt'), True)

	# Check if this unit exists
	if units not in ureg._units :
		raise Exception("Units: " + units + " not a known unit of measure.")

	# Create a quantity from it
	Q_ = ureg.Quantity
	try :
		unitStr = str(Q_(units).to('meter'))
	except :
		raise Exception(units + " has no converstion to meters.")

	# Get the actual conversion in string form
	unitStr = unitStr.split(" ")[0]

	# Return the result
	return unitStr

#
#	Function to create the output folder
#		
def create_output_folder(args) :

	# Figure out the output directory
	if args.output_directory == None :
		output_directory = os.path.join(args.dataset_directory[0], \
			'models', \
			'pointclouds', \
			args.color_by)
	else :
		output_directory = args.output_directory[0]

	# Create it if needed
	if not os.path.exists(output_directory) :
		os.makedirs(output_directory)

	return output_directory


#
#	Functions to handle deducing the names of the scanners and cameras that
#	will be used
#
def collect_cameras(color_by, camera_list, conf) :

	cameras_to_use = []

	# Check if we need to even look for cameras
	if color_by != 'cameras' and color_by != 'ir_cameras' :
		return cameras_to_use

	# List all active cameras
	if color_by == 'cameras' :
		cameraNames = conf.find_sensors_by_type(CAMERA_TYPE)
	elif color_by == 'ir_cameras' :
		cameraNames = conf.find_sensors_by_type(FLIR_TYPE)

	# Use all cameras
	if camera_list == None :
		cameras_to_use  = cameraNames

	# Use all given cameras that
	else :
		for camera in camera_list :
			if camera in cameraNames :
				cameras_to_use.append(camera)
			else :
				print "Warning : " + camera + " is not an active camera " \
					"for this dataset under colorization mode \"" \
					+ color_by + "\".  Ignoring."

	# Check if any cameras are active
	if len(cameras_to_use) == 0 :
		print "No cameras are available under colorization mode \"" \
			+ color_by + "\". Aborting."
		exit(2)

	
	# Return the collected cameras
	return cameras_to_use

def collect_scanners(scanner_list, conf) :

	# Collect all active lasers
	urgSensors = conf.find_sensors_by_type(URG_TYPE)

	# If no scanners were given at the command line then use all scanners
	# that are in the whitelist and also active
	lasers_to_use = []
	if scanner_list == None :
		for urg in urgSensors :
			if urg in URG_WHITELIST :
				lasers_to_use.append(urg)
	else :
		for scanner in scanner_list :
			if scanner in urgSensors :
				lasers_to_use.append(scanner)
			else :
				print "Warning : " + scanner + " is not an active scanner " \
					"for this dataset.  Ignoring."

	# Check if there are any laser scanners
	if len(lasers_to_use) == 0 :
		print "No given or whitelisted lasers were active in the dataset. "
	
	# Return the list of scanners
	return lasers_to_use

def collect_tof(conf) :

	# collect all active tof cameras
	tofSensors = conf.find_sensors_by_type(TOF_TYPE)

	# use all of them
	return tofSensors

#
#	Function that checks if the input arguments exist. 
#
def check_files_exist(args) :

	# Check that the dataset directory actually exists
	if not os.path.exists(args.dataset_directory[0]) :
		raise Exception("Dataset directory does not exist on filesystem!")

	# Check that the given path file exists
	if not os.path.exists(args.path_file[0]) :
		raise Exception("Path file does not exist on filesystem!")

	# Check that the configuration file exists
	if not os.path.exists(os.path.join( \
			args.dataset_directory[0], CONFIGFILE_PATH)) :
		raise Exception("Configuration xml does not exist on filesystem!")

	# Check if the time sync file exists 
	if not os.path.exists(os.path.join( \
			args.dataset_directory[0], TIMEFILE_PATH)) :
		raise Exception("Time sync xml does not exist on filesystem!")


#
#	Function to handle the argument parsing
#
def handle_args() :

	# Set up the argumnet parser
	parser = argparse.ArgumentParser()
	parser.add_argument("-i", "--dataset_directory", 
		required=True, 
		help=("The input directory. This should be the root directory " 
			  "of the dataset. This is typically a folder named the date "
			  "and set number output by the system."),
		nargs=1,
        type=str)
	parser.add_argument("-p", "--path_file",
		required=True,
		help=("The file name of the path file. This can be relative to the "
			"dataset directory or an absolute path. This is typically the "
			"optimized mad file from the localiaztion code."),
		nargs=1,
		type=str)
	parser.add_argument("-o", "--output_directory",
		required=False,
		help=("The desired directory where the point clouds will be stored. "
			"If not given this will default to: <dataset_directory>/models/"
			"pointclouds/<color_by>/"),
		nargs=1,
		default=None)
	parser.add_argument("-x", "--output_filetype",
		required=False,
		help=("The desired output file type. The default is \"xyz\"."),
		choices=['txt','pts','xyz','obj','las','laz','pcd'],
		default='xyz')
	parser.add_argument("--keep_noncolored_points",
		required=False,
		help=("Flags the point cloud generation code to not export points "
			"which are uncolorable. By default this will ignore uncolorable "
			"points."),
		action="store_true",
		default=False)
	parser.add_argument("--default_color",
		required=False,
		help=("Sets the default color for uncolorizable points. This should "
			"given as 3 integers between the range of [0 255] representing "
			"the default <red> <green> <blue> components. If not given this "
			"will default to black."),
		nargs=3,
		type=int,
		default=[0, 0, 0])
	parser.add_argument("--color_by",
		required=False,
		help=("Flags how the point cloud code should be colorized. The "
			"code will allow coloring linearly by height or time or using "
			"camera imagery. By default this will use the visibile light "
			"cameras."),
		choices=['cameras','ir_cameras','time','height','uncolored'],
        default='cameras')
	parser.add_argument("--camera_list",
		required=False,
		help=("The list of camera names that will be used for colorizing the "
			"point cloud. This flag is unignored if --color_by is not set "
			"to \"cameras\" or \"ir_cameras\". If left blank, it will use "
			"all cameras of that type."),
		nargs='*',
		default=None)
	parser.add_argument("--scanner_list",
		required=False,
		help=("The list of scanners that will be used for making point clouds."
			"By default this will be all white listed geometry scanners."),
		nargs='*',
		type=str)
	parser.add_argument("-r", "--range_limit",
		required=False,
		help=("Sets what the maximum allowed scanner range is before "
			"discarding points. By default, all points are used."),
		nargs=1,
		type=float,
		default=None)
	parser.add_argument("--disable_time_buffer",
		required=False,
		help=("Disables the -t flag from being passed to the pointcloud "
			"code."),
		action='store_true',
		default=False)
	parser.add_argument("-t","--time_buffer",
		required=False,
		help=("Specifies how far to search temporally when using images to "
			"color points.  This flag takes two inputs <range> and <dt>. "
			"Both values are doubles, in units of seconds.  Range indicates "
			"how far to search from timestamp of point and dt indicates "
			"spacing to search.  By default this is set to 2 0.5."),
		nargs=2,
		type=float,
		default=[2.0,0.5])
	parser.add_argument("-u", "--units",
		required=False,
		help=("Specifies the units in which the output point cloud will saved."
			"This can either be a number or the such as \"1.0\" or \"3.2\", "
			"which signifies or the converstion from meters to the desired "
			"units or the name of the desired units, such as \"meters\" or "
			"\"feet\". The defualt value is \"meters\"."),
		nargs=1,
		type=str,
		default=['1.0'])

	# Parse and return
	args = parser.parse_args()

	# Return the args
	return args





# If run from the command line then just run the function
if __name__ == "__main__" :
	main()
