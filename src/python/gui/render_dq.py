#!/usr/bin/python

##
# @file render_dq.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Will render the wall samples from a .dq file
#
# @section DESCRIPTION
#
# Will read the given .dq file, parsing the wall sample information
# will open a window showing the points
#

# import matlibplot
from matplotlib import pyplot
import os
import sys

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from dq_io import WallSamples

# ------------------------------------ #
# ------ Function Implentations ------ #
# ------------------------------------ #

##
# Will plot the contents of the specified dq file
#
# @param input_file   The input .dq file to parse
#
def run(input_file):

    # read the file
    dq = WallSamples(input_file)

    # open a figure
    pyplot.figure(1)

    # plot the wall samples
    pyplot.scatter(dq.samples_x, dq.samples_y)
    pyplot.axis('equal')

    # render it
    pyplot.show()


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
        print "\t",sys.argv[0],"<path_to_dq_file>"
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
