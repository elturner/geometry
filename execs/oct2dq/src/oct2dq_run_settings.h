#ifndef OCT2DQ_RUN_SETTINGS_H
#define OCT2DQ_RUN_SETTINGS_H

/**
 * @file   oct2dq_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for oct2dq program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * oct2dq program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the oct2dq program
 */
class oct2dq_run_settings_t
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
		 * Location of the input .noisypath file
		 */
		std::string pathfile;

		/**
		 * Location of the input xml hardware config file
		 */
		std::string configfile;

		/**
		 * Locations of the input scan files
		 */
		std::vector<std::string> fssfiles;

		/**
		 * Location of the output .dq file
		 */
		std::string dqfile;

		/* the following parameters are used for this program */

		/**
		 * The distance threshold to use for region coalescing
		 *
		 * Node faces that are farther than this many std. devs.
		 * away from the fitting plane will be considered outliers
		 * and prevent regions from being merged.
		 */
		double coalesce_distthresh;

		/**
		 * The planarity threshold to use for region coalescing
		 *
		 * Node faces with a planarity less than this value
		 * will not be coalesced into larger regions.
		 */
		double coalesce_planethresh;

		/**
		 * Specifies whether to use isosurface positions
		 *
		 * If true, will use isosurface positions for each
		 * node face when computing fitting planes for regions
		 */
		bool use_isosurface_pos;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		oct2dq_run_settings_t();

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
