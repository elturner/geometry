#ifndef OCTSURF_RUN_SETTINGS_H
#define OCTSURF_RUN_SETTINGS_H

/**
 * @file   octsurf_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for octsurf program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * octsurf program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/* the following specifies output file format */
enum OUTPUT_FILE_FORMAT
{
	FORMAT_VOX, /* .vox file for back-compatability with old carving */
	FORMAT_OBJ, /* Wavefront OBJ file format */
	FORMAT_SOF, /* Tao Ju's SOF (Signed Octree Format) */
	FORMAT_TXT, /* text file format */
	FORMAT_UNKNOWN /* unknown file format */
};

/**
 * This class is used to store run settings for the octsurf program
 */
class octsurf_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * Location of the input .oct files
		 */
		std::vector<std::string> octfiles;

		/**
		 * Location of the output file
		 *
		 * This program supports many different output filetypes,
		 * which are specified by the file extension of this
		 * given file path.
		 */
		std::string outfile;

		/* the following specifies the output mode by parsing
		 * the extension of the outfile. */
		OUTPUT_FILE_FORMAT output_format;

		/**
		 * If exporting to OBJ, this option indicates whether
		 * to export all leaf node centers or to export a mesh
		 */
		bool export_obj_leafs;

		/**
		 * If exporting to OBJ, this option indicates whether
		 * to export boundary leaf faces without any additional
		 * surface reconstruction.
		 */
		bool export_node_faces;

		/**
		 * If exporting to OBJ, this option indicates that
		 * the output should represent the node faces, and they
		 * should be colored based on their planar region.
		 */
		bool export_regions;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		octsurf_run_settings_t();

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

	/* helper functions */
	private:

		/**
		 * Determine extension of output file name
		 *
		 * Given a file name, will determine which format
		 * is being represented.
		 *
		 * @param fn   The filename to analyze
		 *
		 * @return     The parsed format
		 */
		static OUTPUT_FILE_FORMAT get_format(const std::string& fn);
};

#endif
