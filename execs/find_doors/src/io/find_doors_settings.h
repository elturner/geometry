#ifndef FIND_DOORS_SETTINGS_H
#define FIND_DOORS_SETTINGS_H

/**
 * @file   find_doors_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for find_doors program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * find_doors program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the find_doors program
 */
class find_doors_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * The input octree file
		 */
		std::string octfile;

		/**
		 * The input path file
		 *
		 * Can be either .mad or .noisypath
		 */
		std::string pathfile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		find_doors_settings_t();

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
