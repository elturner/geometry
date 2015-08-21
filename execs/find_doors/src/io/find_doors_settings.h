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

		/*-------------*/
		/* input files */
		/*-------------*/

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

		/**
		 * The Histogram of Interior Area (hia) file for
		 * current level.
		 *
		 * If multiple are specified, they will be treated
		 * as separate levels in the same building.
		 */
		std::vector<std::string> hiafiles;

		/*-------------*/
		/* output file */
		/*-------------*/

		/**
		 * The exported files will start with this file prefix
		 *
		 * So, if we are generating doors for levels #0 and #1,
		 * and the outfile_prefix = "foo/bar_", then the generated
		 * files will be named:
		 *
		 * 	foo/bar_0.doors
		 * 	foo/bar_1.doors
		 */
		std::string outfile_prefix;

		/**
		 * If non-empty, will write a pointcloud representation of
		 * the detected doors to this .xyz file.
		 */
		std::string outfile_xyz;

		/*-----------------------*/
		/* processing parameters */
		/*-----------------------*/

		/**
		 * Minimum door width
		 *
		 * This value is used to define a search area when
		 * optimizing the position and extent of doors.
		 *
		 * units:  meters
		 */
		double door_min_width;

		/**
		 * Maximum door width
		 *
		 * This value is used to define a search area when
		 * optimizing geometry of doors
		 *
		 * units:  meters
		 */
		double door_max_width;

		/**
		 * Minimum door height
		 *
		 * This value is used to optimize the height of doors
		 *
		 * units: meters
		 */
		double door_min_height;

		/**
		 * Maximum door height
		 *
		 * This value is used to estimate the height of doors
		 *
		 * units: meters
		 */
		double door_max_height;

		/**
		 * The angular stepsize to search for door orientation
		 *
		 * units:  radians
		 */
		double angle_stepsize;

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
