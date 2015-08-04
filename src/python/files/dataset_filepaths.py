#!/usr/bin/python

##
# @file   dataset_filepaths.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Stores expected locations of files in datasets
# 
# @section DESCRIPTION
#
# This file contains functions that return the expected
# file locations of various dataset files when given
# the path to a dataset.
#

# Get the location of this file
import os
import sys
SCRIPT_LOCATION = os.path.dirname(__file__)

# import libraries
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
import config

#---------------- Generic file functions ------------------------

##
# Returns the body name of the given file
#
def get_file_body(f):
	(head, tail) = os.path.split(f)
	(body, ext) = os.path.splitext(tail)
	return body

#---------------- Files within a dataset ------------------------

##
# Returns the expected location of the hardware config xml file
#
def get_hardware_config_xml(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"config","backpack_config.xml"))

##
# Returns the expected location of the timestamp sync xml file
#
def get_timesync_xml(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, \
		"time", "time_sync.xml"))

#--------------- Sensor-specific files ------------------------

##
# Returns the location of the .fss file generated from the given
# range sensor .dat file
#
def get_fss_file(dat_file):
	name, ext = os.path.splitext(dat_file)
	fss_file = name + ".fss"
	return fss_file

##
# Returns the expected location of the .fss files in this dataset as a list
#
# Note:  this does not include d-imager-generated fss files, but only those
# generated from laser scanners.
#
def get_all_fss_files(dataset_dir, whitelist=None):

	# get the config xml location
	config_xml = get_hardware_config_xml(dataset_dir)

	# parse the hardware configuration file
	sensor_types, sensor_cfn, sensor_names = \
			config.parse_backpack_xml(config_xml)
	if sensor_types is None or sensor_cfn is None \
			or len(sensor_types) != len(sensor_cfn):
		return None # could not parse the xml file

	# find the scan files to use as input
	fssfiles = []
	for si in range(len(sensor_types)):	

		# check for laser scanners
		if sensor_types[si] == 'laser':

			# check if a whitelist is specified
			if whitelist != None \
				and sensor_names[si] not in whitelist:
				continue

			# parse the settings file for this laser
			urg_settings = config.parse_settings_xml( \
					os.path.normpath(os.path.join( \
					dataset_dir, sensor_cfn[si])))
			if urg_settings is None:
				return None # unable to parse settings

			# the fss file is named the same as the raw
			# data file, but with a different file ext
			urg_filename = urg_settings["urg_datafile"]
			urg_outname,ext = os.path.splitext(urg_filename)
			urg_outname += '.fss'

			# add to our list
			fssfiles.append(urg_outname)
		
                # check for tof scanners as well
		elif sensor_types[si] == 'tof_camera':

			# check if a whitelist is specified
			if whitelist != None \
				and sensor_names[si] not in whitelist:
				continue

			# parse the settings file for this sensor
			tof_settings = config.parse_settings_xml( \
					os.path.normpath(os.path.join( \
					dataset_dir, sensor_cfn[si])))
			if tof_settings is None:
				return None # unable to parse settings

			# the fss file is named the same as the raw
			# data file, but with a different file ext
			tof_filename = tof_settings["tof_datafile"]
			tof_outname,ext = os.path.splitext(tof_filename)
			tof_outname += '.fss'

			# add to our list
			fssfiles.append(tof_outname)

	# return the final list of fss files
	return fssfiles

#----------------------- Imagery File Paths ----------------------

##
# Retrieves location of camera color metadata file
#
# @param cam_name      The name of this camera
# @param cam_metafile  The location of the bayer metadata file for camera
#
def get_color_metadata_file(cam_name, cam_metafile):
	return os.path.join(os.path.dirname(cam_metafile), \
		"color_" + cam_name + "_metadata.txt")

##
# Retrieves location of camera color image directory
#
# @param bayerdir   The location of directory that contains bayer images
#
def get_color_image_dir(bayerdir):
	return os.path.normpath(os.path.join(bayerdir, "..", "color"))

##
# Retrieves location of directory that contains masks for the camera imagery
#
# @param dataset_dir   The location of the root of this dataset
#
def get_camera_masks_dir(dataset_dir):
	return os.path.normpath(os.path.join(dataset_dir, \
                'calib', 'camera', 'masks'))

##
# Retrieves location of FlIR normalized metadata file
#
# @param cam_name      The name of this camera
# @param cam_metafile  The location of the raw metadata file for camera
#
def get_ir_normalized_metadata_file(cam_name, cam_metafile):
	return os.path.join(os.path.dirname(cam_metafile), \
		"normalized_" + cam_name + "_metadata.txt")

