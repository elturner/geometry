#!/usr/bin/python

##
# @file render_fp.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Will render the floorplan from an .fp file to a window
#
# @section DESCRIPTION
#
# Will read the given .fp file, parsing the flooplan information, and
# will open a window showing the floorplan geometry
#

# Import standard python libraries
import os
import sys
from Tkinter import *

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from fp_io import Floorplan

# the size of the window to make
WINDOW_WIDTH  = 800
WINDOW_HEIGHT = 800

#-----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS -----------------
#-----------------------------------------------------

##
# The main function of this script, which is used to run render_fp
#
# This call will run the render_fp program, verifying inputs and
# outputs.
#
# @param fpfile       The path to the .fp file to parse
#
# @return             Returns zero on success, non-zero on failure
#
def run(fpfile):

	# read the input file
	fp = Floorplan()
	fp.read(fpfile) # throws error if failure occurs

	# prepare canvas based on bounds of floorplan
	((x_min, x_max), (y_min, y_max)) = fp.compute_bounds()
	x_range = (x_max-x_min)
	y_range = (y_max-y_min)
	pix_per_m = min(WINDOW_WIDTH/x_range, WINDOW_HEIGHT/y_range)
	root = Tk()
	root.title('Floorplan: ' + fpfile)
	canvas = Canvas(root, width=int(pix_per_m*x_range), \
				height=int(pix_per_m*y_range))

	# iterate over the triangles of the floorplan, drawing them
	for t in fp.tris:
		
		# get vertices of this triangle
		(i,j,k) = t
		vs = [fp.verts[i], fp.verts[j], fp.verts[k]]
		coords = []
		for v in vs:
			# convert vertex position to pixels
			(x,y) = v
			px = int(pix_per_m * (x-x_min))
			py = int(pix_per_m * (y_max-y))
			coords += [px, py]

		# draw this triangle
		canvas.create_polygon(*coords, \
			outline='black', fill='blue', width=2)

	# show the canvas
	canvas.pack()
	root.mainloop()

	# success
	return 0

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
		print "\t",sys.argv[0],"<path_to_fp_file>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1])
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
