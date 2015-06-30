#!/usr/bin/python

##
# @file   fp2shp.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Exports floorplan to .dxf file
#
# @section DESCRIPTION
#
# This script will convert floorplan files (.fp) to 
# DXF R12 drawings (.dxf).  These .dxf files can be
# imported into AutoCAD.
#
# This script uses the third-party library 'dxfwrite'.
# This library must be installed for this script to
# function.  See links here:
#
# install:      https://pypi.python.org/pypi/dxfwrite/
#
# docs:         http://packages.python.org/dxfwrite/
#

import sys
import fp_io
from dxfwrite import DXFEngine as dxf

##
# This function converts a .fp file to a .dxf file
#
# @param fpfile   The input .fp file
# @param dxffile  The output .dxf file
#
def run(fpfile, dxffile):
    
    # read the input floorplan
    fp = fp_io.Floorplan(fpfile)

    # prepare the output dxf file
    drawing = dxf.drawing(dxffile)

    # add a layer with all vertices
    points_layer_name = 'wall_samples'
    points_layer = dxf.layer(points_layer_name)
    points_layer['color'] = 1 # valid colors: [1,255]
    drawing.layers.add(points_layer)
    for (vx,vy) in fp.verts:
        # add each point to this layer
        point = dxf.point((vx,vy))
        point['layer']       = points_layer_name
        point['color']       = 256 # color by layer
        point['paper_space'] = 0 # put in model space
        drawing.add(point)

    # iterate over the rooms of the floor plan
    for room_id in range(fp.num_rooms):

        # generate the boundary for this room
        #
        # room_edges is a list of tuples, where each
        # tuple is two vertex indices
        room_edges = fp.compute_room_boundary_edges(room_id)
        
        # make a layer for this room
        room_layer_name = 'room_' + str(room_id+1)
        room_layer = dxf.layer(room_layer_name) 
        room_layer['color'] = 2+room_id # valid colors: [1,255]
        drawing.layers.add(room_layer)

        # draw each edge
        for (vi,vj) in room_edges:
            # add each edge as a line in the drawing
            line = dxf.line( fp.verts[vi], fp.verts[vj] )
            line['layer']       = room_layer_name
            line['color']       = 256 # color by layer
            line['paper_space'] = 0 # put in model space
            drawing.add(line)

    # save the drawing to file
    drawing.save()

##
# The main function
#
# This will parse command-line arguments and call run()
#
def main():

    # check command-line arguments
    if len(sys.argv) != 3:
        print ""
        print " Usage:"
        print ""
        print "\t",sys.argv[0],"<fpfile> <dxffile>"
        print ""
        print " Where:"
        print ""
        print "\t<fpfile>    Input floorplan file to read"
        print "\t<dxffile>   Output .dxf file to generate"
        print ""
        sys.exit(1)

    # get values
    fpfile = sys.argv[1]
    dxffile = sys.argv[2]

    # run it
    run(fpfile, dxffile)
    sys.exit(0)

##
# Boilerplate code to call main function as executable
#
if __name__ == '__main__':
    main()