##
# Retrieves location of FLIR normalized image directory
#
# @param bayerdir   The location of directory that contains normalized IR
#
def get_ir_normalized_image_dir(rawirdir):
	return os.path.normpath(os.path.join(rawirdir, "..", "normalized"))

#--------------- Interfacing with Localization Output --------------

##
# Returns location of the final .mad file given dataset name
#
# @param dataset_dir   The location of the root of this dataset
# @param dataset_name  The human-readable name of this dataset
#
def get_madfile_from_name(dataset_dir, dataset_name):
	# include the optimization tag in name if not already present
	if not dataset_name.endswith("_aligned"):
		dataset_name_aligned = dataset_name + "_aligned"
	else:
		dataset_name_aligned = dataset_name
		dataset_name = dataset_name[:-8] # remove _aligned

	# return the reconstructed filepath to the mad file
	return os.path.abspath(os.path.join(dataset_dir,"localization", \
			dataset_name, "output", \
			(dataset_name_aligned + ".mad")))

##
# Returns the name of the dataset given a .mad file
#
# @param madfile  The file to parse
#
def get_name_from_madfile(madfile):
	name = get_file_body(madfile)
	if name.endswith("_aligned"):
		name = name[:-8]
	return name

#--------------- Files generated from pointcloud code --------------

##
# Returns the location of the models directory
#
def get_models_dir(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir,"models"))

##
# Returns the location of the pointclouds directory
#
def get_pointcloud_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"pointclouds")

##
# Returns the location of the pointcloud levels directory
#
def get_pc_levels_dir(dataset_dir):
	return os.path.join(get_pointcloud_dir(dataset_dir),"levels")

##
# Returns the location of the colored pointclouds directory
#
def get_colored_pc_dir(dataset_dir):
	return os.path.join(get_pointcloud_dir(dataset_dir),"colored")

##
# Returns the prefix path for all the partioned levels pointclouds
#
# @param dataset_dir   Location of this dataset root
# @param name          Human-readable name of dataset
#
def get_pc_levels_prefix(dataset_dir, name):
	return os.path.join(get_pc_levels_dir(dataset_dir), \
		name + "_level_")

##
# Returns location of matlab histogram file from partitioning pointclouds
#
# @param dataset_dir   Location of this dataset root
# @param name          Human-readable name of dataset
#
def get_pc_levels_hist(dataset_dir, name):
	return os.path.join(get_pc_levels_dir(dataset_dir), \
		name + "_hist.m")

##
# Returns a list of the pointcloud files in the pointcloud directory
#
def get_all_pointcloud_files(dataset_dir):
	# get the pointcloud levels directory
	pcdir = get_pointcloud_dir(dataset_dir)

	# iterate through files in this directory
	pcfiles = []
	for f in os.listdir(pcdir):
		(body, ext) = os.path.splitext(f)
		if ext == '.xyz':
			pcfiles.append(os.path.join(pcdir, f))

	# return final list of pointcloud files
	return pcfiles

##
# Returns a list of the pointcloud files in the levels directory
#
def get_pc_levels_list(dataset_dir):
	# get the pointcloud levels directory
	levdir = get_pc_levels_dir(dataset_dir)

	# iterate through files in this directory
	pcfiles = []
	for f in os.listdir(levdir):
		(body, ext) = os.path.splitext(f)
		if ext == '.xyz':
			pcfiles.append(os.path.join(levdir, f))

	# return final list of pointcloud files
	return pcfiles

#------------- Files generated from surface_carve code -------------

##
# Returns the location of the surface carve directory
#
def get_surface_carve_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"surface_carve")

##
# Returns the location of the surface carving .vox file
#
# @param dataset_dir   The root directory of the dataset
# @param madfile       The .mad file to use
#
def get_surface_carve_vox(dataset_dir, madfile):
	return os.path.join(get_surface_carve_dir(dataset_dir), \
	                    get_name_from_madfile(madfile) + ".vox")

##
# Returns the location of the surface carving .ply file
#
# @param dataset_dir   The root directory of the dataset
# @param madfile       The .mad file to use
#
def get_surface_carve_ply(dataset_dir, madfile):
	return os.path.join(get_surface_carve_dir(dataset_dir), \
	                    get_name_from_madfile(madfile) + ".ply")

#--------------- Files generated from floorplan code ---------------

##
# Returns the location of the floorplan directory
#
def get_floorplan_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"floorplan")

