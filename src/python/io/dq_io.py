#!/usr/bin/python

##
# @file dq_io.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  I/O functionality for .dq files
#
# @section DESCRIPTION
#
# This file contains functions that will parse wall samples from
# .dq files and allow for various processing on them.
#

##
# The WallSamples class holds information from .dq files
#
class WallSamples:
	'Class defines wall sample geometry information'

	##
	# Constructor generates class from .dq file
	#
	# This class has the following attributes:
	#
	#	max_depth		max depth of the quadtree
	#	halfwidth		halfwidth of the root node
	#	x			x-coordinate of octree center
	#	y			y-coordinate of octree center
	#
	#	samples_x		A length-N list of x-coordinates
	#	samples_y		A length-N list of y-coordinates
	#	samples_minz		A length-N list of min-z values
	#	samples_maxz		A length-N list of max-z values
	#	samples_numpts		A length-N list of num points 
	#				per sample
	#	samples_poses		A length-N list, where each 
	#				element is a list of pose indices
	#
	# All positions are in units of meters
	#
	def __init__(self, input_file=None):

		# initialize fields
		self.max_depth      = -1
		self.halfwidth      = -1
		self.x              = 0
		self.y              = 0
		self.samples_x      = []
		self.samples_y      = []
		self.samples_minz   = []
		self.samples_maxz   = []
		self.samples_numpts = []
		self.samples_poses  = []
		
		# check arguments
		if input_file != None:
			self.read(input_file)

	##
	# Reads data from input .dq file
	#
	# Will parse the file specified 
	#
	# @param input_file   The input file path to read from
	#
	def read(self, input_file):
        
		# attempt to open file to read
        	with open(input_file) as f:
        		# read file
        		content = f.read().splitlines()
        
	        # check formatting
	        if len(content) < 4:
			raise IOError("Invalid header in .dq file:" + \
        		input_file)

		# store header in object
		self.max_depth = int(content[0])
		self.halfwidth = float(content[1])
		center         = content[2].split()
		self.x         = float(center[0])
		self.y         = float(center[1])

		# iterate through the samples
		for i in range(3, len(content)):

			# parse the current wall sample info
			curr = content[i].split()

			# get the info at the start of the line
			self.samples_x.append(float(curr[0]))
			self.samples_y.append(float(curr[1]))
			self.samples_minz.append(float(curr[2]))
			self.samples_maxz.append(float(curr[3]))
			self.samples_numpts.append(float(curr[4]))
			num_poses = int(curr[5])

			# iterate through the pose indices for this sample
			s = curr[6:]
			self.samples_poses.append( \
				[float(s[j]) for j in range(num_poses)])


