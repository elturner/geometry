#ifndef NOISYPATH_RUN_SETTINGS_H
#define NOISYPATH_RUN_SETTINGS_H

/**
 * @file   noisypath_run_settings.h
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

#include <string>

/**
 * This class is used to store run settings for the noisypath program
 */
class noisypath_run_settings_t
{
	/* parameters */
	public:

		/* the following parameters can be used */

		/**
		 * The specified linear sigma value to optionally use
		 *
		 * This value, if specified as non-negative, will
		 * represent a constant uncertainty standard-deviation
		 * to use for the linear position of each pose.
		 */
		double linear_sigma;

		/**
		 * The specified rotational sigma value to optionally use
		 *
		 * This value, if specified as non-negative, will
		 * represent a constant uncertainty standard-deviation
		 * to use for the rotational orientation of each pose
		 */
		double rotational_sigma;

		/* the following input files are used by this program */

		/**
		 * The input .mad file to use
		 *
		 * This should specify a 3D .mad file, which represents
		 * the deterministic localization output.
		 */
		std::string madfile;

		/* the following specifies output file(s) */

		/**
		 * This file path represents where the output .noisypath
		 * file is written.
		 */
		std::string outfile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		noisypath_run_settings_t();

		/**
		 * Parses settings from command-line
		 *
		 * Will parse the command-line arguments to get all
		 * the necessary settings.  This may also include
		 * parsing xml settings files that were passed on
		 * the command-line.
		 *
		 * @param argc  Number of command-line arguments
		 * @param argv  The command-line arguments given to main()
		 *
		 * @return      Returns zero on success, non-zero on failure
		 */
		int parse(int argc, char** argv);
};

#endif
