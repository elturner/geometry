#ifndef MERGE_RUN_SETTINGS_H
#define MERGE_RUN_SETTINGS_H

/**
 * @file   merge_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for merge_fp_oct program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * merge_fp_oct program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the merge program
 */
class merge_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * Location of the input .oct file
		 */
		std::string input_octfile;

		/**
		 * Location of the input chunklist file
		 */
		std::string input_chunklistfile;

		/**
		 * Location of the input wedge file
		 */
		std::string input_wedgefile;

		/**
		 * Location of the input carve map file
		 */
		std::string input_carvemapfile;

		/**
		 * Location of the output .oct file
		 *
		 * This can be the same as the input file, in which
		 * case this program will rewrite the file contents.
		 */
		std::string output_octfile;

		/**
		 * Locations of the floorplan files
		 */
		std::vector<std::string> fpfiles;

		/* the following parameters are used in this program */

		/**
		 * How much further, in number of octree levels, to
		 * carve object nodes than regular nodes.
		 */
		unsigned int object_refine_depth;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		merge_run_settings_t();

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
