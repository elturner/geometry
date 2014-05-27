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

#define SETTINGS_FLAG       "-s" /* program-specific settings (.xml) */
#define INPUT_OCTFILE_FLAG  "-i" /* location of input octree file (.oct) */
#define OUTPUT_OCTFILE_FLAG "-o" /* location of output octree file (.oct) */

/* file extensions */

#define FP_FILE_EXT         "fp"

/* function implementations */
		
merge_run_settings_t::merge_run_settings_t()
{
	/* set default values for this program's files */
	this->input_octfile = "";
	this->output_octfile = "";
	this->fpfiles.clear();
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
			"to adjust the optimization algorithm.", true, 1);
	args.add(INPUT_OCTFILE_FLAG, "The input octree (.oct) file to "
			"parse.  This file represents the probabilistic "
			"carving of the dataset in the form of an octree.",
			false, 1);
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

	/* get oct files */
	this->input_octfile  = args.get_val(INPUT_OCTFILE_FLAG);
	this->output_octfile = args.get_val(OUTPUT_OCTFILE_FLAG);
	args.files_of_type(FP_FILE_EXT, this->fpfiles);
	
	/* check if a settings xml file was specified */
	if(args.tag_seen(SETTINGS_FLAG))
	{
		/* retrieve the specified file */
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

		// No settings needed
	}

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
