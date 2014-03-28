#!/usr/bin/python

##
# @file floorplan.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the floorplan pipeline on a point-cloud
#
# @section DESCRIPTION
#
# Will call the floorplan generation pipeline on a point-cloud, 
# verifying that all input files exist, and that the output 
# directories are valid.
#

# Import standard python libraries
import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import dataset_filepaths

# the following indicates the expected location of the floorplan
# executable file
EXE_DIR = os.path.join(SCRIPT_LOCATION,'..','..','..','bin')
XYZ2DQ_EXE     = os.path.abspath(os.path.join(EXE_DIR,  "xyz2dq"))
FLOORPLAN_EXE  = os.path.abspath(os.path.join(EXE_DIR,  "floorplan_gen"))
FP2MODEL_EXE   = os.path.abspath(os.path.join(EXE_DIR,  "fp2model"))
FP2MODEL_TEXTURE_DIR = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
                       "..","..","..","execs","fp2model","texture"))

##
# The main function of this script, which is used to run the pipeline
#
# This call will run the floorplan program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
# @param path_file    The path to the 3D path file to use
# @param xyz_file     The path to the xyz point-cloud to use
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, path_file, xyz_file):

	# check that all required executables exist
	if not os.path.exists(XYZ2DQ_EXE):
		print "Error! Could not find wall sampling executable:", \
		      XYZ2DQ_EXE
		return -1
	if not os.path.exists(FLOORPLAN_EXE):
		print "Error!  Could not find floorplan executable:", \
		      FLOORPLAN_EXE
		return -2
	if not os.path.exists(FP2MODEL_EXE):
		print "Error!  Could not find fp2model executable:", \
		      FP2MODEL_EXE
		return -3
	
	# determine the expected location of necessary files from
	# within the dataset
	models_dir    = dataset_filepaths.get_models_dir(dataset_dir)
	floorplan_dir = dataset_filepaths.get_floorplan_dir(dataset_dir)
	texture_dir   = dataset_filepaths.get_faketexture_dir(dataset_dir)
	
	dq_file  = dataset_filepaths.get_dq_file(dataset_dir, xyz_file)
	fp_file  = dataset_filepaths.get_fp_file(dq_file)
	obj_file = dataset_filepaths.get_floorplan_obj_file(dataset_dir,\
	                                                    fp_file)

	# The following folders will contain the output files of this
	# script, so we need to verify that they exist
	if not os.path.exists(models_dir):
		os.makedirs(models_dir)
	if not os.path.exists(floorplan_dir):
		os.makedirs(floorplan_dir)
	if not os.path.exists(texture_dir):
		shutil.copytree(FP2MODEL_TEXTURE_DIR, texture_dir) 
	
	# generate wall samples
	print "##### generating wall samples from pointcloud data #####"
	ret = subprocess.call([XYZ2DQ_EXE, xyz_file, dq_file, path_file], \
			executable=XYZ2DQ_EXE, cwd=dataset_dir, \
			stdout=None, stderr=None, stdin=None, shell=False)
	if ret != 0:
		print "Error! Wall sampling program returned",ret
		return -4

	# make floorplan from wall samples
	print "##### generating floor plans #####"
	ret = subprocess.call([FLOORPLAN_EXE, \
                      dq_file, path_file, fp_file], \
                      executable=FLOORPLAN_EXE, cwd=dataset_dir, \
                      stdout=None, stderr=None, stdin=None, shell=False)
	if ret != 0:
		print "Error! Floorplan generation program returned",ret
		return -5

	# make obj from floorplan
	print "##### generating extruded model #####"
	ret = subprocess.call([FP2MODEL_EXE, \
                      obj_file, fp_file], \
                      executable=FP2MODEL_EXE, cwd=dataset_dir, \
                      stdout=None, stderr=None, stdin=None, shell=False)
	if ret != 0:
		print "Error! Floorplan->model program returned",ret
		return -6

	# success
	return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 4:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>",\
			"<localization_file>", "<xyz_file>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1], sys.argv[2], sys.argv[3])
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
