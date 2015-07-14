#include "find_doors_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

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

/* the command-line arguments to look for */

#define SETTINGS_FILE      "-s"
#define OUTFILE_FLAG       "-o"

/* the file formats to check for */

#define OCTFILE_EXT        "oct"
#define HIAFILE_EXT        "hia"
#define MADFILE_EXT        "mad"
#define NOISYPATHFILE_EXT  "noisypath"

/* the xml parameters to look for */

#define XML_MIN_WIDTH      "find_doors_min_width"
#define XML_MAX_WIDTH      "find_doors_max_width"
#define XML_MIN_HEIGHT     "find_doors_min_height"
#define XML_MAX_HEIGHT     "find_doors_max_height"
#define XML_ANGLE_STEPSIZE "find_doors_angle_stepsize"

/* function implementations */
		
find_doors_settings_t::find_doors_settings_t()
{
	/* set default values for this program's files */
	this->octfile         = "";
	this->pathfile        = "";
	this->hiafiles.clear();
	this->outfile         = "";
	this->door_min_width  = 0.8128; /* in meters, or about 32 inches */
	this->door_max_width  = 1.2192; /* in meters, or about 48 inches */
	this->door_min_height = 2.0;    /* in meters, or about one Nick */
	this->door_max_height = 2.7432; /* in meters, or about 9 feet */
	this->angle_stepsize  = 0.08; /* units: radians */
}

int find_doors_settings_t::parse(int argc, char** argv)
{
	vector<string> files, path_files;
	XmlSettings settings;
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program estimates the position "
			"of doors in a scanned model by detecting the "
			"locations in an octree where the specified path "
			"moves from one room to another.");
	args.add(SETTINGS_FILE, "The xml settings file that defines "
			"parameters used for this analysis.  If not "
			"specified, default parameters will be used.",
			true, 1);
	args.add(OUTFILE_FLAG, "The file path to write the output of "
			"this processing.  This program will export an "
			"ascii file that has a door position (x,y,z) on "
			"each line.  The units are in meters.", false, 1);
	args.add_required_file_type(OCTFILE_EXT, 1, "The octree file "
			"representing the model geometry.  This must "
			"already be merged with the floor plan data "
			"in order to identify different rooms.  This "
			"should be the same octfile that was used to "
			"generate the input .hia files.");
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
	args.add_required_file_type(HIAFILE_EXT, 1, 
			"Specifies the Top-down Histogram of Interior "
			"Area (hia) file.  Each file represents a level "
			"of the building.  Doors will be located for this "
			"level.");

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

	/* get the provided hia files */
	this->hiafiles.clear();
	args.files_of_type(HIAFILE_EXT, this->hiafiles);

	/* get location of output file */
	this->outfile = args.get_val(OUTFILE_FLAG);

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

	/* check to see if the xml settings file is provided.  If so,
	 * then read its contents */
	if(args.tag_seen(SETTINGS_FILE))
	{
		/* read the file */
		if(!(settings.read(args.get_val(SETTINGS_FILE))))
		{
			cerr << "[find_doors_settings::parse]\t"
			     << "Error!  Unable to read xml settings "
			     << "file for this program." << endl;
			return -3;
		}

		/* read in values from settings file */
		if(settings.is_prop(XML_MIN_WIDTH))
			this->door_min_width 
				= settings.getAsDouble(XML_MIN_WIDTH);
		if(settings.is_prop(XML_MAX_WIDTH))
			this->door_max_width 
				= settings.getAsDouble(XML_MAX_WIDTH);
		if(settings.is_prop(XML_MIN_HEIGHT))
			this->door_min_height
				= settings.getAsDouble(XML_MIN_HEIGHT);
		if(settings.is_prop(XML_MAX_HEIGHT))
			this->door_max_height
				= settings.getAsDouble(XML_MAX_HEIGHT);
		if(settings.is_prop(XML_ANGLE_STEPSIZE))
		{
			/* this value should be denoted in the file in
			 * units of degrees, but stored in memory in
			 * units of radians, so do the conversion
			 * here. */
			this->angle_stepsize
				= settings.getAsDouble(XML_ANGLE_STEPSIZE);
			this->angle_stepsize *= M_PI / 180;
		}
	}

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