##
# Returns the location of the pseudo-texture directory 
#
def get_faketexture_dir(dataset_dir):
	return os.path.join(get_floorplan_dir(dataset_dir),"texture")

##
# Returns the wall sampling file that corresponds with the given pointcloud
#
# @param dataset_dir   The location of the dataset directory
# @param pc_file       The location of the pointcloud file
#
def get_dq_file(dataset_dir, pc_file):
	return os.path.join(get_floorplan_dir(dataset_dir), \
	                    get_file_body(pc_file) + ".dq")

##
# Returns the fp file that corresponds with the input dq file
#
def get_fp_file(dq_file):
	(body, ext) = os.path.splitext(dq_file)
	return (body + ".fp")

##
# Returns the output .csv file for a floorplan given the input fp file
#
def get_csv_floorplan_file(fp_file):
	(body, ext) = os.path.splitext(fp_file)
	return (body + ".csv")

##
# Returns the floorplan mesh (obj) file, given the fp
#
def get_floorplan_obj_file(dataset_dir, fp_file):
	return os.path.join(get_faketexture_dir(dataset_dir), \
		get_file_body(fp_file) + ".obj")

##
# Returns the floorplan mesh (ply) file, given the fp
#
def get_floorplan_ply_file(dataset_dir, fp_file):
	return os.path.join(get_floorplan_dir(dataset_dir), \
		get_file_body(fp_file) + ".ply")

##
# Returns all floorplan files in the floorplans directory
#
def get_all_floorplan_files(dataset_dir):
	
	# get the floorplan directory
	fpdir = get_floorplan_dir(dataset_dir)
	
	# get all fp files in this directory
	fpfiles = [os.path.join(fpdir, f) \
			for f in os.listdir(fpdir) \
				if f.endswith(".fp")]
	return fpfiles

#----------- Files generated for LaTeX info file -----------------

##
# Returns the location of the docs directory for a dataset
#
def get_docs_dir(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, "docs"))

##
# Returns the path to the .tex file in the docs directory
#
# @param dataset_dir    The root folder of the dataset
# @param dataset_name   The human-readable name of this dataset
#
def get_tex_file(dataset_dir, dataset_name):
	return os.path.join(get_docs_dir(dataset_dir), \
			(dataset_name + ".tex"))

#--------------- Files generated from procarve suite --------------------

##
# Returns the location of the carving directory
#
def get_carving_dir(dataset_dir):
	return os.path.join(get_models_dir(dataset_dir),"carving")

##
# Returns location of .noisypath file of this dataset
#
# @param dataset_dir   The location of the root of this dataset
#
def get_noisypath_file(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), \
			"path.noisypath")

##
# Returns the location of the carvemap file in a dataset
#
def get_carvemap_file(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), \
			"scandists.carvemap")

##
# Returns the location of the wedge file in a dataset
#
def get_wedgefile(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), \
			"scandists.wedge")

##
# Returns the expected location of the chunklist file
#
def get_chunklist(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir),"cache.chunklist")
##
# Returns the expected location of the carved octree file
#
def get_octree(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), "carving.oct")

#----------- Files generated from oct -> floorplan -------------------

##
# Returns the directory containing floorplan files generated from octrees
#
def get_carving_fp_dir(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), "fp")

##
# Returns the .levels file generated from an octre by oct2dq
#
def get_carving_levels_file(dataset_dir):
	return os.path.join(get_carving_fp_dir(dataset_dir), \
			"building.levels")

##
# Returns the prefix used for the carving-dq wall sampling files
#
def get_carving_dq_prefix(dataset_dir):
	return os.path.join(get_carving_fp_dir(dataset_dir), "level_")

##
# Returns all the dq files generated from an octree
#
def get_carving_dq_files(dataset_dir):
	
	# get the floorplan directory
	fpdir = get_carving_fp_dir(dataset_dir)
	
	# get all dq files in this directory
	dqfiles = [os.path.join(fpdir, f) \
			for f in os.listdir(fpdir) \
				if f.endswith(".dq")]
	return dqfiles

##
# Returns all the floorplan files generated from an octree
#
def get_carving_fp_files(dataset_dir):
	
	# get the floorplan directory
	fpdir = get_carving_fp_dir(dataset_dir)
	
	# get all fp files in this directory
	fpfiles = [os.path.join(fpdir, f) \
			for f in os.listdir(fpdir) \
				if f.endswith(".fp")]
	return fpfiles

#--------------- Files generated from merging ------------------------

##
# Returns the directory containing merged geometry information
#
def get_merged_dir(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), "merged")

