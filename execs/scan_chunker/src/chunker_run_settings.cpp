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

#define MADFILE_FLAG   "-p" /* localization output path file (.mad) */
#define CONFILE_FLAG   "-c" /* hardware config file (.xml) */
#define TIMEFILE_FLAG  "-t" /* time-sync output file (.xml) */
#define SETTINGS_FLAG  "-s" /* program-specific settings (.xml) */
#define CHUNKLIST_FLAG "-o" /* output chunklist file (.chunklist) */

/* file extensions to check for */

#define FSS_FILE_EXT "fss"
#define XML_FILE_EXT "xml"

/* xml tags to check for in settings file */

#define XML_DEFAULT_CLOCK_UNCERTAINTY "procarve_default_clock_uncertainty"
#define XML_CARVEBUF_TAG  "procarve_carvebuf"
#define XML_CHUNKSIZE_TAG "procarve_chunksize"
#define XML_CHUNKDIR_TAG  "procarve_chunkdir"

/* function implementations */
		
chunker_run_settings_t::chunker_run_settings_t()
{
	/* set default values for this program */
	this->madfile  = "";
	this->confile  = "";
	this->timefile = "";
	this->fssfiles.clear();
	this->chunklist_outfile = "";
	
	/* the following values are read from an input xml settings
	 * file.  If that file does not have the setting listed, then
	 * the following default settings will be used. */
	this->default_clock_uncertainty = 0.001; /* units of seconds */
	this->carvebuf  = 2; /* two standard deviations */
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
	args.add(MADFILE_FLAG, "The localization output file that contains"
			" 3D path information.  Formatted as a .mad file",
			false, 1);
	args.add(CONFILE_FLAG, "The backpack hardware configuration file."
			"  This stores the sensor-specific extrinsics and "
			"settings.  Should be a .xml file.", false, 1);
	args.add(TIMEFILE_FLAG, "The timestamp synchronization output file."
			"  Used by this program for estimating error in "
			"timestamp values.  Should be a .xml file.",
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
	args.add_required_file_type(FSS_FILE_EXT, 1,
			"These files are used as input scan files.  They"
			" also contain statistical information about the "
			"scanner that generated the data.");

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
	this->madfile           = args.get_val(MADFILE_FLAG);
	this->confile           = args.get_val(CONFILE_FLAG);
	this->timefile          = args.get_val(TIMEFILE_FLAG);
	settings_file           = args.get_val(SETTINGS_FLAG);
	this->chunklist_outfile = args.get_val(CHUNKLIST_FLAG);
	args.files_of_type(FSS_FILE_EXT, this->fssfiles);

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
	if(settings.is_prop(XML_CARVEBUF_TAG))
		this->carvebuf = settings.getAsDouble(XML_CARVEBUF_TAG);
	if(settings.is_prop(XML_CHUNKSIZE_TAG))
		this->chunk_size = settings.getAsDouble(XML_CHUNKSIZE_TAG);
	if(settings.is_prop(XML_CHUNKDIR_TAG))
		this->chunkdir = settings.get(XML_CHUNKDIR_TAG);
	if(settings.is_prop(XML_DEFAULT_CLOCK_UNCERTAINTY))
		this->default_clock_uncertainty = settings.getAsDouble(
			XML_DEFAULT_CLOCK_UNCERTAINTY);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
