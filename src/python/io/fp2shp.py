#!/usr/bin/python

##
# @file   fp2shp.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Exports floorplan to .shp file

import sys
import fp_io
import shapefile

##
# This function converts a .fp file to a .shp file
#
# @param fpfile   The input .fp file
# @param shpfile  The output .shp file
#
def run(fpfile, shpfile):
    
    # read the input floorplan
    fp = fp_io.Floorplan(fpfile)

    # generate the oriented boundary
    boundary_list = fp.compute_oriented_boundary()

    # make input for shapefile library
    vert_lists = []
    for b in boundary_list:
        vert_lists.append([])
        for v in b:
            vert_lists[-1].append(fp.verts[v])
        verts_list[-1].append(fp.verts[b[0]]) # put end point twice

    # pass it to the library
    w = shapefile.Writer(shapefile.POLYGON)
    w.poly(parts=vert_lists)
    w.field('FIRST_FLD','C',40);
    w.record('First','Polygon')
    w.save(shpfile)

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
        print "\t",sys.argv[0],"<fpfile> <shpfile>"
        print ""
        print " Where:"
        print ""
        print "\t<fpfile>    Input floorplan file to read"
        print "\t<shpfile>   Output .shp file to generate"
        print ""
        sys.exit(1)

    # get values
    fpfile = sys.argv[1]
    shpfile = sys.argv[2]

    # run it
    run(fpfile, shpfile)
    sys.exit(0)

##
# Boilerplate code to call main function as executable
#
if __name__ == '__main__':
    main()


