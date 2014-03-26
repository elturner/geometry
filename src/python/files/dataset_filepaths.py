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

#--------------- Files generated from processing --------------------

##
# Returns the expected location of the chunklist file
#
def get_chunklist(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"models", "carving", "cache.chunklist"))
##
# Returns the expected location of the carved octree file
#
def get_octree(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"models", "carving", "carving.oct"))

