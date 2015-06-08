#include "generate_scanorama_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   generate_scanorama_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for generate_scanorama program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * generate_scanorama program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */
// TODO

/* file extensions to check for */
// TODO

/*--------------------------*/
/* function implementations */
/*--------------------------*/
		
generate_scanorama_run_settings_t::generate_scanorama_run_settings_t()
{
	// TODO
}

int generate_scanorama_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	vector<string> files;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates scanoramas "
			"for camera positions in the specified dataset.  "
			"Scanoramas are a point cloud representation that "
			"is used to indicate a panoramic image with depth "
			"at each pixel.");

	// TODO

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[generate_scanorama_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	// TODO

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
