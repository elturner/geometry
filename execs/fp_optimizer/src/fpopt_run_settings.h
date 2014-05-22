#ifndef FPOPT_RUN_SETTINGS_H
#define FPOPT_RUN_SETTINGS_H

/**
 * @file   fpopt_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for fpopt program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * fpopt program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the fpopt program
 */
class fpopt_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * Location of the input .oct file
		 */
		std::string octfile;

		/**
		 * Locations of input floorplan files
		 */
		std::vector<std::string> input_fpfiles;

		/**
		 * Location of output floorplan files
		 */
		std::vector<std::string> output_fpfiles;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		fpopt_run_settings_t();

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
