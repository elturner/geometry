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
import random
from Tkinter import *

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from fp_io import Floorplan

# the size of the window to make
WINDOW_WIDTH  = 1000
WINDOW_HEIGHT = 800

# default floorplan color, if we don't color-by-room
FOREGROUND_COLOR = '#0099FF'

#-----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS -----------------
#-----------------------------------------------------

##
# The main function of this script, which is used to run render_fp
#
# This call will run the render_fp program, verifying inputs and
# outputs.
#
# @param fpfile        The path to the .fp file to parse
# @param color_by_room If true, will color each room separately 
#
# @return              Returns zero on success, non-zero on failure
#
def run(fpfile, color_by_room=True):

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

	# iterate over the rooms of this floorplan
	for room in range(fp.num_rooms):
	
		# generate a random color for this room
		r = lambda: random.randint(64,192)
		room_color = '#{:02x}{:02x}{:02x}'.format(r(), r(), r())
		if not color_by_room:
			room_color = FOREGROUND_COLOR

		# iterate over the triangles of the room, drawing them
		for ti in fp.room_tris[room]:
		
			# get vertices of this triangle
			(i,j,k) = fp.tris[ti]
			vs = [fp.verts[i], fp.verts[j], fp.verts[k]]
			coords = []
			for (x,y) in vs:
				# convert vertex position to pixels
				px = int(pix_per_m * (x-x_min))
				py = int(pix_per_m * (y_max-y))
				coords += [px, py]

			# draw this triangle
			canvas.create_polygon(*coords, fill=room_color)

	# draw boundary edges over triangles
	boundary_edges = fp.compute_boundary_edges()
	for (i,j) in boundary_edges:

		# get vertices of this line
		vs = [fp.verts[i], fp.verts[j]]
		coords = []
		for (x,y) in vs:
			# convert to pixels
			coords += [int(pix_per_m * (x-x_min)), \
					int(pix_per_m * (y_max-y))]

		# draw the line
		canvas.create_line(*coords, width=2)

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
	if len(sys.argv) != 2 and len(sys.argv) != 3:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"[--no_color]","<path_to_fp_file>"
		print ""
		sys.exit(1)

	# get arguments
	color_by_room = True
	fpfile = sys.argv[1]
	if sys.argv[1] == '--no_color':
		color_by_room = False
		fpfile = sys.argv[2]

	# run this script with the given arguments
	ret = run(fpfile, color_by_room)
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
