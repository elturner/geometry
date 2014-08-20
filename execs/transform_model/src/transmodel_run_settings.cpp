#include "transmodel_run_settings.h"
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   transmodel_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for transmodel program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * transmodel program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SCALE_FLAG           "-s" /* specifies scale value */
#define TRANSLATE_FLAG       "-t" /* specifies translation value */

/* file extensions to check for */

#define PLY_FILE_EXT   "ply"
#define OBJ_FILE_EXT   "obj"
#define XYZ_FILE_EXT   "xyz"

/* function implementations */
		
transmodel_run_settings_t::transmodel_run_settings_t()
{
	/* set default values for this program's files */
	this->scale = 1; /* identity */
	this->translate = 0; /* identity */
	this->plyfiles.clear();
	this->objfiles.clear();
	this->xyzfiles.clear();
}

int transmodel_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program applies rigid "
			"transforms to meshes and pointclouds.  By "
			"specifying a scale and offset, the user can "
			"modify models in a variety of file formats by "
			"applying the transform to each vertex/point.\n\n"
			"Note that two files must always be specified, "
			"where the first file is assumed to be the input "
			"model and the second file is assumed to be the "
			"output model.  The models must be of the same "
			"file format.");
	args.add(SCALE_FLAG, "Specifies the scale to apply to each vertex "
			"or point in this model.  If not specified, a "
			"unit scale is assumed (which is a no-op).\n\n"
			"For example, if you had a pointcloud in meters "
			"that you wanted to convert to millimeters, then "
			"the scale should be 1000.", true, 1);
	args.add(TRANSLATE_FLAG, "Specifies a translation to apply to "
			"each vertex or point in this model.  Note that "
			"the translation is applied after any specified "
			"scale is applied, so it should be in units of "
			"the output.", true, 1);
	args.add_required_file_type(PLY_FILE_EXT, 0,
			"Stanford Polygon Format.  If using this file "
			"format, then two files must be specified, where "
			"the files are assumed in the order <input> "
			"<output>.\n\nNote that only ascii-formatted "
			"files can be converted.");
	args.add_required_file_type(OBJ_FILE_EXT, 0,
			"Wavefront OBJ Format.  If using this file "
			"format, then two files must be specified, where "
			"the files are assumed in the order <input> "
			"<output>.");
	args.add_required_file_type(XYZ_FILE_EXT, 0,
			"XYZ ASCII pointcloud format.  If using this file "
			"format, then two files must be specified, where "
			"the files are assumed in the order <input> "
			"<output>.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	if(args.tag_seen(SCALE_FLAG))
		this->scale = args.get_val_as<double>(SCALE_FLAG);
	if(args.tag_seen(TRANSLATE_FLAG))
		this->translate = args.get_val_as<double>(TRANSLATE_FLAG);
	args.files_of_type(PLY_FILE_EXT, this->plyfiles);
	args.files_of_type(OBJ_FILE_EXT, this->objfiles);
	args.files_of_type(XYZ_FILE_EXT, this->xyzfiles);

	/* verify the input arguments */
	if(this->scale == 0)
	{
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Error " << ret << ": Will not scale by zero."
		     << endl;
		return ret;
	}
	if(this->plyfiles.size() != 0 && this->plyfiles.size() != 2)
	{
		ret = PROPEGATE_ERROR(-3, ret);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Error " << ret << ": If converting PLY files, "
		     << "then exactly two files (<input> <output>) must "
		     << "be specified." << endl;
		return ret;
	}
	if(this->objfiles.size() != 0 && this->objfiles.size() != 2)
	{
		ret = PROPEGATE_ERROR(-4, ret);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Error " << ret << ": If converting OBJ files, "
		     << "then exactly two files (<input> <output>) must "
		     << "be specified." << endl;
		return ret;
	}
	if(this->xyzfiles.size() != 0 && this->xyzfiles.size() != 2)
	{
		ret = PROPEGATE_ERROR(-5, ret);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Error " << ret << ": If converting XYZ files, "
		     << "then exactly two files (<input> <output>) must "
		     << "be specified." << endl;
		return ret;
	}
	if(this->plyfiles.empty() 
			&& this->objfiles.empty() 
			&& this->xyzfiles.empty())
	{
		ret = PROPEGATE_ERROR(-6, ret);
		args.print_usage(argv[0]);
		cerr << "[transmodel_run_settings_t::parse]\t"
		     << "Error " << ret << ": No input given" << endl;
		return ret;
	}

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
