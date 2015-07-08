#!/usr/bin/python

##
# @file render_mad.py
# @author Eric Turner <elturner@indoorreality.edu>
# @brief  Will render the path from a .mad file
#
# @section DESCRIPTION
#
# Will read the given .mad file, parsing the path information
# will open a window showing the path
#

# import matlibplot
from matplotlib import pyplot
import os
import sys

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'io'))
from madformat import Mad

# ------------------------------------ #
# ------ Function Implentations ------ #
# ------------------------------------ #

##
# Will plot the contents of the specified mad file
#
# @param input_file   The input .mad file to parse
#
def run(input_file):

    # read the file
    mad = Mad(input_file)

    # open a figure
    pyplot.figure(1)

    # plot the path
    x = [mad.poses[i][0] for i in range(len(mad.poses))]
    y = [mad.poses[i][1] for i in range(len(mad.poses))]
    pyplot.plot(x, y)
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
        print "\t",sys.argv[0],"<path_to_mad_file>"
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
