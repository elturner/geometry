#include "hia_floorplan_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   hia_floorplan_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for hia_floorplan program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * hia_floorplan program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the file formats to check for */

#define HIAFILE_EXT     "hia"

/* function implementations */
		
hia_floorplan_settings_t::hia_floorplan_settings_t()
{
	/* set default values for this program's files */
	this->hiafile     = "";
}

int hia_floorplan_settings_t::parse(int argc, char** argv)
{
	vector<string> files;
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates a floorplan "
			"model based on the input Histogrammed Interior "
			"Area (HIA) file.");
	args.add_required_file_type(HIAFILE_EXT, 1, "The input histogram "
			"file.  This file represents density of open area "
			"in the building environment for this level.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[hia_floorplan_settings_t::parse]\t"
		     << "Bad command-line args:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* get octree file */
	files.clear();
	args.files_of_type(HIAFILE_EXT, files);
	if(files.size() > 1)
		cerr << "[hia_floorplan_settings_t::parse]\t"
		     << "WARNING: Multiple ." << HIAFILE_EXT << " files "
		     << "given, only the first will be used: "
		     << files[0] << endl;
	this->hiafile = files[0];

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
