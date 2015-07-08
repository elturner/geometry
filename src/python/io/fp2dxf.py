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

    # iterate over the rooms of the floor plan
    for room_id in range(fp.num_rooms):

        # make a layer for this room
        room_layer_name = 'room_' + str(room_id+1)
        room_layer = dxf.layer(room_layer_name) 
        room_layer['color'] = 1+room_id # valid colors: [1,255]
        drawing.layers.add(room_layer)
        
        # generate the polylines for this room's boundaries
        polyline_arr = room_to_polylines(fp, room_id)

        # draw each polyline
        for pi in range(len(polyline_arr)):
            # add each polyline to the drawing
            polyline_arr[pi]['layer'] = room_layer_name
            polyline_arr[pi]['color'] = 256 # color by layer
            polyline_arr[pi]['paper_space'] = 0 # put in model space
            drawing.add(polyline_arr[pi])

    # save the drawing to file
    drawing.save()

#----------------------------------------------------------------
#--------------------- Helper Functions -------------------------
#----------------------------------------------------------------

##
# Creates a .dxf polyface object from a room
#
# @param fp        The floor plan object
# @param room_id   The index of the room
#
# @return    Returns the dxf polyface object
#
def room_to_polyface(fp, room_id):

    # prepare the polyface object
    pface = dxf.polyface()

    # add each triangle to the face
    for ti in fp.room_tris[room_id]:

        # get the vertices of this triangle
        (i,j,k) = fp.tris[ti]

        # add this triangle to the polyface
        pface.add_face( [ fp.verts[i], fp.verts[j], fp.verts[k] ] )

    # return the final polyface
    return pface

##
# Creates a .dxf polyline object from a room
#
# Since a room might have genus >= 1, it may be
# represented by multiple disjoint borders.  This
# function will return an array of polyline objects
# based on the room.
#
# @param fp        The floor plan object
# @param room_id   The index of the room
#
# @return    Returns the array of dxf polyline objects
#
def room_to_polylines(fp, room_id):

    # prepare the array to return
    polyline_arr = []

    # compute the oriented boundary
    boundary_list = fp.compute_room_oriented_boundary(room_id)
    
    # iterate over each boundary
    for bi in range(len(boundary_list)):
        
        # prepare the next polyline
        pl = dxf.polyline()

        # make the array of vertices
        verts = [ fp.verts[vi] for vi in boundary_list[bi] ]
        pl.add_vertices(verts)
        pl.close()

        # make the polyline
        polyline_arr.append(pl)

    # return the final list
    return polyline_arr

#----------------------------------------------------------------
#----------------------------------------------------------------
#----------------------------------------------------------------

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

