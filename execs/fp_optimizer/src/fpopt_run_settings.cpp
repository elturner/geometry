#include "fpopt_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   fpopt_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for fpopt program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * fpopt program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FLAG   "-s" /* program-specific settings (.xml) */
#define OCTFILE_FLAG    "-o" /* location of input octree file (.oct) */
#define FPFILE_FLAG     "-f" /* input and output floorplan files */

/* function implementations */
		
fpopt_run_settings_t::fpopt_run_settings_t()
{
	/* set default values for this program's files */
	this->octfile = "";
}

int fpopt_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	vector<string> fpfiles;
	XmlSettings settings;
	string settings_file;
	tictoc_t clk;
	size_t i, n;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program optimizes the geometry "
			"of generated floorplans for a given dataset by "
			"aligning their surfaces with the carvings "
			"described in the given octree file.");
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to adjust the optimization algorithm.", true, 1);
	args.add(OCTFILE_FLAG, "The input octree (.oct) file to parse.  "
			"This file represents the probabilistic carving "
			"of the dataset in the form of an octree.",false,1);
	args.add(FPFILE_FLAG, "Specifies the input and output floorplan "
			"files (.fp).  Multiple instances of this flag can "
			"occur, and each indicates the <input> and <output>"
			" .fp files, in order.  These files define the "
			"geometry and room information for floorplans.",
			false, 2);

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[fpopt_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */

	/* get oct file */
	this->octfile = args.get_val(OCTFILE_FLAG);
	
	/* get fp files */
	args.tag_seen(FPFILE_FLAG, fpfiles);
	n = fpfiles.size();
	this->input_fpfiles.clear();
	this->output_fpfiles.clear();
	for(i = 0; i < n; i += 2)
	{
		/* separate input and output files */
		this->input_fpfiles.push_back(  fpfiles[i  ] );
		this->output_fpfiles.push_back( fpfiles[i+1] );
	}

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
