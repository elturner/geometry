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
		std::vector<std::string> fisheye_cam_metafiles;

		/**
		 * This vector lists all the fisheye camera calibration
		 * file paths.
		 *
		 * There should be N file paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> fisheye_cam_calibfiles;

		/**
		 * This vector lists all the image directory paths
		 * for each of the cameras used.
		 *
		 * There should be N directory paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> fisheye_cam_imgdirs;
		
		/**
		 * This vector lists all the metadata files
		 * given for input rectilinear cameras used to
		 * color the scanorama.
		 *
		 * There should be N file paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> rectilinear_cam_metafiles;

		/**
		 * This vector lists all the rectilinear camera calibration
		 * file paths.
		 *
		 * There should be N file paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> rectilinear_cam_calibfiles;

		/**
		 * This vector lists all the image directory paths
		 * for each of the cameras used.
		 *
		 * There should be N directory paths in this vector,
		 * where N is the number of active cameras on the
		 * system.
		 */
		std::vector<std::string> rectilinear_cam_imgdirs;

		/*------------*/
		/* parameters */
		/*------------*/

		/**
		 * The number of rows in the exported scanoramas
		 */
		size_t num_rows;

		/**
		 * The number of columns in the exported scanoramas
		 */
		size_t num_cols;

		/**
		 * The blending width to use for scanoramas
		 *
		 * The blendwidth indicates how much blending will
		 * occur between two images that overlap the same
		 * viewing angle.  This value should be in the range
		 * [0,1].  A value of zero indicates no blending, and
		 * a value 1 indicates a LOT of blending.
		 */
		double blendwidth;

		/**
		 * The minimum spacing distance between scanorama poses
		 * exported.
		 *
		 * The system will need to move at least this distance
		 * away before another scanorama is generated.
		 *
		 * units:  meters
		 */
		double min_spacing_dist;

		/**
		 * The maximum spacing distance between scanorama poses
		 * exported.
		 *
		 * The system will move at most this distance
		 * away before another scanorama is generated.
		 *
		 * units:  meters
		 */
		double max_spacing_dist;

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

		/**
		 * If true, will export .ptx files for each
		 * scanorama position.
		 */
		bool export_ptx;

		/**
		 * If true, will export .e57 files for each
		 * scanorama position.
		 */
		bool export_e57;

		/**
		 * If true, will export .png image files for
		 * each scanorama position.
		 */
		bool export_png;

		/*--------------*/
		/* output files */
		/*--------------*/

		/**
		 * The output scanorama file that will be exported by
		 * this program.
		 */
		std::string scano_outfile;

		/**
		 * The output .scanolist metadata file that will
		 * be export along with the data.
		 *
		 * If this is blank, no metadata file is exported
		 */
		std::string meta_outfile;

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
