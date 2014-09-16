#!/usr/bin/python

##
# @file   imap_io.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  I/O functionality for image map files
#
# @section DESCRIPTION
#
# This file contains functions that will parse .imap (image map)
# files to allow for processing.
#
# Last modified on September 16, 2014 
#

##
# The Imagemap class holds information from the file
#
class Imagemap:
    'Class defines imagemap file information'

    ##
    # Constructor generates class from .imap file
    #
    # This class has the following fields:
    #
    #   res -       The resolution of the map (resolution)
    #   cells   -   A dictionary that goes from cell position to list 
    #               of files.
    #
    def __init__(self, input_file=None):

        # check arguments
        if input_file is None:
            self.res = -1.0 # invalid
            self.cells = {} # empty dictionary
        else:
            self.read(input_file)


    ##
    # Reads .imap file into this structure
    #
    # Will parse the specified .imap file
    #
    # @param input_file   The path to the input .imap file
    #
    def read(self, input_file):

        # attemp to open file to read
        with open(input_file) as f:
            content = f.read().splitlines()

        # check formatting
        if len(content) < 1:
            raise IOError("Empty .imap file: " + input_file)

        # parse the header
        self.res = float(content[0])

        # iterate over the remaining lines, importing
        # each cell
        for tline in content[1:]:
            # parse this line
            vals = tline.split(' ')

            # get center point for this cell
            cx       = float(vals[0])
            cy       = float(vals[1])

            # get number of images on this line
            num_vals = int(vals[2])

            # add the values to the cell
            for i in range(num_vals):
                self.add((cx,cy), vals[3+i])

    ##
    # Adds value to the specified cell
    #
    # Given a (x,y) position, will find
    # the corresponding cell, and add the
    # given value to that cell
    #
    # @param (x,y)  The position of the value
    # @param val    The value to add (a string)
    #
    def add(self, (x,y), val):

        # get the discrete cell from this position
        xi = floor(x / self.res)
        yi = floor(y / self.res)

        # add to cell map
        if (xi,yi) not in res.cells:
            # create empty list
            res.cells[(xi,yi)] = []

        # add this value
        res.cells[(xi,yi)].append(val)
    
    ##
    # Gets the values for a given cell position
    #
    # Given a continuous 2D position (x,y), will
    # retrieve the list of values in the cell
    #
    # @param (x,y)   The position to search
    #
    # @return        The list of values at this location
    def get(self, (x,y)):

        # get the discrete cell from this position
        xi = floor(x / self.res)
        yi = floor(y / self.res)

        try:
            # return the values for this cell
            return res.cells[(xi,yi)]
        except:
            return [] # nothing here



