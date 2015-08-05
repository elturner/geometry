#!/usr/bin/python

##
# @file   light_detection.py
# @author Eric Turner <elturner@indoorreality.com>
# @brief  The light detection python script
# 
# @section DESCRIPTION
#
# This script will call the programs in the light detection
# pipeline after verifying that all input files
# exist, output directories exist, and the given
# dataset is formatted appropriately.
#
# The method employed in this code is the same as developed by
# Satarupa Mukherjee in the UC Berkeley VIP Lab in 2014.
#
# Created August 2015
#

# import python libraries
import os
import sys
import cv2
import numpy
from matplotlib import pyplot as plt

#-------------------------------------
#---------- CONSTANTS ----------------
#-------------------------------------

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local code
import make_ceiling_screenshots
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
import config
import backpackconfig
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import dataset_filepaths
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from levels_io import Levels
from fp_io import Floorplan
from coordmap_io import CoordMap

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will perform light detection for a dataset
#
# This will run the code, but will first check
# that all files exist and the output can be written.  
#
# @param dataset_dir   Path to dataset directory
#
def run(dataset_dir):

	# ensure abspath for input files
	dataset_dir = os.path.abspath(dataset_dir)

	#-------- Generate Screenshots ----------
	ret = make_ceiling_screenshots.run(dataset_dir)
	if ret != 0:
		print "Error!  Unable to make pointcloud ceiling images"
		return -1

	#-------- Perform Detection -----------
	ret = process_model_lights(dataset_dir)
	if ret != 0:
		print "Error!  Unable to perform light detection"
		return -2

	# success
	return 0

#-----------------------------------------------------------
#---------- HELPER FUNCTION IMPLEMENTATIONS ----------------
#-----------------------------------------------------------

##
# Perform light detection for all rooms in this model
#
# Will iterate over all rooms in all levels of the model,
# performing light detection on each room by calling the
# function count_room_lights()
#
# @param dataset_dir    The root dataset directory
#
# @return      Returns zero on success, non-zero on failure.
#
def process_model_lights(dataset_dir):

	# find the important files in the dataset
	fpfiles    = dataset_filepaths.get_carving_fp_files(dataset_dir)
	levelsfile = dataset_filepaths.get_carving_levels_file(dataset_dir)
	levels = Levels()
	ret = levels.read(levelsfile)
	if ret != 0:
		print "Error!  Unable to parse building levels file"
		return -1

	# make sure the data makes sense
	if len(fpfiles) != levels.num_levels:
		print "Error!  Expected number of levels:",levels.num_levels
		print "        # of found floorplans:    ",len(fpfiles)
		return -6

	# iterate over each level in the model
	for level_index in range(levels.num_levels):

		# read this floorplan's information
		fp = Floorplan(fpfiles[level_index])
		
		# prepare list of wattages for each room
		wattages = [0.0] * (fp.num_rooms)

		# iterate over the rooms of this floorplan
		for room_index in range(fp.num_rooms):
			
			# get the files for this room
			room_image_file = dataset_filepaths. \
				get_light_detection_room_ceiling_image( \
						dataset_dir, \
						level_index, \
						room_index)
			room_coord_file = dataset_filepaths. \
				get_light_detection_room_ceiling_coordmap( \
						dataset_dir, \
						level_index, \
						room_index)

			# process this room
			wattages[room_index] = count_room_lights( \
					room_image_file, room_coord_file)

		# export the esimated wattages to the output file
		wattsfile = open(dataset_filepaths \
				.get_light_detection_level_wattages( \
					dataset_dir, level_index), 'w')
		for w in wattages:
			  wattsfile.write("%f\n" % w)
		wattsfile.close()
		
	# success
	return 0

##
# Will estimate the light wattage for a room based on a ceiling screenshot
#
# Given a pointcloud ceiling screenshot and corresponding metadata
# files for one room, will analyze the image to estimate the location
# of light fixtures.
#
# @param room_image_file    The path to the image file for this room
# @param room_coord_file    The coordinate system file for this room
#
# @return      An estimate of the wattage used for lighting in this room
#
def count_room_lights(room_image_file, room_coord_file):

	# the following parameters are used in this code
	WINDOW_SIZE = 0.11 # how much to smooth (units: meters)
	INTENSITY_THRESHOLD = 230 # how bright a light needs to be to detect
	MIN_COMPONENT_SIZE = 0.03 # threshold area of light (units: m^2)
	WATTS_PER_SQUARE_METER = 1 # TODO use good value here

	#---------------
	# open the files
	#---------------
	
	img0 = cv2.imread(room_image_file, cv2.CV_LOAD_IMAGE_GRAYSCALE)
	coords = CoordMap(room_coord_file)

	# get the units of the area of the image
	meters_per_pixel = coords.resolution
	m2_per_pix2      = meters_per_pixel * meters_per_pixel

	# the smoothing window size must be an odd integer
	winsize = int(1+2*(numpy.floor(WINDOW_SIZE/meters_per_pixel/2)));

	#-----------------
	# filter the image
	#-----------------
	
	img1 = cv2.medianBlur(img0, winsize)
	ret,img2 = cv2.threshold(img1, INTENSITY_THRESHOLD, 255, \
			cv2.THRESH_BINARY)

	# find all connected components, and keep ones that are big enough
	img3 = img2.copy()
	components,hierarchy = cv2.findContours(img3, cv2.RETR_CCOMP, \
				cv2.CHAIN_APPROX_NONE)
	areas = [cv2.contourArea(c) for c in components]
	shouldkeep = [False] * len(components)
	for i in range(len(components)):
		area = cv2.contourArea(components[i]) 
		if area <= 0:
			# this component is invalid
			shouldkeep[i] = False
			continue
		if area < (MIN_COMPONENT_SIZE / m2_per_pix2):
			# this component is too small
			cv2.drawContours(img3, components, i, 0)
			shouldkeep[i] = False
		else:
			# this component is big enough to keep
			cv2.drawContours(img3, components, i, 255, \
					thickness=-1) # fill it
			shouldkeep[i] = True

	# export the labeled image (because we can)
	labeledimg = dataset_filepaths.get_light_detection_labeled_image( \
						room_image_file)
	cv2.imwrite(labeledimg, img3)

	# compute the total area of valid lights,
	# in units of square meters
	totalarea = 0
	for i in range(len(areas)):
		if shouldkeep[i]:
			totalarea += (areas[i] * m2_per_pix2)

	# now that we have the area of the lights, we can estimate
	# the wattage by assuming there is a constant power density
	# for the surface of any light (THIS IS A BIG ASSUMPTION)
	return (totalarea * WATTS_PER_SQUARE_METER);


# ----------------------------------------------------
# ------- Boilerplate For Calling Executable ---------
# ----------------------------------------------------

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 2:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1])
	
	# display results
	print ""
	if ret != 0:
		sys.exit(1)

	# success
	sys.exit(0)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