##
# Returns the directory containing aligned floorplan files
#
def get_aligned_fp_dir(dataset_dir):
	return os.path.join(get_merged_dir(dataset_dir), "aligned_fp")

##
# Returns all aligned floorplan files in the aligned_fp directory
#
def get_aligned_fp_files(dataset_dir):
	
	# get the floorplan directory
	fpdir = get_aligned_fp_dir(dataset_dir)
	
	# get all fp files in this directory
	fpfiles = [os.path.join(fpdir, f) \
			for f in os.listdir(fpdir) \
				if f.endswith(".fp")]
	return fpfiles

##
# Returns location of refined octree file.  This file contains fp info
#
def get_refined_octree(dataset_dir):
	return os.path.join(get_merged_dir(dataset_dir), "refined.oct")

#--------------- File generated from meshing ----------------------

##
# Returns the directory containing all generated meshes and submeshes
#
def get_mesh_dir(dataset_dir):
	return os.path.join(get_carving_dir(dataset_dir), "mesh")

##
# Returns the expected location of the carved mesh file
#
def get_full_mesh_file(dataset_dir):
	return os.path.join(get_mesh_dir(dataset_dir), "mesh_all.ply")

##
# Returns the expected location of the mesh file that represents
# objects in the environment (such as furniture), but not room surfaces
#
def get_object_mesh_file(dataset_dir):
	return os.path.join(get_mesh_dir(dataset_dir), "mesh_objects.ply")

##
# Returns the expected location of the mesh file that represents
# room surfaces in the environment (floors, walls, ceilings, etc), but
# not objects.
#
def get_room_mesh_file(dataset_dir):
	return os.path.join(get_mesh_dir(dataset_dir), "mesh_room.ply")

#--------------- Files generated from scanorama -----------------

##
# Returns the directory containing all scanorama files
#
def get_scanorama_dir(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, "scanorama"))

##
# Returns the file prefix for scanorama files
#
# @param dataset_dir   The dataset directory path
#
def get_scanorama_ptx_prefix(dataset_dir):
	return os.path.join(get_scanorama_dir(dataset_dir), "scan_")

##
# Returns the location for scanorama metadata file (.scanolist)
#
# @param dataset_dir   The dataset directory path
#
def get_scanorama_metadata_file(dataset_dir):
	return os.path.join(get_scanorama_dir(dataset_dir), \
				"scan_metadata.scanolist")

#--------------- files generated for light detection ---------------

##
# The directory containing all detection data
#
# @param dataset_dir   The root dataset directory
#
def get_detection_dir(dataset_dir):
	return os.path.abspath(os.path.join(dataset_dir, "detection"))

##
# The directory containing light detection information
#
# @param dataset_dir   The root dataset directory
#
def get_light_detection_dir(dataset_dir):
	return os.path.join(get_detection_dir(dataset_dir), "lights")

##
# The directory containing light detection info
# for the given building level
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_dir(dataset_dir, level_index):
	return os.path.join(get_light_detection_dir(dataset_dir), \
			"level_" + str(level_index))

##
# Where the cropped pointcloud containing just the level ceiling is stored
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_ceiling_pc(dataset_dir, level_index):
	return os.path.join(get_light_detection_level_dir( \
			dataset_dir, level_index), \
			"level_" + str(level_index) + "_ceiling.xyz")

##
# Where the screenshot of the full level ceiling pointcloud is stored
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_ceiling_image(dataset_dir, level_index):
	return os.path.join(get_light_detection_level_dir( \
			dataset_dir, level_index), \
			"level_" + str(level_index) + "_ceiling.png")

##
# Where the coordinate mapping file associated with the screenshot is stored
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_ceiling_coordmap(dataset_dir, level_index):
	return os.path.join(get_light_detection_level_dir( \
		dataset_dir, level_index), \
		"level_" + str(level_index) + "_ceiling_coordmap.txt")

##
# Where the time mapping file associated with the screenshot is stored
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_ceiling_timemap(dataset_dir, level_index):
	return os.path.join(get_light_detection_level_dir( \
		dataset_dir, level_index), \
		"level_" + str(level_index) + "_ceiling_timemap.png")

##
# Folder where the room-specific pointcloud ceiling screenshots are stored
#
# @param dataset_dir   The root directory of the dataset
# @param level_index   The index of the level in question
#
def get_light_detection_level_ceiling_rooms(dataset_dir, level_index):
	return os.path.join(get_light_detection_level_dir( \
		dataset_dir, level_index), "rooms")
