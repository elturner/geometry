#ifndef GENERATE_SCANORAMA_RUN_SETTINGS_H
#define GENERATE_SCANORAMA_RUN_SETTINGS_H

/**
 * @file   generate_scanorama_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for generate_scanorama program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * generate_scanorama program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for 
 * the generate_scanorama program
 */
class generate_scanorama_run_settings_t
{
	/* parameters */
	public:

		/*-------------*/
		/* input files */
		/*-------------*/

		/**
		 * The xml hardware configuration file for the system
		 */
		std::string xml_config;

		/**
		 * The system path file (.mad or .noisypath)
		 */
		std::string pathfile;

		/**
		 * The geometry model file to import (.obj or .ply)
		 */
		std::string modelfile;

		/**
		 * This vector lists all the metadata files
		 * given for input fisheye cameras used to
		 * color the scanorama.
		 *
		 * There should be N file paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> cam_metafiles;

		/**
		 * This vector lists all the fisheye camera calibration
		 * file paths.
		 *
		 * There should be N file paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> cam_calibfiles;

		/**
		 * This vector lists all the image directory paths
		 * for each of the cameras used.
		 *
		 * There should be N directory paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> cam_imgdirs;

		/**
		 * The output .ptx file that will be exported by
		 * this program.
		 */
		std::string ptx_outfile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		generate_scanorama_run_settings_t();

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
