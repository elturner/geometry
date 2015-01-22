#include "align_path_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   align_path_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for align_path program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * align_path program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define CONFIGFILE_FLAG   "-c" /* hardware xml config file (.xml) */
#define TIMEFILE_FLAG     "-t" /* timestamp sync file (.xml) */
#define IC4FILE_FLAG      "-d" /* the imu data file (.dat) */
#define INPUTPATH_FLAG    "-i" /* the input .mad file */
#define OUTPUTPATH_FLAG   "-o" /* the output .mad file */
#define MAGDEC_FLAG       "--mag_dec" /* angle to true north from magnetic
                                       * north, in degrees */

/* function implementations */
		
align_path_run_settings_t::align_path_run_settings_t()
{
	/* set default values for this program's files */
	this->configfile  = "";
	this->timefile    = "";
	this->ic4file     = "";
	this->input_path  = "";
	this->output_path = "";
	this->magnetic_declination = 0; /* none by default */ 
}

int align_path_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("NOTE: THIS PROGRAM IS DEPRECATED\n"
			"\tPlease use align_path found in the localization "
			"repo\n\n"
			"This program reads in a 3D path "
			"file that can be aligned to any arbitrary "
			"coordinate system (z+ is assumed to be up).  The "
			"program will determine the direction of north, "
			"and export the path modified so that it aligned "
			"to north.");
	args.add(CONFIGFILE_FLAG, "The .xml hardware config file that "
			"specifies the location of the sensors with "
			"with respect to the rest of the hardware system.",
			false, 1);
	args.add(TIMEFILE_FLAG, "The .xml file that defines the timestamp "
			"synchronization between sensors on the system.",
			false, 1);
	args.add(IC4FILE_FLAG, "The .dat file that stores the recorded "
			"data from the IC4 intersense IMU.", false, 1);
	args.add(INPUTPATH_FLAG, "The input .mad file to parse for the 3D "
			"path.", false, 1);
	args.add(OUTPUTPATH_FLAG, "The output .mad file to write to when "
			"the path has been aligned.", false, 1);
	args.add(MAGDEC_FLAG, "This value specifies the magnetic "
			"declination at the scan location.  Magnetic "
			"declination is used to convert from magnetic "
			"north to true north.\n\n"
			"If you want the output to be aligned to magnetic "
			"north, then don't use this flag.  If you want "
			"the output path to be aligned to true north, then "
			"the value after this flag should be set to the "
			"magnetic declination at the lat/lon of the scan.  "
			"You can compute this value at this website:\n\n"
			"\thttp://magnetic-declination.com/\n\n"
			"The value passed to the program should be in "
			"degrees, with eastern angles as positive and "
			"western angles as negative.\n\n"
			"Examples:\n\n"
			"\tBerkeley, CA     =>  13.816 degrees\n"
			"\tWashington, D.C. => -10.85  degrees\n"
			"\tParis, France    =>   0.05  degrees\n",
			true, 1);

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[align_path_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* input xml hardware config file */
	this->configfile  = args.get_val(CONFIGFILE_FLAG);
	this->timefile    = args.get_val(TIMEFILE_FLAG);
	this->ic4file     = args.get_val(IC4FILE_FLAG);
	this->input_path  = args.get_val(INPUTPATH_FLAG);
	this->output_path = args.get_val(OUTPUTPATH_FLAG);

	/* check for optional arguments */
	if(args.tag_seen(MAGDEC_FLAG))
		this->magnetic_declination 
			= args.get_val_as<double>(MAGDEC_FLAG);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
