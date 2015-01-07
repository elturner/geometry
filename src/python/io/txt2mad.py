##
#   @file     txt2mad.py
#   @author   Eric Turner <elturner@eecs.berkeley.edu>
#   @brief    Convert from text files to mad files
#
#
#   Will read in a given .txt file, where each line represents a pose
#   in 3D space as follows:
#
#	<x> <y> <z> <timestamp>
#
#   These values will be converted into a mad file.
#

import argparse
from madformat import Mad

#
#   This is the main function
#
def main() :

    # Parse the args
    args = handle_args()

    # Open the txt file
    infile = open(args.input_file[0])

    # init path
    path = Mad()
    path.clear()
    
    # init values
    header_lines = args.header_lines
    units = args.units_scale[0]
    num_ignored = 0
    
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
        if len(invals) < 4 :
            # ignore it
            num_ignored += 1
            continue
       
        # get the (x,y,z) position
        x = float(invals[0]) * units
        y = float(invals[1]) * units
        z = float(invals[2]) * units
        timestamp = float(invals[3])
        timestamp = float(invals[3])

        # add to path
	path.poses.append([x, y, z, 0, 0, 0])
	path.times.append(timestamp)
	
    # export the path
    path.write_mad(args.output_file[0])
    infile.close()

    # Done
    print
    print "Exported %d poses" % len(path.poses)
    print "Ignored %d lines" % num_ignored
    print

#
#   I handle args
#
def handle_args() :
   
    # Create the argument parser
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_file", 
        required=True, 
        help=("The input txt file that stores pose info."),
        nargs=1,
        type=str)
    parser.add_argument("-o","--output_file",
        required=True,
        help=("This is where the code writes the .mad file"),
        nargs=1,
        type=str)
    parser.add_argument("-u","--units_scale",
        required=False,
        help=("The scale-factor for the spatial input units in " \
                        "order to convert to the output file " \
                        "(meters)."),
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


