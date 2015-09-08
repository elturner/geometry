#!/usr/bin/python

##
# @file   process_tango.py
# @author Eric Turner <elturner@indoorreality.edu>
# @brief  Calls all programs to generate models from Tango scans
#
# @section DESCRIPTION
#
# This metascript will call the python scripts in charge
# of the individual programs to create models from tango scans.
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
POINTCLOUD_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
				"bin",  "pointcloud_gen"))

# this variable specifies the number of stages in this script
NUM_STAGES = 4

# import local libraries
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'geometry'))
import filter_tango_scans
import floorplan
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'files'))
import dataset_filepaths
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'config'))
import backpackconfig
sys.path.append(os.path.join(PYTHON_SRC_LOC, 'latex'))
import generate_tex

#-----------------------------------------------------------------
#------------ RUNS SCRIPTS FOR INDIVIDUAL PROGRAMS ---------------
#-----------------------------------------------------------------

##
# This function runs this script
#
# @param dataset_dir  The path to the dataset to process
# @param madfile      The path to the mad file to use
# @param stages	      The stages of this script to run.
#                     Stages are indexed from 1, and go to NUM_STAGES
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, madfile, stages):
	
	# filter the raw range scans to include statistical information
	# about the scanner intrinsics
	if 1 in stages:
		print ""
		print "STAGE 1 - GENERATING SCAN STATISTICS"
		print ""
		ret = filter_tango_scans.run(dataset_dir)
		if ret != 0:
			return -1 # an error occurred

	# create a colored pointcloud
	if 2 in stages:
		print ""
		print "STAGE 2 - GENERATING POINTCLOUD"
		print ""
		xyzfile = pointcloud_gen(dataset_dir, madfile)
		if xyzfile == None:
			return -2 # an error occurred

	# create a floorplan from the pointcloud	
	if 3 in stages:
		print ""
		print "STAGE 3 - GENERATING FLOORPLAN"
		print ""
		ret = floorplan.run(dataset_dir, madfile, xyzfile)
		if ret != 0:
			return -3 # an error occurred

	# generate tex
	if 4 in stages:
		print ""
		print "STAGE 4 - GENERATING PDF SUMMARY"
		print ""
		ret = generate_tex.run(dataset_dir, \
				dataset_filepaths.get_name_from_madfile(madfile))
		if ret != 0:
			return -4
	
	# success
	return 0

##
# Performs the pointcloud generation call for a tango dataset
#
# @return     Returns path to pointcloud file on success, None on failure.
#
def pointcloud_gen(dataset_dir, madfile):

	# the following constants are used to interface with the system
	# hardware configuration
	CAMERA_TYPE            = 'cameras'
	CAMERA_METAFILE_XPATH  = 'configFile/dalsa_metadata_outfile'
	CAMERA_RECTCALIB_XPATH = 'configFile/flir_rectilinear_calibration_file'
	CAMERA_IMAGEDIR_XPATH  = 'configFile/dalsa_output_directory'
	CAMERA_WHITELIST       = ['tango_color']

	# get the necessary file paths from the dataset
	conffile = dataset_filepaths.get_hardware_config_xml(dataset_dir)
	timefile = dataset_filepaths.get_timesync_xml(dataset_dir)
	fssfiles = dataset_filepaths.get_all_fss_files(dataset_dir)
	xyzfile  = os.path.join( \
			dataset_filepaths.get_colored_pc_dir(dataset_dir), \
			dataset_filepaths.get_name_from_madfile(madfile) \
				+ '_tango.xyz')

	# check if the appropriate camera(s) is/are defined
	conf = backpackconfig.Configuration(conffile, True, dataset_dir)
	cameraSensors = conf.find_sensors_by_type(CAMERA_TYPE)

	# prepare the command-line arguments for this executable
	args = [POINTCLOUD_EXE, \
			'-t', timefile, \
			'-c', conffile, \
			'-p', madfile, \
			'-u', '1000', \
			]

	# add all available sensors
	for f in fssfiles:
		args += ['--fss', f]

	# add all available cameras that are in the whitelist
	colorbycamera = False
	for c in cameraSensors:
		if c in CAMERA_WHITELIST:
			# we should use this camera for color
			colorbycamera = True
			
			# get the argument files for this camera
			cmetadata = dataset_filepaths.get_color_metadata_file( \
					c, \
					os.path.join(dataset_dir, \
					conf.find_sensor_prop(c, \
					CAMERA_METAFILE_XPATH, \
					CAMERA_TYPE)))
			crectcalib = os.path.join(dataset_dir, \
					conf.find_sensor_prop(c, \
					CAMERA_RECTCALIB_XPATH, \
					CAMERA_TYPE))
			cimagedir = os.path.join(dataset_dir, \
					conf.find_sensor_prop(c, \
					CAMERA_IMAGEDIR_XPATH, \
					CAMERA_TYPE))

			# add to the command-line args
			args += ['-q', cmetadata, crectcalib, cimagedir]
	
	# prepare output file
	outdir = os.path.dirname(xyzfile)
	if not os.path.exists(outdir):
		os.makedirs(outdir)
	args += ['-o', xyzfile]

	# call the pointcloud executable
	ret = subprocess.call(args, executable=POINTCLOUD_EXE, \
			cwd=dataset_dir, stdout=None, stderr=None, \
			stdin=None, shell=False)
	if ret != 0:
		print "Pointcloud generation code returned error",ret
		return None

	# return the xyzfile as success
	return xyzfile

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
	madfile	 = args.madfile[0]
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
