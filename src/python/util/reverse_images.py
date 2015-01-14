#!/usr/bin/python

##
# @file    reverse_images.py
# @author  Eric Turner <elturner@eecs.berkeley.edu>
#
# Makes a copy of a folder of images, flipping their ordering
#

import sys
import os
import shutil

# check args
if len(sys.argv) != 3:
    print "Usage:"
    print
    print "\t",sys.argv[0],"<infolder> <outfolder>"
    print
    print "Given a folder containing '.jpg' files, copies the"
    print "the images from the <infolder> to the <outfolder>,"
    print "renaming the images so they will be indexed in the"
    print "reversed ordering from the original."
    print
    print "This process is useful when generating videos from"
    print "a set of images, and you want to reverse the video"
    print
    sys.exit(0)

# get args
infolder = sys.argv[1]
outfolder = sys.argv[2]
n = 0

# iterate through input files
for f in reversed(sorted(os.listdir(infolder))):
    
    # check if this is an image file
    if not f.endswith('.jpg') or f[0] == '.':
        continue

    # get number
    n += 1
    out = ('img_' + str(n) + '.jpg')

    # copy the file
    print "moving: %s => %s" % (f, out)
    shutil.copyfile(os.path.join(infolder, f), \
            os.path.join(outfolder, out))




