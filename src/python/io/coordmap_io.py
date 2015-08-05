#!/usr/bin/python

##
# @file   coordmap_io.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  I/O functionality for coordinate mapping .txt files
#
# @section DESCRIPTION
#
# This file contains functions that will parse coordinate mappings from
# .txt files and allow for various processing on them.
#

##
# The CoordMap class holds information from coordinate mapping .txt files
#
class CoordMap:
	'Class defines 2D coordinate mappings'

	##
	# Constructor generates class from coord map .txt file
	#
	# This class has the following fields:
	#
	#	resolution	-	scale of mapping
	#	offset_x	-	offset in x-dimension
	#	offset_y	-	offset in y-dimension
	#
	# To use these parameters:
	#
	#	model_x = (image_x - offsetX)*resolution
	#	model_y = (image_y - offsetY)*resolution
	#
	# @param input_file    Optional input file to initialize
	#
	def __init__(self, input_file=None):

		# check if file given
		if input_file is None:
			# initialize to identity mapping
			resolution = 1.0
			offset_x   = 0.0
			offset_y   = 0.0
		else:
			self.read(input_file)

	##
	# Reads the specified input file
	#
	# @param input_File   The file to read
	#
	def read(self, input_file=None):

		# read the file contents
		vals = open(input_file).read().split()

		# check validity
		if len(vals) < 3:
			raise IOException('Not valid coord map file: ' \
					+ input_file)

		# store values
		self.resolution = float(vals[0])
		self.offset_x   = float(vals[1])
		self.offset_y   = float(vals[2])

