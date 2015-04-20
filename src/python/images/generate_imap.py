#
# Driver functions to run the image generation code
#
import backpackconfig
import sys
from os import *
import glob

#
# the main run function
#
# This code can only be run after the normal 
# and depth maps have been generated
# 
def run(settings) :

    # The first thing we need to do is import the 
        # backpack configuration file
    # so that we can deduce the locations of all the files names
    confFile = path.join(settings['datasetDir'],
        "config","backpack_config.xml")
    conf = backpackconfig.Configuration(confFile, True, \
                settings['datasetDir']);

    # Create the image maps dir
    imagemapdir = path.join(settings['datasetDir'],
        'imagemaps')

    # Then we need to get a list of all active cameras for this dataset
    cameras = conf.find_sensors_by_type("cameras")
    code_args = []

    for camera in cameras :
        # We need to create first the camera pose file
        poseFiles = glob.glob(path.join(settings['datasetDir'], \
            'localization',settings['datasetName'], \
            'cameraposes',camera+"_poses.txt"))
        if(len(poseFiles) == 0) :
            print ("No camera pose file for " \
                 + camera + " found in " \
                 + path.join(settings['datasetDir'], \
                    'localization', \
                    settings['datasetName'], \
                    'cameraposes'))
            return 1;
        poseFile = poseFiles[0]

        # Then we need to create the names of the depth 
        # and normal files
        depthfile = path.join(imagemapdir, camera, 
            "depthmaps", camera+"_imagelog.txt")
        normalfile = path.join(imagemapdir, camera, 
            "normalmaps", camera+"_imagelog.txt")

        code_args.append(tuple([poseFile, depthfile, normalfile]))

    # Create the binary file name
    binFile = path.join(settings['binaryDir'],'create_image_mapping')

    # Create the output file name
    imapfile = path.join(imagemapdir,
        "imagemap.imap")
    keyfile = path.join(imagemapdir,
        "imgids.txt")

    args = [binFile,
        "-dir", settings['datasetDir'],
        "-o", imapfile, keyfile,
        "-r", str(settings['resolution'])]
    for argset in code_args :
        args += ["-i"]
        args += argset[:]

    # Run the code
    ret = system(" ".join(args))
    if ret != 0:
        print "Image mapping returned error : " + str(ret)

    # Return success
    return ret
