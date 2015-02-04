#include "oct2dq_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   oct2dq_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for oct2dq program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * oct2dq program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FLAG     "-s" /* program-specific settings (.xml) */
#define CONFIGFILE_FLAG   "-c" /* hardware xml config file */
#define OUTFILE_FLAG      "-o" /* specifies dq file prefix */

/* file extensions to check for */

#define OCT_FILE_EXT    "oct"
#define PATH_FILE_EXT   "noisypath"
#define FSS_FILE_EXT    "fss"
#define LEVELS_FILE_EXT "levels"

/* the xml parameters to look for */

#define XML_COALESCE_DISTTHRESH      "oct2dq_coalesce_distthresh"
#define XML_COALESCE_PLANETHRESH     "oct2dq_coalesce_planethresh"
#define XML_USE_ISOSURFACE_POS       "oct2dq_use_isosurface_pos"
#define XML_VERTICALITYTHRESH        "oct2dq_verticalitythresh"
#define XML_SURFACEAREATHRESH        "oct2dq_surfaceareathresh"
#define XML_FLOORCEILSURFAREATHRESH  "oct2dq_floorceilsurfareathresh"
#define XML_WALLHEIGHTTHRESH         "oct2dq_wallheightthresh"
#define XML_MINROOMSIZE              "oct2dq_minroomsize"
#define XML_MINLEVELHEIGHT           "oct2dq_minlevelheight"
#define XML_CHOICERATIOTHRESH        "oct2dq_choiceratiothresh"
#define XML_DQ_RESOLUTION            "oct2dq_dq_resolution"

/* function implementations */
		
oct2dq_run_settings_t::oct2dq_run_settings_t()
{
	/* set default values for this program's files */
	this->octfile           = ""; /* input file */
	this->pathfile          = ""; /* input path file */
	this->dqfile_prefix     = ""; /* output dq file */
	this->levelsfile        = ""; /* output levels file */
	this->fssfiles.clear();       /* input scan files */

	/* set default parameter values */
	this->coalesce_distthresh     = 2.0;
	this->coalesce_planethresh    = 0.5;
	this->use_isosurface_pos      = false;
	this->verticalitythresh       = 0.08;
	this->surfaceareathresh       = 1.0;
	this->wallheightthresh        = 2.5;
	this->floorceilsurfareathresh = 2.0;
	this->minlevelheight          = 2.0;
	this->minroomsize             = 1.5;
	this->choiceratiothresh       = 0.1;
	this->dq_resolution           = -1.0;
}

