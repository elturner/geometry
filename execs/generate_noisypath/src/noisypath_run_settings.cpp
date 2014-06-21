#include "noisypath_run_settings.h"
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   noisypath_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for noisypath program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * noisypath program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define LINEAR_SIGMA_FLAG   "--lin_sigma" /* constant linear uncertainty */
#define ROTATION_SIGMA_FLAG "--rot_sigma" /* constant rot. uncertainty */

/* file extensions to check for */

#define MAD_FILE_EXT   "mad"
#define NP_FILE_EXT    "noisypath"

/* function implementations */
		
noisypath_run_settings_t::noisypath_run_settings_t()
{
	/* set default values for this program's files */
	this->linear_sigma     = -1;
	this->rotational_sigma = -1;
	this->madfile          = "";
	this->outfile          = "";
}

int noisypath_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	vector<string> madfiles, npfiles;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program converts path info into "
			"the .noisypath format, which houses statistical "
			"information as well as the deterministic path.");
	args.add(LINEAR_SIGMA_FLAG, "Specifies the constant-value standard "
			"deviation to assume for the positional "
			"distrubition for each pose.", true, 1);
	args.add(ROTATION_SIGMA_FLAG, "Specifies the constant-value "
			"standard deviation to assume for the rotational "
			"orientation distribution for each pose.", true, 1);
	args.add_required_file_type(MAD_FILE_EXT, 0,
			"The input .mad file specifies the deterministic "
			"3D localization output information.");
	args.add_required_file_type(NP_FILE_EXT, 1,
			"The output .noisypath file specifies where to "
			"export the final path statistics information.  "
			"This file contains a superset of the info that "
			"is represented in a .mad file.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[noisypath_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	args.files_of_type(NP_FILE_EXT,  npfiles);
	this->outfile = npfiles[0];
	
	/* get optional parameters */
	args.files_of_type(MAD_FILE_EXT, madfiles);
	if(madfiles.empty())
		this->madfile = "";
	else
		this->madfile = madfiles[0];
	if(args.tag_seen(LINEAR_SIGMA_FLAG))
		this->linear_sigma = args.get_val_as<double>(
				LINEAR_SIGMA_FLAG);
	else
		this->linear_sigma = -1.0; /* specified as invalid */
	if(args.tag_seen(ROTATION_SIGMA_FLAG))
		this->rotational_sigma = args.get_val_as<double>(
				ROTATION_SIGMA_FLAG);
	else
		this->rotational_sigma = -1.0; /* specified as invalid */

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
