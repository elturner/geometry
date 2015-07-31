#include "merge_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   merge_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for merge_fp_oct program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * merge_fp_oct program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FLAG        "-s" /* program-specific settings (.xml) */
#define INPUT_OCTFILE_FLAG   "-i" /* location of input octree file (.oct) */
#define INPUT_CHUNKLIST_FLAG "-l" /* flag for input chunklist file */
#define INPUT_WEDGE_FLAG     "-w" /* flag for input wedge file */
#define INPUT_CARVEMAP_FLAG  "-m" /* flag for input carvemap file */
#define OUTPUT_OCTFILE_FLAG  "-o" /* flag for output octree file (.oct) */

/* file extensions */

#define FP_FILE_EXT         "fp"

/* xml settings flags */

#define XML_OBJECT_REFINE_DEPTH "object_refine_depth"
#define XML_INTERPOLATE_TAG     "procarve_interpolate"

/* function implementations */
		
merge_run_settings_t::merge_run_settings_t()
{
	/* set default values for this program's files */
	this->input_octfile       = "";
	this->input_chunklistfile = "";
	this->input_wedgefile     = "";
	this->input_carvemapfile  = "";
	this->output_octfile      = "";
	this->fpfiles.clear();
	this->object_refine_depth = 0;
	this->interpolate         = true;
}

int merge_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program optimizes the geometry "
			"of generated floorplans for a given dataset by "
			"aligning their surfaces with the carvings "
			"described in the given octree file.");
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to adjust the optimization algorithm.", false, 1);
	args.add(INPUT_OCTFILE_FLAG, "The input octree (.oct) file to "
			"parse.  This file represents the probabilistic "
			"carving of the dataset in the form of an octree.",
			false, 1);
	args.add(INPUT_CHUNKLIST_FLAG, "The input chunk list (.chunklist) "
			"file to parse.  This file represents a list of "
			"chunks in the carved environment, which are a "
			"spatial separation of the data into volumetric "
			"cubes that can be processed independently.", false,
			1);
	args.add(INPUT_WEDGE_FLAG, "The input wedge (.wedge) file to parse."
			"  This file denotes a list of carving wedges, "
			"which reference volumes to carve in the octree.",
			false, 1);
	args.add(INPUT_CARVEMAP_FLAG, "The input carve map (.carvemap) file"
			" to parse.  This file represents the statistical "
			"info for the raw scan points.  It is referenced "
			"by the wedge file in order to interpolate between "
			"scans.", false, 1);
	args.add(OUTPUT_OCTFILE_FLAG, "The output octree (.oct) file to "
			"export.  This file will contain the same info "
			"as the input, but with the updated floorplan room "
			"identifiers.  This file is allowed to be the same "
			"as the input, in which case the file is just "
			"overwritten with the new information.", false, 1);
	args.add_required_file_type(FP_FILE_EXT, 1, "The floorplan files.  "
			"These files specify the geometry of the floorplans"
			" to merge with the octree.  They will be parsed "
			"and incorporated with the octree file.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[merge_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* get file paths */
	this->input_octfile       = args.get_val(INPUT_OCTFILE_FLAG);
	this->output_octfile      = args.get_val(OUTPUT_OCTFILE_FLAG);
	this->input_chunklistfile = args.get_val(INPUT_CHUNKLIST_FLAG);
	this->input_wedgefile     = args.get_val(INPUT_WEDGE_FLAG);
	this->input_carvemapfile  = args.get_val(INPUT_CARVEMAP_FLAG);
	args.files_of_type(FP_FILE_EXT, this->fpfiles);
	
	/* retrieve the specified settings file */
	settings_file = args.get_val(SETTINGS_FLAG);
	
	/* attempt to open and parse the settings file */
	if(!settings.read(settings_file))
	{
		/* unable to open or parse settings file.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[merge_run_settings_t::parse]\t"
		     << "Error " << ret << ":  Unable to parse "
		     << "settings file: " << settings_file << endl;
		return ret;
	}
	
	/* read in settings from file.  If they are not in the given
	 * file, then the default settings that were set in this
	 * object's constructor will be used. */

	/* get object refinement depth */
	if(!(settings.is_prop(XML_OBJECT_REFINE_DEPTH)))
	{
		/* inform user of missing field */
		cerr << "[merge_run_settings_t::parse]\tERROR! "
		     << "The input settings .xml file is missing "
		     << "\"" << XML_OBJECT_REFINE_DEPTH << "\" field!"
		     << endl;
		return -3;
	}
	this->object_refine_depth 
		= settings.getAsUint(XML_OBJECT_REFINE_DEPTH);
	if(settings.is_prop(XML_INTERPOLATE_TAG))
		this->interpolate
			= (settings.getAsUint(XML_INTERPOLATE_TAG) != 0);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
