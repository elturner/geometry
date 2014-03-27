#ifndef PROCARVE_RUN_SETTINGS_H
#define PROCARVE_RUN_SETTINGS_H

/**
 * @file   procarve_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for procarve program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * procarve program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the procarve program
 */
class procarve_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * a .mad file represents the 3D path of the system
		 */
		std::string madfile;

		/**
		 * A hardware xml configuration file represents sensor
		 * extrinsics.
		 */
		std::string confile;

		/**
		 * The time synchronization xml output file.  Used for
		 * determining timestamping error.
		 */
		std::string timefile;
		
		/**
		 * The input .chunklist file
		 */
		std::string chunklist;

		/**
		 * The directory to store the chunk files.  This is
		 * relative to the chunklist_outfile.
		 */
		std::string chunkdir;
		
		/**
		 * The input scan files.
		 *
		 * These .fss files contain statistical information as
		 * well as scan geometry from range sensors.
		 */
		std::vector<std::string> fssfiles;

		/**
		 * The input floor plan files.
		 *
		 * These .fp files contain floor plan information.
		 */
		std::vector<std::string> fpfiles;

		/**
		 * Where to store the output .oct file
		 */
		std::string octfile;

		/* the following parameters specify details about how
		 * to store the generated chunks to disk */

		/**
		 * This value indicates the number of threads to use
		 * during the carving portion of procarve.
		 */
		unsigned int num_threads;

		/**
		 * The default clock uncertainty to use
		 *
		 * The clock uncertainty for a sensor can be computed
		 * based on how well a linear fit modeled its timestamp
		 * synchronization, but if this value is unavailable, then
		 * the following value is used instead.  It is measured
		 * in units of seconds, and denotes a standard deviation
		 * of the error.
		 */
		double default_clock_uncertainty;

		/**
		 * The limit resolution for volumetric carving.
		 *
		 * The generated octree will not have leafs smaller
		 * than this size.
		 */
		double resolution;

		/**
		 * How far past scan points to carve, in units of
		 * standard deviations.
		 */
		double carvebuf;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		procarve_run_settings_t();

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
