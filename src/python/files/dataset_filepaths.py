#!/usr/bin/python

##
# @file dataset_filepaths.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief Stores expected locations of files in datasets
# 
# @section DESCRIPTION
#
# This file contains functions that return the expected
# file locations of various dataset files when given
# the path to a dataset.
#

# Get the location of this file
import os
import sys
SCRIPT_LOCATION = os.path.dirname(__file__)

# import libraries
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
import config

#---------------- Generic file functions ------------------------

##
# Returns the body name of the given file
#
def get_file_body(f):
	(head, tail) = os.path.split(f)
	(body, ext) = os.path.splitext(tail)
	return body

#---------------- Files within a dataset ------------------------

##
# Returns the expected location of the hardware config xml file
#
def get_hardware_config_xml(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"config","backpack_config.xml"))

##
# Returns the expected location of the timestamp sync xml file
#
def get_timesync_xml(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"time", "time_sync.xml"))

#--------------- Sensor-specific files ------------------------

##
# Returns the expected location of the .fss files in this dataset as a list
#
# Note:  this does not include d-imager-generated fss files, but only those
# generated from laser scanners.
#
def get_all_fss_files(dataset_dir):

	# get the config xml location
	config_xml = get_hardware_config_xml(dataset_dir)

	# parse the hardware configuration file
	sensor_types, sensor_cfn = config.parse_backpack_xml(config_xml)
	if sensor_types is None or sensor_cfn is None \
			or len(sensor_types) != len(sensor_cfn):
		return None # could not parse the xml file

	# find the scan files to use as input
	fssfiles = []
	for si in range(len(sensor_types)):	
		# check for laser scanners
		if sensor_types[si] == 'laser':

			# parse the settings file for this laser
			urg_settings = config.parse_settings_xml( \
					os.path.normpath(os.path.join( \
					dataset_dir, sensor_cfn[si])))
			if urg_settings is None:
				return None # unable to parse settings

			# the fss file is named the same as the raw
			# data file, but with a different file ext
			urg_filename = urg_settings["urg_datafile"]
			urg_outname,ext = os.path.splitext(urg_filename)
			urg_outname += '.fss'

			# add to our list
			fssfiles.append(urg_outname)

	# return the final list of fss files
	return fssfiles

#--------------- Files generated from pointcloud code --------------

##
# Returns the location of the models directory
#
def get_models_dir(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir,"models"))

##
# Returns the location of the pointclouds directory
#
def get_pointcloud_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"pointclouds")

##
# Returns the location of the pointcloud levels directory
#
def get_pc_levels_dir(dataset_dir):
	return os.path.join(get_pointcloud_dir(dataset_dir),"levels")

##
# Returns a list of the pointcloud files in the levels directory
#
def get_pc_levels_list(dataset_dir):
	# get the pointcloud levels directory
	levdir = get_pc_levels_dir(dataset_dir)

	# iterate through files in this directory
	pcfiles = []
	for f in os.listdir(levdir):
		(body, ext) = os.path.splitext(f)
		if ext == '.xyz':
			pcfiles.append(os.path.join(levdir, f))

	# return final list of pointcloud files
	return pcfiles

#--------------- Files generated from floorplan code ---------------

##
# Returns the location of the floorplan directory
#
def get_floorplan_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"floorplan")

##
# Returns the location of the pseudo-texture directory 
#
def get_faketexture_dir(dataset_dir):
	return os.path.join(get_floorplan_dir(dataset_dir),"texture")

##
# Returns the wall sampling file that corresponds with the given pointcloud
#
# @param dataset_dir   The location of the dataset directory
# @param pc_file       The location of the pointcloud file
#
def get_dq_file(dataset_dir, pc_file):
	return os.path.join(get_floorplan_dir(dataset_dir), \
	                    get_file_body(pc_file) + ".dq")

##
# Returns the fp file that corresponds with the input dq file
#
def get_fp_file(dq_file):
	(body, ext) = os.path.splitext(dq_file)
	return (body + ".fp")

##
# Returns the floorplan mesh file, given the fp
#
def get_floorplan_obj_file(dataset_dir, fp_file):
	return os.path.join(get_faketexture_dir(dataset_dir), \
		get_file_body(fp_file) + ".obj")

##
# Returns all floorplan files in the floorplans directory
#
def get_all_floorplan_files(dataset_dir):
	
	# get the floorplan directory
	fpdir = get_floorplan_dir(dataset_dir)
	
	# get all fp files in this directory
	fpfiles = [f for f in os.listdir(fpdir) if f.endswith(".fp")]
	return fpfiles

#--------------- Files generated from carving --------------------

##
# Returns the location of the carving directory
#
def get_carving_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"carving")

##
# Returns the location of the wedge file in a dataset
#
def get_wedgefile(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir),"scandists.wedge")

##
# Returns the expected location of the chunklist file
#
def get_chunklist(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir),"cache.chunklist")
##
# Returns the expected location of the carved octree file
#
def get_octree(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), "carving.oct")


