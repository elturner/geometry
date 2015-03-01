#ifndef GENERATE_HIA_SETTINGS_H
#define GENERATE_HIA_SETTINGS_H

/**
 * @file   generate_hia_settings.h
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

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the generate_hia program
 */
class generate_hia_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * The input octree file
		 */
		std::string octree_file;

		/**
		 * The input levels file
		 */
		std::string levels_file;

		/**
		 * The output hia file prefix.
		 *
		 * We only store the prefix since we may be exporting
		 * multiple output files, based on how many levels there
		 * are.
		 */
		std::string hia_prefix;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		generate_hia_settings_t();

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
