#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Configuration parameters for program
 *
 * Represents the functions used
 * to read command-line arguments.
 */

#include <string>
#include <vector>

/**
 * The config_t class is used to store user-provided arguments
 */
class config_t
{
	/* parameters */
	public:

		/**
		 * provided fp file
		 *
		 * This file describes floorplan geometry to import
		 */
		std::string fp_infile; 
	
		/**
		 * Provided windows files
		 *
		 * Can be empty, which means no windows specified
		 */
		std::vector<std::string> windows_infiles;
		
		/**
		 * Provided lights files
		 *
		 * Can be empty, which means no lights specified
		 */
		std::vector<std::string> lights_infiles;
		
		/**
		 * Provided plugloads files
		 *
		 * Can be empty, which means no plugloads specified
		 */
		std::vector<std::string> plugloads_infiles;

		/** 
		 * The list of output files
		 *
		 * only write ones that have valid file paths
		 */
		std::vector<std::string> outfile_obj;
		std::vector<std::string> outfile_idf;
		std::vector<std::string> outfile_wrl;
		std::vector<std::string> outfile_csv;
		std::vector<std::string> outfile_ply;
		std::vector<std::string> outfile_shp;

	/*** functions ***/
	public:

		/**
		 * Reads the input command-line arguments and stores values
		 *
		 * Will parse the provided command-line arguments, and
		 * store the found values in the fields of this structure.
		 *
		 * @param argc   Number of command-line arguments
		 * @param argv   The command-line arguments from main()
		 *
		 * @returns      Returns 0 on success, non-zero on failure.
		 */
		int parse(int argc, char** argv);
};

#endif
