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
#define LIGHTS_FILE_EXT    "lights"
#define PEOPLE_FILE_EXT    "people"
#define PLUGLOADS_FILE_EXT "plugloads"

#define OBJ_FILE_EXT       "obj"
#define IDF_FILE_EXT       "idf"
#define WRL_FILE_EXT       "wrl"
#define CSV_FILE_EXT       "csv"
#define PLY_FILE_EXT       "ply"
#define SHP_FILE_EXT       "shp"

/* function implementations */

int config_t::parse(int argc, char** argv)
{
	vector<string> fp_infile_list;
	cmd_args_t args;
	int ret;

	/* set default config */
	this->fp_infile = "";
	this->windows_infiles.clear();
	this->lights_infiles.clear();
	this->people_infiles.clear();
	this->plugloads_infiles.clear();
	this->outfile_obj.clear();
	this->outfile_idf.clear();
	this->outfile_wrl.clear();
	this->outfile_ply.clear();
	this->outfile_shp.clear();

	/* prepare command-args parser */
	args.set_program_description("This program is used to convert "
		"floorplan geometry that is represented in .fp files to "
		"other formats.");
	args.add_required_file_type(WINDOWS_FILE_EXT, 0, "Specifies "
		"location of windows relative to the given floorplan.");
	args.add_required_file_type(LIGHTS_FILE_EXT, 0, "Specifies "
		"light power usages for each room.");
	args.add_required_file_type(PEOPLE_FILE_EXT, 0, "Specifies "
		"the number of people occupying each room.");
	args.add_required_file_type(PLUGLOADS_FILE_EXT, 0, "Specifies "
		"wattages of plug loads in each room of the floorplan.");
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
	args.add_required_file_type(PLY_FILE_EXT, 0, "If present, then "
		"will export floorplan in Stanford Polygon format (PLY), "
		"with the additional region information.  This format "
		"is a valid .ply file (viewable in meshlab) and is "
		"required for Peter Cheng's texture-mapping code.");
	args.add_required_file_type(SHP_FILE_EXT, 0, "If present, then "
		"will export floorplan in ESRI Shape File format.  This "
		"format provides a way to represent shapes for database "
		"look-ups.");

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
	args.files_of_type(WINDOWS_FILE_EXT,   this->windows_infiles);
	args.files_of_type(LIGHTS_FILE_EXT,    this->lights_infiles);
	args.files_of_type(PEOPLE_FILE_EXT,    this->people_infiles);
	args.files_of_type(PLUGLOADS_FILE_EXT, this->plugloads_infiles);
	args.files_of_type(OBJ_FILE_EXT, this->outfile_obj);
	args.files_of_type(IDF_FILE_EXT, this->outfile_idf);
	args.files_of_type(WRL_FILE_EXT, this->outfile_wrl);
	args.files_of_type(CSV_FILE_EXT, this->outfile_csv);
	args.files_of_type(PLY_FILE_EXT, this->outfile_ply);
	args.files_of_type(SHP_FILE_EXT, this->outfile_shp);

	/* success */
	return 0;
}
