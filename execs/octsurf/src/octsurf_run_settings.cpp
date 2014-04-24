#include "octsurf_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   octsurf_run_settings.cpp
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

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FLAG  "-s" /* program-specific settings (.xml) */
#define OUTPUT_FLAG    "-o" /* where to store the output file */

/* file extensions to check for */

#define OCT_FILE_EXT   "oct"
#define VOX_FILE_EXT   "vox"
#define OBJ_FILE_EXT   "obj"
#define TXT_FILE_EXT   "txt"

/* function implementations */
		
octsurf_run_settings_t::octsurf_run_settings_t()
{
	/* set default values for this program's files */
	this->octfiles.clear();
	this->outfile = "";
	this->output_format = FORMAT_UNKNOWN;
}

int octsurf_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates meshed surface"
			" reconstructions from an input .oct file.  The "
			"input file should be generated using the procarve"
			" program.");
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to generate chunks and where to store them on "
			"disk.", true, 1);
	args.add(OUTPUT_FLAG, "Where to store the output file, which "
			"represents the meshed surface of the volume "
			"described by the input .oct files.  This program "
			"supports multiple output file formats, including: "
			".vox, .obj", false, 1);
	args.add_required_file_type(OCT_FILE_EXT, 1,
			"The input octree files.  These represent the "
			"volume information of the scanned environment, and"
			" are processed at a given resolution.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[octsurf_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	this->outfile = args.get_val(OUTPUT_FLAG);
	this->output_format 
		= octsurf_run_settings_t::get_format(this->outfile);
	args.files_of_type(OCT_FILE_EXT, this->octfiles);

	/* check if a settings xml file was specified */
	if(args.tag_seen(SETTINGS_FLAG))
	{
		/* retrieve the specified file */
		settings_file = args.get_val(SETTINGS_FLAG);
	
		/* attempt to open and parse the settings file */
		if(!settings.read(settings_file))
		{
			/* unable to open or parse settings file.  Inform
			 * user and quit. */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[chunk_run_settings_t::parse]\t"
			     << "Error " << ret << ":  Unable to parse "
			     << "settings file: " << settings_file << endl;
			return ret;
		}
	
		/* read in settings from file.  If they are not in the given
		 * file, then the default settings that were set in this
		 * object's constructor will be used. */

		// No settings needed
	}

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
		
OUTPUT_FILE_FORMAT octsurf_run_settings_t::get_format(const std::string& fn)
{
	string ext;
	size_t dotpos;

	/* find the position of the file extension */
	dotpos = fn.find_last_of('.');
	if(dotpos == string::npos)
		return FORMAT_UNKNOWN;
	
	/* get extension as string */
	ext = fn.substr(dotpos+1);

	/* determine format */
	if(!ext.compare(VOX_FILE_EXT))
		return FORMAT_VOX;
	if(!ext.compare(OBJ_FILE_EXT))
		return FORMAT_OBJ;
	if(!ext.compare(TXT_FILE_EXT))
		return FORMAT_TXT;
	return FORMAT_UNKNOWN; /* unknown file format */
}
