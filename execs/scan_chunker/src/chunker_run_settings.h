#ifndef CHUNKER_RUN_SETTINGS_H
#define CHUNKER_RUN_SETTINGS_H

/**
 * @file   chunker_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for chunker program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the scan
 * chunker program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the chunker program
 */
class chunker_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * The carvemap input file stores the probabilistic models
		 * for all scan points in this dataset
		 */
		std::string carvemapfile;

		/**
		 * The wedge file contains a list of wedge objects, which
		 * are composed of indices into the carvemapfile.  Each
		 * wedge references for scan points in the carve map
		 * file in order to be able to interpolate across scan
		 * points during volumetric analysis.
		 */
		std::string wedgefile;

		/* the following parameters specify details about how
		 * to store the generated chunks to disk */

		/**
		 * The chunk size, in units of meters 
		 */
		double chunk_size;

		/**
		 * Where to store the generated .chunklist file
		 */
		std::string chunklist_outfile;

		/**
		 * The directory to store the chunk files.  This is
		 * relative to the chunklist_outfile.
		 */
		std::string chunkdir;

		/**
		 * Flag determines whether scans should be interpolated
		 * into full wedges during geometric intersection tests.
		 */
		bool interpolate;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		chunker_run_settings_t();

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
