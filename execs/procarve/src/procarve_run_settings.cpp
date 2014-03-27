#include "procarve_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <boost/thread.hpp>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   procarve_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for procarve program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * procarve program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define MADFILE_FLAG   "-p" /* localization output path file (.mad) */
#define CONFILE_FLAG   "-c" /* hardware config file (.xml) */
#define TIMEFILE_FLAG  "-t" /* time-sync output file (.xml) */
#define SETTINGS_FLAG  "-s" /* program-specific settings (.xml) */
#define CHUNKLIST_FLAG "-l" /* input chunklist file (.chunklist) */
#define OCTFILE_FLAG   "-o" /* where to store the output octfile (.oct) */

/* file extensions to check for */

#define FSS_FILE_EXT       "fss"
#define XML_FILE_EXT       "xml"
#define FLOORPLAN_FILE_EXT "fp"

/* xml tags to check for in settings file */

#define XML_DEFAULT_CLOCK_UNCERTAINTY "procarve_default_clock_uncertainty"
#define XML_RESOLUTION_TAG            "procarve_resolution"
#define XML_CARVEBUF_TAG              "procarve_carvebuf"
#define XML_CHUNKDIR_TAG              "procarve_chunkdir"
#define XML_NUM_THREADS_TAG           "procarve_num_threads"

/* function implementations */
		
procarve_run_settings_t::procarve_run_settings_t()
{
	/* set default values for this program's input files */
	this->madfile   = "";
	this->confile   = "";
	this->timefile  = "";
	this->chunklist = "";
	this->chunkdir  = "chunks"; /* by default, chunks in subdir */
	this->fssfiles.clear();
	this->fpfiles.clear();
	this->octfile = "";

	/* the following values are read from an input xml settings
	 * file.  If that file does not have the setting listed, then
	 * the following default settings will be used. */
	this->default_clock_uncertainty = 0.001; /* units of seconds */
	this->resolution = 0.01; /* units: meters */
	this->carvebuf   = 2; /* two standard deviations */
	this->num_threads = boost::thread::hardware_concurrency();
}

int procarve_run_settings_t::parse(int argc, char** argv)
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
	args.add(CHUNKLIST_FLAG, "Input .chunklist file indicates location "
			"of chunks to read from disk.  Each chunk lists "
			"scan indices associated with each location in "
			"space.", false, 1);
	args.add(OCTFILE_FLAG, "Where to store the output .oct file, which "
			"represents the carved and labeled volume from the "
			"input scans.", false, 1);
	args.add_required_file_type(FSS_FILE_EXT, 1,
			"These files are used as input scan files.  They"
			" also contain statistical information about the "
			"scanner that generated the data.");
	args.add_required_file_type(FLOORPLAN_FILE_EXT, 0,
			"These files represent reconstructed floor plans"
			" of the environment with room label information."
			"  Any floorplan files given will be merged into "
			"the carved model.");

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
	this->chunklist         = args.get_val(CHUNKLIST_FLAG);
	settings_file           = args.get_val(SETTINGS_FLAG);
	this->octfile           = args.get_val(OCTFILE_FLAG);
	args.files_of_type(FSS_FILE_EXT, this->fssfiles);
	args.files_of_type(FLOORPLAN_FILE_EXT, this->fpfiles);

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
	if(settings.is_prop(XML_CHUNKDIR_TAG))
		this->chunkdir = settings.get(XML_CHUNKDIR_TAG);
	if(settings.is_prop(XML_RESOLUTION_TAG))
		this->resolution = settings.getAsDouble(XML_RESOLUTION_TAG);
	if(settings.is_prop(XML_CARVEBUF_TAG))
		this->carvebuf = settings.getAsDouble(XML_CARVEBUF_TAG);
	if(settings.is_prop(XML_DEFAULT_CLOCK_UNCERTAINTY))
		this->default_clock_uncertainty = settings.getAsDouble(
			XML_DEFAULT_CLOCK_UNCERTAINTY);
	if(settings.is_prop(XML_NUM_THREADS_TAG))
		this->num_threads = settings.getAsUint(XML_NUM_THREADS_TAG);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
