#include "chunker_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   chunker_run_settings.cpp
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

using namespace std;

/* the command-line flags to check for */

#define WEDGEFILE_FLAG  "-w" /* input wedge file (.wedge) */
#define SETTINGS_FLAG   "-s" /* program-specific settings (.xml) */
#define CHUNKLIST_FLAG  "-o" /* output chunklist file (.chunklist) */

/* file extensions to check for */

#define FSS_FILE_EXT "fss"
#define XML_FILE_EXT "xml"

/* xml tags to check for in settings file */

#define XML_CHUNKSIZE_TAG "procarve_chunksize"
#define XML_CHUNKDIR_TAG  "procarve_chunkdir"

/* function implementations */
		
chunker_run_settings_t::chunker_run_settings_t()
{
	/* set default values for this program */
	this->wedgefile = "";
	this->chunklist_outfile = "";
	
	/* the following values are read from an input xml settings
	 * file.  If that file does not have the setting listed, then
	 * the following default settings will be used. */
	this->chunk_size = 2.0; /* default chunks: cube edge two meters */
	this->chunkdir  = "chunks"; /* by default, put chunks in a subdir */
}

int chunker_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates chunk files"
			" from input scans to be used in the procarve "
			"program.");
	args.add(WEDGEFILE_FLAG, "The wedge input file, containing the"
			" probabilistic models for carve wedges made from "
			"the original scan files of this dataset.",
			false, 1);
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to generate chunks and where to store them on "
			"disk.", false, 1);
	args.add(CHUNKLIST_FLAG, "Where to store the output chunklist file."
			"  This file contains a list of all chunks written"
			"  to disk.  The chunks themselves will be stored "
			" in a directory relative to this file as specified"
			" by the input settings file.", false, 1);

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[chunk_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	this->wedgefile         = args.get_val(WEDGEFILE_FLAG);
	settings_file           = args.get_val(SETTINGS_FLAG);
	this->chunklist_outfile = args.get_val(CHUNKLIST_FLAG);

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
	if(settings.is_prop(XML_CHUNKSIZE_TAG))
		this->chunk_size = settings.getAsDouble(XML_CHUNKSIZE_TAG);
	if(settings.is_prop(XML_CHUNKDIR_TAG))
		this->chunkdir = settings.get(XML_CHUNKDIR_TAG);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
