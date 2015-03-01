#include "generate_hia_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   generate_hia_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for generate_hia program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * generate_hia program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to look for */

#define OUTFILE_FLAG    "-o"

/* the file formats to check for */

#define OCTFILE_EXT     "oct"
#define LEVELSFILE_EXT  "levels"

/* function implementations */
		
generate_hia_settings_t::generate_hia_settings_t()
{
	/* set default values for this program's files */
	this->octree_file    = "";
	this->levels_file    = "";
	this->hia_prefix     = "";
}

int generate_hia_settings_t::parse(int argc, char** argv)
{
	vector<string> files;
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates a "
			"Histogrammed Interior Area (HIA) file based on "
			"the input from an octree file.");
	args.add(OUTFILE_FLAG, "The prefix of the ouput files to write.  "
			"This program will export one or more Histogrammed "
			"Interior Area (HIA) files from the input data.  "
			"These files represent 2D top-down histograms of "
			"each level specified in the input levels file.  "
			"This flag specifies the prefix for the output "
			"files, which will be numbered by level.\n\n"
			"Example:\n\nIf the input is:\n\n\t\"foo/bar_\""
			"\n\nThen the output files for a model with two "
			"levels will be:\n\n\t\"foo/bar_0.hia\"\n\t\""
			"foo/bar_1.hia\"", false, 1);
	args.add_required_file_type(OCTFILE_EXT, 1, "The input octree "
			"file.  This file represents the 3D volume "
			"information of the scanned environment, and is "
			"used to generate the output .hia file."); 
	args.add_required_file_type(LEVELSFILE_EXT, 1, "The input levels "
			"file.  This file represents the division of how "
			"the building is separated vertically into levels, "
			"or stories.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[generate_hia_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* get octree file */
	files.clear();
	args.files_of_type(OCTFILE_EXT, files);
	if(files.size() > 1)
		cerr << "[generate_hia_settings_t::parse]\t"
		     << "WARNING: Multiple ." << OCTFILE_EXT << " files "
		     << "given, only the first will be used: "
		     << files[0] << endl;
	this->octree_file = files[0];

	/* get levels file */ 
	files.clear();
	args.files_of_type(LEVELSFILE_EXT, files);
	if(files.size() > 1)
		cerr << "[generate_hia_settings_t::parse]\t"
		     << "WARNING: Multiple ." << LEVELSFILE_EXT << " files "
		     << "given, only the first will be used: "
		     << files[0] << endl;
	this->levels_file = files[0];

	/* get output hia file prefix */
	this->hia_prefix = args.get_val(OUTFILE_FLAG);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
