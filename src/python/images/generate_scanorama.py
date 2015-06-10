#!/usr/bin/python

##
# @file generate_scanorama.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief The pointcloud generation python script
# 
# @section DESCRIPTION
#
# This script will call the generate_scanorama
# executable after verifying that all input files
# exist, output directories exist, and the given
# dataset is formatted appropriately.
#

# import python libraries
import os
import sys
import shutil
import subprocess

#-------------------------------------
#---------- CONSTANTS ----------------
#-------------------------------------

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# import local code
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import backpackconfig
import dataset_filepaths

# store path to executable
SCANORAMA_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, "..", \
        "..", "..", "bin",  "generate_scanorama"))
if sys.platform == 'win32' :
    SCANORAMA_EXE += '.exe'

# store path to the settings xml file
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
        '..', '..', '..', 'config', 'scanorama', 'scanorama_settings.xml'))

#----------------------------------------------------
#---------- FUNCTION IMPLEMENTATIONS ----------------
#----------------------------------------------------

##
# This function will generate a set of scanoramas for a dataset
#
# This will run the code, but will first check
# that all files exist and the output can be written.  It will also
# check what sensors are available in the dataset, and only use
# existing sensor data.
#
# @param dataset_dir   Path to dataset directory
# @param pathfile      Path to 3D .mad path file
#
def run(dataset_dir, pathfile):

    # check that executable exists
    if not os.path.exists(SCANORAMA_EXE):
        print "Error!  Could not find pointcloud", \
            "generation executable:", \
            SCANORAMA_EXE
        return None

    # verify that output directory exists
    scanoramadir = dataset_filepaths.get_scanorama_dir(dataset_dir)
    if not os.path.exists(scanoramadir):
        os.makedirs(scanoramadir)

    # get necessary files for this dataset
    scanoprefix = dataset_filepaths.get_scanorama_ptx_prefix(dataset_dir)
    modelfile = dataset_filepaths.get_full_mesh_file(dataset_dir)
    config_xml = dataset_filepaths.get_hardware_config_xml(dataset_dir)
    conf = backpackconfig.Configuration(config_xml, True, dataset_dir)
    
    # find all active cameras in the file
    cam_metas  = []
    cam_calibs = []
    cam_dirs   = []
    cameraList =  conf.find_sensors_by_type("cameras")
    for cameraName in cameraList:

        # prepare the command-line arguments for this camera
        metafile = os.path.join(dataset_dir, \
                conf.find_sensor_prop(cameraName, \
                    'configFile/dalsa_metadata_outfile', 'cameras'))
        metafile = dataset_filepaths.get_color_metadata_file( \
                cameraName, metafile)
        calibfile = os.path.join(dataset_dir, \
                conf.find_sensor_prop(cameraName, \
                    'configFile/dalsa_fisheye_calibration_file', 'cameras'))
        imgdir = os.path.join(dataset_dir, \
                dataset_filepaths.get_color_image_dir( \
                    conf.find_sensor_prop(cameraName, \
                        'configFile/dalsa_output_directory', 'cameras')))

        # add to arguments lists
        cam_metas.append(metafile)
        cam_calibs.append(calibfile)
        cam_dirs.append(imgdir)

    # Prepare arguments for program
    args = [SCANORAMA_EXE, '-c', config_xml, '-s', SETTINGS_XML, \
                '-m', modelfile, \
                '-p', os.path.abspath(pathfile), \
                '-o', scanoprefix]

    # add camera information
    for ci in range(len(cam_metas)):
        args += ['-f', cam_metas[ci], \
                cam_calibs[ci], cam_dirs[ci]]

    # run pointcloud generation program
    ret = subprocess.call(args, executable=SCANORAMA_EXE, \
            cwd=dataset_dir, stdout=None, stderr=None, \
            stdin=None, shell=False)
    if ret != 0:
        print "Error! Scanorama generation program", \
                "returned",ret
        return ret

    # success
    return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

    # check command-line arguments
    if len(sys.argv) != 3:
        print ""
        print " Usage:"
        print ""
        print "\t",sys.argv[0],"<path_to_dataset>", \
            "<madfile>"
        print ""
        sys.exit(1)

    # run this script with the given arguments
    ret = run(sys.argv[1],sys.argv[2])
    
    # display results
    print ""
    if ret != 0:
        sys.exit(1)

    # success
    sys.exit(0)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
    main()
