#ifndef TRANSMODEL_RUN_SETTINGS_H
#define TRANSMODEL_RUN_SETTINGS_H

/**
 * @file   transmodel_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for transmodel program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * transmodel program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the transmodel program
 */
class transmodel_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * User-specified input/output ply files
		 *
		 * There must be either zero or two files.
		 */
		std::vector<std::string> plyfiles;

		/**
		 * User-specified input/output obj files
		 *
		 * There must be either zero or two files.
		 */
		std::vector<std::string> objfiles;

		/**
		 * User-specified input/output xyz files
		 *
		 * There must be either zero or two files.
		 */
		std::vector<std::string> xyzfiles;

		/**
		 * The scale factor to apply
		 */
		double scale;

		/**
		 * The translation offset to apply 
		 *
		 * (gets applied after scale)
		 */
		double translate;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		transmodel_run_settings_t();

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
