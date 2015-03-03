#ifndef HIA_FLOORPLAN_SETTINGS_H
#define HIA_FLOORPLAN_SETTINGS_H

/**
 * @file   hia_floorplan_settings.h
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

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the hia_floorplan program
 */
class hia_floorplan_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * The input hia file.
		 */
		std::string hiafile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		hia_floorplan_settings_t();

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
