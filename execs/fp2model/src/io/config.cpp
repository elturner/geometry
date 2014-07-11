#include "config.h"
#include <util/cmd_args.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file config.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Configuration parameters for program
 *
 * Represents the functions used
 * to read command-line arguments.
 */

using namespace std;

/* desired file formats */

#define FLOORPLAN_FILE_EXT "fp"
#define WINDOWS_FILE_EXT   "windows"

#define OBJ_FILE_EXT       "obj"
#define IDF_FILE_EXT       "idf"
#define WRL_FILE_EXT       "wrl"
#define CSV_FILE_EXT       "csv"

/* function implementations */

int config_t::parse(int argc, char** argv)
{
	vector<string> fp_infile_list;
	cmd_args_t args;
	int ret;

	/* set default config */
	this->fp_infile = "";
	this->windows_infiles.clear();
	this->outfile_obj.clear();
	this->outfile_idf.clear();
	this->outfile_wrl.clear();

	/* prepare command-args parser */
	args.set_program_description("This program is used to convert "
		"floorplan geometry that is represented in .fp files to "
		"other formats.");
	args.add_required_file_type(WINDOWS_FILE_EXT, 0, "Specifies "
		"location of windows relative to the given floorplan.");
	args.add_required_file_type(FLOORPLAN_FILE_EXT, 1, "Specifies "
		"floorplan geometry to convert and export.  If multiple "
		"files are given, only the first will be used.");
	args.add_required_file_type(OBJ_FILE_EXT, 0, "If present, then "
		"will export floorplan geometry to the specified Wavefront "
		"OBJ file, which represents the triagulation mesh.");
	args.add_required_file_type(IDF_FILE_EXT, 0, "If present, then "
		"will export floorplan geometry to the specified EnergyPlus"
		" Input Data File (IDF), which represents the building "
		"information, including rooms, windows, and "
		"constructions.");
	args.add_required_file_type(WRL_FILE_EXT, 0, "If present, then "
		"will export floorplan geometry to the specified Virtual "
		"Reality Modeling Language (VRML), which stores the model "
		"as a set of of meshed surfaces.");
	args.add_required_file_type(CSV_FILE_EXT, 0, "If present, then "
		"will export floorplan statistical information to the "
		"specified comma-separated-variable file, which can be "
		"viewed in a spreadsheet program, such as excel.");

	/* parse the args */
	ret = args.parse(argc, argv);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* retrieve the parsed values */
	args.files_of_type(FLOORPLAN_FILE_EXT, fp_infile_list);
	if(fp_infile_list.empty())
	{
		/* must specify infile! */
		cerr << "[config_t::parse]\tMust specify an input .fp file!"
		     << endl;
		return -2;
	}
	this->fp_infile = fp_infile_list[0];
	args.files_of_type(WINDOWS_FILE_EXT, this->windows_infiles);
	args.files_of_type(OBJ_FILE_EXT, this->outfile_obj);
	args.files_of_type(IDF_FILE_EXT, this->outfile_idf);
	args.files_of_type(WRL_FILE_EXT, this->outfile_wrl);
	args.files_of_type(CSV_FILE_EXT, this->outfile_csv);

	/* success */
	return 0;
}
