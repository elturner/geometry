#include <iostream>
#include <string>
#include <vector>
#include <io/latex/latex_writer.h>
#include <config/backpackConfig.h>
#include <geometry/system_path.h>
#include <mesh/floorplan/floorplan.h>
#include <util/cmd_args.h>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This is the main file for the generate_tex program
 *
 * @section DESCRIPTION
 *
 * This program is used to generate a LaTeX file that describes
 * various features of a given dataset.  The resulting PDF file
 * produced is meant to be a convenient look-up for understanding
 * the contents of a dataset at a glance.
 */

using namespace std;

/*-------------------*/
/* command-line tags */
/*-------------------*/

#define CONFIG_FLAG      "-c"
#define PATH_FLAG        "-p"
#define FLOORPLAN_FLAG   "-f"
#define OUTPUT_FLAG      "-o"

/*-------------------------*/
/* helper function headers */
/*-------------------------*/

/**
 * Initializes the command-line args parser for this program
 *
 * Will populate this parser object with the user-interface usage
 * information, which includes help dialogs.
 *
 * @param args   The object to modify
 */
void init(cmd_args_t& args);

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int main(int argc, char** argv)
{
	cmd_args_t args;
	latex_writer_t outfile;
	backpackConfig conf;
	system_path_t path;
	vector<string> fp_files;
	fp::floorplan_t floorplan;
	unsigned int i, n;
	int ret;

	/* initialize the command-line argument parser */
	init(args);
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse args */
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse command line args" << endl;
		return 1;
	}

	/* open output file */
	ret = outfile.open(args.get_val(OUTPUT_FLAG));
	if(ret)
	{
		/* unable to write to output */
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to open output file for writing" << endl;
		return 2;
	}

	/* get values for any observed inputs */
	
	/* check for config file */
	if(args.tag_seen(CONFIG_FLAG))
	{
		/* write info for this */
		if(!conf.read_config_file(args.get_val(CONFIG_FLAG)))
		{
			/* unable to access file */
			cerr << "[main]\tError: "
			     << "Unable to read/parse xml hardware config "
			     << "file" << endl;
			return 3;
		}

		/* analyze it */
		outfile.write_conf_info(conf);
	}

	/* check for path file */
	if(args.tag_seen(PATH_FLAG))
	{
		/* write info for this */
		ret = path.readmad(args.get_val(PATH_FLAG));
		if(ret)
		{
			/* unable to access file */
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to read/parse input .mad file"
			     << endl;
			return 4;
		}

		/* analyze it */
		outfile.write_path_info(path);
	}

	/* check for floorplan file */
	if(args.tag_seen(FLOORPLAN_FLAG, fp_files))
	{
		/* iterate over all provided floorplans */
		n = fp_files.size();
		for(i = 0; i < n; i++)
		{
			/* write info about it */
			floorplan.clear();
			ret = floorplan.import_from_fp(fp_files[i]);
			if(ret)
			{
				/* unable to access file */
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to read/parse input .fp "
				     << "file" << endl;
				return 5;
			}
		
			/* analyze it */
			outfile.write_floorplan_info(floorplan);
		}
	}

	/* success */
	outfile.close();
	return 0;
}

void init(cmd_args_t& args)
{
	args.set_program_description("This program is used to generate a "
			"LaTeX file that describes various features of a "
			"given dataset.  The resulting PDF file produced "
			"is meant to be a convenient look-up for "
			"understanding the contents of a dataset at a "
			"glance.");
	args.add(CONFIG_FLAG, "Specifies the hardware xml configuration "
			"file used by this dataset.", true, 1);
	args.add(PATH_FLAG, "Specifies the .mad localization path file "
			"generated from this dataset.", true, 1);
	args.add(FLOORPLAN_FLAG, "Specifies the .fp floorplan file "
			"generated from this dataset.  Multiple floorplans"
			" can be provided with multiple instances of this "
			"flag.", true, 1);
	args.add(OUTPUT_FLAG, "Specifies where to write the output .tex "
			"file.", false, 1);
}
