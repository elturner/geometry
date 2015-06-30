/*
	split_image_by_floorplan

	This executable splits an input pointcloud image based on the room 
	assignments found in the floorplan
*/

/* includes */
#include <string>
#include <iostream>

#include <util/error_codes.h>
#include <util/cmd_args.h>

#include "split_image_by_floorplan.h"

/* namespaces */
using namespace std;

/* defines */
#define FLAG_IMAGE "-ii"
#define FLAG_COORDMAPPING "-ic"
#define FLAG_TIMEMAP "-it"
#define FLAG_FLOORPLAN "-if"
#define FLAG_OUTFOLDER "-o"
#define FLAG_BACKGROUNDCOLOR "-b"

/* main function */
int main(int argc, char* argv[])
{

	int ret;

	/* create the argument parser */
	cmd_args_t parser;
	parser.set_program_description("This executable is responsible for "
		"splitting the input pointcloud image based on the room assingments "
		"present in the given floorplan file.  When completed an individual "
		"pointcloud image file and coordinate mapping file will exist for "
		"each room in the floorplan.");
	parser.add(FLAG_IMAGE,
		"The input pointcloud image file. This is the file path of the "
		"pointcloud image file that will be split by room assignments.",
		false,
		1);
	parser.add(FLAG_COORDMAPPING,
		"The input coordinate mapping file for the pointcloud image file. "
		"This file specifies the mapping between image pixels and model "
		"coordinates.",
		false,
		1);
	parser.add(FLAG_TIMEMAP,
		"The input time map image. This file has the timestamps of the points "
		"used to fill the pixel in units of tenths of seconds.",
		false,
		1);
	parser.add(FLAG_FLOORPLAN,
		"The input floorplan file.  This is the file path of the floorplan fp "
		"file that contains the room assignments.",
		false,
		1);
	parser.add(FLAG_OUTFOLDER,
		"Specifies the desired output directory of the pointcloud image "
		"files. This creates the directory if needed. If not given then it "
		"defaults current directory",
		true,
		1);
	parser.add(FLAG_BACKGROUNDCOLOR,
		"Specifies the background color of the images.  This should be an "
		"RGB triplet in the range [0-255]. If not given then it will default "
		"to black.",
		true,
		3);

	/* parse the inputs */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* copy in the inputs */
	sf::AlgSettings settings;
	settings.pointcloud_image_file = parser.get_val(FLAG_IMAGE);
	settings.coord_mapping_file = parser.get_val(FLAG_COORDMAPPING);
	settings.floorplan_file = parser.get_val(FLAG_FLOORPLAN);
	settings.time_map_file = parser.get_val(FLAG_TIMEMAP);
	if(parser.tag_seen(FLAG_OUTFOLDER))
	{
		settings.output_prefix = parser.get_val(FLAG_OUTFOLDER);
	}
	if(parser.tag_seen(FLAG_BACKGROUNDCOLOR))
	{
		settings.backgroundColor[0] 
			= parser.get_val_as<int>(FLAG_BACKGROUNDCOLOR, 0);
		settings.backgroundColor[1] 
			= parser.get_val_as<int>(FLAG_BACKGROUNDCOLOR, 1);
		settings.backgroundColor[2] 
			= parser.get_val_as<int>(FLAG_BACKGROUNDCOLOR, 2);
	}

	/* run the code */
	ret = sf::run(settings);
	if(ret)
		POST_RETURN_ERROR("Image splitting returned error!",ret);

	/* return success */
	return 0;
}