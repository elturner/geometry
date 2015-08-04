#!/usr/bin/python

##
# @file	   levels_io.py
# @author  Eric Turner <elturner@indoorreality.com>
# @brief   I/O functionality for .levels files
#
# @section DESCRIPTION
#
# This file contains functions that will parse building levels from
# .levels files and allow for various processing on them.
#

##
# The Levels class holds information from .levels files
#
class Levels:
	'Class defines building levels geometry information'

	##
	# The latest supported major file version
	#
	SUPPORTED_FILE_VERSION_MAJOR = 1

	##
	# Constructor generates class from .levels file
	#
	# This class has the following fields:
	#
	#	num_levels 	-	The number of levels found
	#	
	#	floor_heights	-	A list of elevations of the floor
	#				of each level (length N)
	#
	#	ceiling_heights	-	A list of elevations of the ceiling
	#				of each level (length N)
	#
	#	split_heights	-	A list of elevations that mark
	#				the split locations between
	#				level #i and level #i+1 (length N-1)
	#
	# @param input_file   Optional.  An input file to parse.
	#
	def __init__(self, input_file=None):

		# check arguments
		if input_file is None:

			# default fields
			self.num_levels      = 0
			self.floor_heights   = []
			self.ceiling_heights = []
			self.split_heights   = []

		else:
			self.read(input_file)

	##
	# Reads the specified file and populates this structure
	#
	# Will parse the specified file and store the retrieved data
	# in 'self'.
	#
	# @param input_file   The input file to parse
	#
	# @return     Returns zero on success, non-zero on failure.
	#
	def read(self, input_file):

		# read the lines of this file
		lines = open(input_file).read().split('\n')

		# filter out empty lines and comments
		for i in range(len(lines)):
			commentstart = lines[i].find('#')
			if commentstart >= 0:
				lines[i] = lines[i][0:commentstart]
		lines = [L for L in lines if len(L) > 0]

		# next, parse the header, make sure this is a levels file
		if lines[0] != 'levels':
			print 'Error!  Not a valid levels file:',input_file
			return -1

		# parse the fields
		for linenum in range(1,len(lines)):
			
			# split this line into key and values
			vals = lines[linenum].split()

			# what we do depends on the key
			if vals[0] == 'version':

				# parse the version
				if len(vals) != 3:
					print "[levels_io]\tError!  " \
						+ "Cannot parse version: " \
						+ lines[linenum]
					return -2
				
				# check version
				major = int(vals[1])
				minor = int(vals[2])
				if major != \
					Levels.SUPPORTED_FILE_VERSION_MAJOR:
					print "[levels_io]\tError!"\
						+ " Unsupported "\
						+ "file version: ",\
						major
					return -3

			elif vals[0] == 'num_levels':
				
				# check that line is correctly formatted
				if len(vals) != 2:
					print "[levels_io]\tError! " \
						+ "Cannot parse num " \
						+ "levels: " \
						+ lines[linenum]
					return -4

				# store the number of levels
				self.num_levels = int(vals[1])

				# resize the lists (initialize as invalid)
				self.floor_heights   = [1]*self.num_levels
				self.ceiling_heights = [0]*self.num_levels

			elif vals[0] == 'level':
				
				# check that line is formatted correctly
				if len(vals) != 4:
					print "[levels_io]\tError! " \
						+ "Cannot parse level: " \
						+ lines[linenum]
					return -5

				# check index is valid
				levind = int(vals[1])
				if levind >= self.num_levels or levind < 0:
					print "[levels_io]\tError! " \
						+ "invalid level index: " \
						+ vals[1]
					return -6;

				# store the level information
				self.floor_heights[levind]   =float(vals[2])
				self.ceiling_heights[levind] =float(vals[3])

			else:

				# don't know what this is
				print '[levels_io]\tError!  " \
						+ "Unknown field:',vals[0]
				return -7

		# populate the split heights
		self.split_heights = \
			[(0.5 * (self.floor_heights[i+1] \
				+ self.ceiling_heights[i])) \
					for i in range(self.num_levels - 1)]


		# success
		return 0
