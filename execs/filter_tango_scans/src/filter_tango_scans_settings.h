#ifndef FILTER_TANGO_SCANS_SETTINGS_H
#define FILTER_TANGO_SCANS_SETTINGS_H

/**
 * @file   filter_tango_scans_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for filter_tango_scans program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * filter_tango_scans program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for 
 * the filter_tango_scans program
 */
class filter_tango_scans_run_settings_t
{
	/* parameters */
	public:

		/*-------------*/
		/* input files */
		/*-------------*/

		/**
		 * The tango data file (.dat)
		 */
		std::string tangofile;

		/**
		 * The timstamp synchronization xml file
		 */
		std::string timefile;

		/*------------*/
		/* parameters */
		/*------------*/

		/**
		 * The name of the tango sensor to filter.
		 */
		std::string sensor_name;

		/**
		 * Specifies the start index of the exported scanoramas
		 *
		 * If specified, then only the subset of scanoramas
		 * starting at this index (inclusive) will be exported.
		 *
		 * This value is useful if a previous run was prematurely
		 * terminated, and you want to start where you left off.
		 *
		 * The index specified is in the output indexing, NOT
		 * the input pose indices.
		 */
		int begin_idx;

		/**
		 * Specifies the ending index of the exported scanoramas
		 *
		 * If specified, then only the subset of scanoramas
		 * before this index (exclusive) will be exported.
		 *
		 * This value is useful if you only want to export a
		 * subset of the total scanoramas for a dataset.  If
		 * a negative value is specified, then all indices until
		 * the end of the dataset will be exported.
		 *
		 * The index specified is in the output indexing, NOT
		 * the input pose indices.
		 */
		int end_idx;

		/*--------------*/
		/* output files */
		/*--------------*/

		/**
		 * The output .fss file that contains scan data
		 */
		std::string fss_outfile;

		/**
		 * The output path file (.mad)
		 */
		std::string mad_outfile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		filter_tango_scans_run_settings_t();

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
