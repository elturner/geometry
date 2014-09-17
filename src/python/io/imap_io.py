#!/usr/bin/python
from math import floor

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
    def __init__(self, map_file=None, key_file=None):

        self.res = -1.0 # invalid
        self.cells = {} # empty dictionary
        self.namemap = {}

        # check arguments
        if map_file != None:
            self.cells = {}
            sel.namemap = {}
            self.res = -1.0
            self.read(map_file, key_file)


    ##
    # Reads .imap file into this structure
    #
    # Will parse the specified .imap file
    #
    # @param map_file   The path to the input .imap file
    # @param key_file   Maps the keys in the map_file to readable names
    def read(self, map_file, key_file=None):

        # attemp to open file to read
        with open(map_file) as f:
            content = f.read().splitlines()

        # check formatting
        if len(content) < 1:
            raise IOError("Empty .imap file: " + map_file)

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

        # if given load the name file 
        if key_file != None :

            # Then we load the name file
            with open(key_file) as f:
                content = f.read().splitlines()

            # Check formatting 
            if len(content) < 1:
                raise IOError("Empty name file: " + key_file)

            # Interate over the lines inserting them
            for lines in content :

                # split the line
                vals = lines.split(' ')

                # insert into the map
                self.namemap[vals[0]] = vals[1]

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
        if (xi,yi) not in self.cells:
            # create empty list
            self.cells[(xi,yi)] = []

        # add this value
        self.cells[(xi,yi)].append(val)
    
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
        
        # Query the dictionary for all ids at this location
        try:
            ids =  self.cells[(xi,yi)]
        except:
            return [] # nothing here

        # Map them to string names
        names = []
        for thisid in ids :
            try :
                names.append((thisid,self.namemap[thisid]))
            except :
                names.append((thisid, ""))
        return names




