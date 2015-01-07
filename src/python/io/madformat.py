#
#	madformat.py
#
#	Gives a class for reading .mad path files.
#
from struct import *

class Mad :

	#
	#	__init__(self, filename=None)
	#
	#	The class initialization routine
	#
	def __init__(self, filename=None) :

		self.clear()
		if filename is not None :
			self.read_mad(filename)

	#
	#	clear(self)
	#
	#	Clears the class of any existing data
	#
	def clear(self) :
		self.poses = []
		self.times = []
		self.zupts = []

	#
	#	read_mad(self, filename)
	#
	#	Reads the contents of the .mad file into the class.
	#
	def read_mad(self, filename) :

		# Clear the internal data so we can append below with no
		# worries
		self.clear()

		# Open the mad file in binary mode because mad files are 
		# binary files
		f = open(filename, 'rb')

		# Read in the zero-velocity update information
		numZupts = unpack('i',f.read(4))[0]
		for i in range(0,numZupts) :
			self.zupts.append(unpack('dd',f.read(8*2)))

		# Read in the number of poses
		numPoses = unpack('i',f.read(4))[0]
		for i in range(0, numPoses) :
			self.times.append(unpack('d',f.read(8))[0])
			self.poses.append(unpack('dddddd',f.read(8*6)))

	#
	#	write_mad(self, filename) :
	#
	#	Writes the internal class data back to the .mad format
	#
	def write_mad(self, filename) :

		# Ensure that we have a sane quantity of things stored in the class
		if len(self.times) != len(self.poses) :
			Exception("File does not contain equal number of times and poses.")

		# Open the file for writing in binary mode
		f = open(filename, 'wb')

		# Write the number of zupts
		f.write(pack('i',len(self.zupts)))
		for zupt in self.zupts :
			f.write(pack('d',zupt[0]))
			f.write(pack('d',zupt[1]))

		# Write the number of poses
		f.write(pack('i',len(self.poses)))
		for i in range(0, len(self.poses)) :
			f.write(pack('d', self.times[i]))
			f.write(pack('d', self.poses[i][0]))
			f.write(pack('d', self.poses[i][1]))
			f.write(pack('d', self.poses[i][2]))
			f.write(pack('d', self.poses[i][3]))
			f.write(pack('d', self.poses[i][4]))
			f.write(pack('d', self.poses[i][5]))

		# Close like a good citizen
		f.close()






		
		