int oct2dq_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	vector<string> files;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates wall samples "
			"from an input octree (.oct) file.  Wall samples "
			"are used to generate floorplans, and are point "
			"representations of the major walls in the "
			"environment.");
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to generate wall samples from the input.",
			false, 1);
	args.add(CONFIGFILE_FLAG, "The .xml hardware config file that "
			"specifies the location of the sensors with "
			"with respect to the rest of the hardware system.",
			false, 1);
	args.add(OUTFILE_FLAG, "Specifies where to write the output .dq "
			"files.  This ascii-formatted file indicates "
			"the extracted wall samples for each level of "
			"the scanned building environment.\n\n"
			"Should provide a filepath prefix for "
			"these output files.  So, if the following value "
			"is given:\n\n"
			"\t../foo/bar/output_\n\n"
			"And if, for example, two dq files are generated, "
			"then they will be exported to the following "
			"paths:\n\n"
			"\t../foo/bar/output_0.dq\n"
			"\t../foo/bar/output_1.dq\n\n"
			"The number and extension are appended "
			"automatically to the given string.", false, 1);
	args.add_required_file_type(OCT_FILE_EXT, 1,
			"The input octree file.  This file represent the "
			"volume information of the scanned environment, and"
			" are processed at a given resolution.");
	args.add_required_file_type(PATH_FILE_EXT, 1,
			"The input path file.  This file represents the "
			"path the system took to traverse the "
			"environment.");
	args.add_required_file_type(FSS_FILE_EXT, 1,
			"The input scan files.  These scan files should "
			"represents the scanners that observed the "
			"environment and were used to generate the "
			"octree.");
	args.add_required_file_type(LEVELS_FILE_EXT, 0,
			"The output levels file.  Will specify how many "
			"building levels (stories) were discovered, and "
			"the elevation ranges on each.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[oct2dq_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* input octfile */
	files.clear();
	args.files_of_type(OCT_FILE_EXT, files);
	if(files.size() > 1)
		cerr << "[oct2dq_run_settings_t::parse]\t"
		     << "WARNING: Multiple ." << OCT_FILE_EXT << " files "
		     << "given, only the first will be used." << endl;
	this->octfile = files[0];

	/* input path file */
	files.clear();
	args.files_of_type(PATH_FILE_EXT, files);
	if(files.size() > 1)
		cerr << "[oct2dq_run_settings_t::parse]\t"
		     << "WARNING: Multiple ." << PATH_FILE_EXT << " files "
		     << "given, only the first will be used." << endl;
	this->pathfile = files[0];

	/* input fss files */
	args.files_of_type(FSS_FILE_EXT, this->fssfiles);
	
	/* input xml hardware config file */
	this->configfile = args.get_val(CONFIGFILE_FLAG);

	/* output levels file */
	files.clear();
	args.files_of_type(LEVELS_FILE_EXT, files);
	if(files.size() > 1)
		cerr << "[oct2dq_run_settings_t::parse]\t"
		     << "WARNING: Multiple ." << LEVELS_FILE_EXT << " files"
		     << " given, only the first will be used." << endl;
	this->levelsfile = files[0];

	/* retrieve the specified files from flags */
	settings_file       = args.get_val(SETTINGS_FLAG);
	this->dqfile_prefix = args.get_val(OUTFILE_FLAG);

	/* attempt to open and parse the settings file */
	if(!settings.read(settings_file))
	{
		/* unable to open or parse settings file.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[oct2dq_run_settings_t::parse]\t"
		     << "Error " << ret << ":  Unable to parse "
		     << "settings file: " << settings_file << endl;
		return ret;
	}
	
	/* read in settings from file.  If they are not in the given
	 * file, then the default settings that were set in this
	 * object's constructor will be used. */
	if(settings.is_prop(XML_COALESCE_DISTTHRESH))
		this->coalesce_distthresh 
			= settings.getAsDouble(XML_COALESCE_DISTTHRESH);
	if(settings.is_prop(XML_COALESCE_PLANETHRESH))
		this->coalesce_planethresh 
			= settings.getAsDouble(XML_COALESCE_PLANETHRESH);
	if(settings.is_prop(XML_USE_ISOSURFACE_POS))
		this->use_isosurface_pos 
			= (0 != settings.getAsInt(XML_USE_ISOSURFACE_POS));
	if(settings.is_prop(XML_VERTICALITYTHRESH))
		this->verticalitythresh
			= settings.getAsDouble(XML_VERTICALITYTHRESH);
	if(settings.is_prop(XML_SURFACEAREATHRESH))
		this->surfaceareathresh
			= settings.getAsDouble(XML_SURFACEAREATHRESH);
	if(settings.is_prop(XML_WALLHEIGHTTHRESH))
		this->wallheightthresh
			= settings.getAsDouble(XML_WALLHEIGHTTHRESH);
	if(settings.is_prop(XML_FLOORCEILSURFAREATHRESH))
		this->floorceilsurfareathresh
			= settings.getAsDouble(XML_FLOORCEILSURFAREATHRESH);
	if(settings.is_prop(XML_MINLEVELHEIGHT))
		this->minlevelheight
			= settings.getAsDouble(XML_MINLEVELHEIGHT);
	if(settings.is_prop(XML_MINROOMSIZE))
		this->minroomsize
			= settings.getAsDouble(XML_MINROOMSIZE);
	if(settings.is_prop(XML_CHOICERATIOTHRESH))
		this->choiceratiothresh
			= settings.getAsDouble(XML_CHOICERATIOTHRESH);
	if(settings.is_prop(XML_DQ_RESOLUTION))
		this->dq_resolution
			= settings.getAsDouble(XML_DQ_RESOLUTION);
	
	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
