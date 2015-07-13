#include "find_doors_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   find_doors_settings.cpp
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

using namespace std;

/* the file formats to check for */

#define OCTFILE_EXT        "oct"
#define MADFILE_EXT        "mad"
#define NOISYPATHFILE_EXT  "noisypath"

/* function implementations */
		
find_doors_settings_t::find_doors_settings_t()
{
	/* set default values for this program's files */
	this->octfile     = "";
	this->pathfile    = "";
}

int find_doors_settings_t::parse(int argc, char** argv)
{
	vector<string> files, path_files;
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program estimates the position "
			"of doors in a scanned model by detecting the "
			"locations in an octree where the specified path "
			"moves from one room to another.");
	args.add_required_file_type(OCTFILE_EXT, 1, "The octree file "
			"representing the model geometry.  This must "
			"already be merged with the floor plan data "
			"in order to identify different rooms.");
	args.add_required_file_type(MADFILE_EXT, 0, "The path can be "
			"imported as a ." MADFILE_EXT " file.  "
			"Exactly one path file "
			"should be given, which can be formatted either "
			"as a ." MADFILE_EXT " or a ." NOISYPATHFILE_EXT
			" file.");
	args.add_required_file_type(NOISYPATHFILE_EXT, 0, "The path can be "
			"imported as a ." NOISYPATHFILE_EXT " file.  "
			"Exactly one path file "
			"should be given, which can be formatted either "
			"as a ." MADFILE_EXT " or a ." NOISYPATHFILE_EXT
			" file.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[find_doors_settings_t::parse]\t"
		     << "Bad command-line args:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* get octree file */
	files.clear();
	args.files_of_type(OCTFILE_EXT, files);
	if(files.size() > 1)
		cerr << "[find_doors_settings_t::parse]\t"
		     << "WARNING: Multiple ." << OCTFILE_EXT << " files "
		     << "given, only the first will be used: "
		     << files[0] << endl;
	this->octfile = files[0];

	/* get path file, which can be one of multiple formats */
	path_files.clear();
	files.clear();
	args.files_of_type(MADFILE_EXT, files);
	path_files.insert(path_files.begin(), files.begin(), files.end());
	files.clear();
	args.files_of_type(NOISYPATHFILE_EXT, files);
	path_files.insert(path_files.begin(), files.begin(), files.end());
	if(path_files.empty())
	{
		cerr << "[find_doors_settings_t::parse]\t"
		     << "ERROR: No path file specified.  Must include "
		     << "either a ." << MADFILE_EXT << " or a ."
		     << NOISYPATHFILE_EXT << " file." << endl;
		return -2;
	}
	if(path_files.size() > 1)
		cerr << "[find_doors_settings_t::parse]\t"
		     << "WARNING: Multiple path files "
		     << "given, only the first will be used: "
		     << path_files[0] << endl;
	this->pathfile = path_files[0];

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
