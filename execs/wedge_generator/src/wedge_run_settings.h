#ifndef WEDGE_RUN_SETTINGS_H
#define WEDGE_RUN_SETTINGS_H

/**
 * @file   wedge_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for wedge program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the wedge
 * generation program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the wedge program
 */
class wedge_run_settings_t
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
		 * The input scan files.
		 *
		 * These .fss files contain statistical information as
		 * well as scan geometry from range sensors.
		 */
		std::vector<std::string> fssfiles;

		/**
		 * The output carvemap file
		 *
		 * This .carvemap file stores the probabilistic
		 * distributions for each scan point, along with results
		 * of curvature analysis for each point.
		 */
		std::string carvemapfile;

		/**
		 * The output wedge file
		 *
		 * This .wedge file stores the list of wedges, where each
		 * wedge is represented by four carvemap points.  The
		 * indices referenced in this file are relative to the
		 * carvemaps stored in carvemapfile.
		 */
		std::string wedgefile;

		/* the following parameters specify details about how
		 * to generate the wedges */

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
		 * How far past scan points to carve, in units of
		 * standard deviations.
		 */
		double carvebuf;

		/**
		 * The line-fitting distance parameter.  This value
		 * indicates how far away neighbors can be from a point
		 * in a scan frame when checking the planarity
		 * characteristics of that point.  A small value will allow
		 * for smaller lines within the scan, but a larger value
		 * will allow for robustness against noise.
		 *
		 * Units: meters
		 */
		double linefit_dist;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		wedge_run_settings_t();

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
