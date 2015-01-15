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
import argparse
import subprocess

# Import our python files
SCRIPT_LOCATION = os.path.dirname(__file__)
PYTHON_SRC_DIR = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
                                 '..', 'src','python'))
PYTHON_CONFIG_DIR   = os.path.join(PYTHON_SRC_DIR, 'config')
sys.path.append(PYTHON_SRC_DIR)
sys.path.append(PYTHON_CONFIG_DIR)
import backpackconfig


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

# XPATH Paths
URG_DAT_XPATH = 'configFile/urg_datafile'
DALSA_CALIB_XPATH = 'configFile/dalsa_fisheye_calibration_file'
DALSA_METADATA_XPATH = 'configFile/dalsa_metadata_outfile'
DALSA_OUTPUTDIR_XPATH = 'configFile/dalsa_output_directory'
FLIR_CALIB_XPATH = 'configFile/flir_rectilinear_calibration_file'
FLIR_METADATA_XPATH = 'configFile/flir_metadata_outfile'
FLIR_OUTPUTDIR_XPATH = 'configFile/flir_output_directory'

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

	# Read in the hardware config file so that we have access to the list of 
	# active sensors that were used in this dataset
	conf = backpackconfig.Configuration( \
		os.path.join(args.dataset_directory[0],CONFIGFILE_PATH), \
		True, \
		args.dataset_directory[0])
	args.scanner_list = collect_scanners(args.scanner_list, conf)

	# Resolve the camera names that will be used in the colorizing the point 
	# cloud
	args.camera_list = collect_cameras(args.color_by, args.camera_list, conf)

	# Ensure that the output folder exists
	args.output_directory = create_output_folder(args)

	# Create common filenames
	timeFile = os.path.join(args.dataset_directory[0], TIMEFILE_PATH)
	configFile = os.path.join(args.dataset_directory[0], CONFIGFILE_PATH)
	pathFileBase = os.path.splitext(os.path.basename(args.path_file[0]))[0]

	# Lastly run the executable for each scanner
	for scanner in args.scanner_list :

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

		# Tack on the laser scanner info
		scannerDataFile = conf.find_sensor_prop(scanner, \
			URG_DAT_XPATH, \
			URG_TYPE)
		scannerDataFile = os.path.join(args.dataset_directory[0], \
			scannerDataFile)
		cargs.append('-l')
		cargs.append(scanner)
		cargs.append(scannerDataFile)

		# Handle tacking on the colorization flag
		if args.color_by == 'cameras' :
			for camera in args.camera_list :
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
		elif args.color_by == 'ir_cameras' :
			for camera in args.camera_list :
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
		print "No given or whitelisted scanners were active in the dataset. " \
			"Aborting."
		exit(1)
	
	# Return the list of scanners
	return lasers_to_use


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
		choices=['txt','pts','xyz','obj'],
		default='xyz')
	#parser.add_argument("--single_output_file",
	#	required=False,
	#	help=("Specifies that all scanners should be merged into a single "
	#		"output file instead of a single file per scanner."),
	#	action="store_true",
	#	default=False)
	parser.add_argument("--keep_noncolored_points",
		required=False,
		help=("Flags the point cloud generation code to not export points "
			"which are uncolorable. By default this will ignore uncolorable "
			"points."),
		action="store_true",
		default=False)
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
		help=("Floating point number that specifies the units of the output " 
			"point cloud. A value of 1.0 indicates meteres, 1000.0 indicates "
			"millimeters, 3.28084 indicates feet, etc. The default is 1.0 "
			"(meters)."),
		nargs=1,
		type=float,
		default=1.0)

	# Parse and return
	args = parser.parse_args()

	# Return the args
	return args





# If run from the command line then just run the function
if __name__ == "__main__" :
	main()