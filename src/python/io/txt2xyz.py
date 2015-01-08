##
#   @file     txt2xyz.py
#   @author   Eric Turner <elturner@eecs.berkeley.edu>
#   @brief    Convert from arbitrary-formatted text files to xyz files
#
#
#   Will read in a given .txt file, where each line represents a point
#   in 3D space.  It will export a .xyz file, where each line is formatted
#   as follows:
#
#       <x> <y> <z> <red> <green> <blue> <index> <timestamp> <serial>
#
#   Where (x,y,z) are in units of millimeters, (r,g,b) is in range [0,255]
#

import argparse

#
#   This is the main function
#
def main() :

    # Parse the args
    args = handle_args()

    # Open the txt file
    infile = open(args.input_file[0])

    # open the output file
    outfile = open(args.output_pointcloud[0], 'w')

    # init values
    header_lines = args.header_lines
    units = args.units_scale
    num_points = 0
    num_ignored = 0
    vals_per_line = -1
    
    # Iterate
    for line in infile :

        # check if we should ignore line
        if header_lines > 0 :
            header_lines -= 1
            num_ignored += 1
            continue

        # parse this line
        invals = line.split(" ")

        # check if invalid line
        if len(invals) < 3 :
            # ignore it
            num_ignored += 1
            continue
       
        # get the (x,y,z) position
        x = float(invals[0]) * units
        y = float(invals[1]) * units
        z = float(invals[2]) * units
        r = 0 # default red
        g = 0 # default green
        b = 0 # default blue
        index = 0 # default index
        timestamp = 0 # default timestmap
        serial = 0 # default serial
        num_points += 1 

        # determine how many additional values are in a line
        if len(invals) == 4 :
            # assume <x> <y> <z> <timestamp>
            timestamp = float(invals[3])
            index = int(timestamp)

            # check if this is the first line of this type
            if vals_per_line < 0:
                print
                print "Assuming format: <x> <y> <z> <timestamp>"
                print
        elif len(invals) == 6 :
            # assume <x> <y> <z> <r> <g> <b>
            r = int(invals[3])
            g = int(invals[4])
            b = int(invals[5])

            # check if this is the first line of this type
            if vals_per_line < 0:
                print
                print "Assuming format: <x> <y> <z> <red> <green> <blue>"
                print
        elif len(invals) == 6 :
            # assume <x> <y> <z> <r> <g> <b> <timestamp>
            r = int(invals[3])
            g = int(invals[4])
            b = int(invals[5])
            timestamp = float(invals[6])
            index = int(timestamp)

            # check if this is the first line of this type
            if vals_per_line < 0:
                print
                print "Assuming format: <x> <y> <z> " \
                        "<red> <green> <blue> <timestamp>"
                print
        elif len(invals) == 9 :
            # assume <x> <y> <z> <r> <g> <b> <index> <timestamp> <serial>
            r = int(invals[3])
            g = int(invals[4])
            b = int(invals[5])
            index = int(invals[6])
            timestamp = float(invals[7])
            serial = float(invals[8])

            # check if this is the first line of this type
            if vals_per_line < 0:
                print
                print "Assuming format: <x> <y> <z> " \
                        "<red> <green> <blue> <index> " \
                        "<timestamp> <serial>"
                print
 
        # record that we've seen this format
        vals_per_line = len(invals)

        # prepare output
        outvals = [str(x), str(y), str(z), \
                    str(r), str(g), str(b), \
                    str(index), str(timestamp), str(serial)]

        # write it
        outfile.write(" ".join(outvals))
        outfile.write("\n")

        # print status
        if num_points % 1000000 == 0 :
            print "Exported %d points so far..." % num_points

    # Done
    print
    print "Exported %d points" % num_points
    print "Ignored %d lines" % num_ignored
    print
    outfile.close()
    infile.close()

#
#   I handle args
#
def handle_args() :
   
    # Create the argument parser
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_file", 
        required=True, 
        help=("The input txt file that stores points."),
        nargs=1,
        type=str)
    parser.add_argument("-o","--output_pointcloud",
        required=True,
        help=("This is where the code places the pointcloud."),
        nargs=1,
        type=str)
    parser.add_argument("-u","--units_scale",
        required=False,
        help=("The scale-factor for the spatial input units in " \
                        "order to convert to the output file " \
                        "(millimeters)."),
        nargs=1,
        default=1.0,
        type=float)
    parser.add_argument("-H","--header_lines",
        required=False,
        help=("The number of header lines to ignore in input file."),
        nargs=1,
        default=0,
        type=int)

    # Do the parsing
    args = parser.parse_args()

    return args;

#
#   Plate of boiler
#
if __name__ == "__main__" :
   main()


