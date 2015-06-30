/*
	screenshot_pointcloud

	This executable programmatically creates screenshots of a given point cloud
*/

/* includes */
#include <iostream>
#include <string>

#include <util/cmd_args.h>
#include <util/error_codes.h>

#include "screenshot_pointcloud.h"

/* namespaces */
using namespace std;

/* defines */
#define FLAG_INPUT "-i"
#define FLAG_OUTPUT "-o"
#define FLAG_UNITS "-u"
#define FLAG_RESOLUTION "-r"
#define FLAG_BACKGROUNDCOLOR "-b"
#define FLAG_IGNOREUNCOLORED "--ignore_uncolored"

/* the main function */
int main(int argc, char * argv[])
{

	int ret;

	/* create the argument parser */
	cmd_args_t parser;
	parser.set_program_description("This program programmatically generates "
		"a screen shot of a point cloud.");
	parser.add(FLAG_INPUT,
		"Specifies the file path of the input point cloud file. This "
		"currently supports the following point cloud formats:\n"
		"\t.xyz",
		false,
		1);
	parser.add(FLAG_OUTPUT,
		"Specifies the desired output file names.  This flag expects three "
		"inputs:\n"
		"\tArg1 : Output image filename.\n"
		"\tArg2 : Output coordinate mapping file.\n"
		"\tArg3 : Output time mapping file.\n"
		"\n"
		"If not given then these will default to \"image.png\", "
		"\"coordinate_mapping.txt\", and \"time_map.png\"",
		true,
		3);
	parser.add(FLAG_UNITS,
		"Specifies the conversion from point cloud units to meters. If not "
		"given then this defaults to 1.",
		true,
		1);
	parser.add(FLAG_RESOLUTION,
		"Specifies the size of a pixel in the generated image in meters. If "
		"this is not given then it defaults to 0.1 meters.",
		true,
		1);
	parser.add(FLAG_BACKGROUNDCOLOR,
		"Specifies the background color. This flag expects an RGB triplet in "
		"the range [0 255]. If not given it will default to black.",
		true,
		3);
	parser.add(FLAG_IGNOREUNCOLORED,
		"Instructs the code to ignore all points that are uncolored.");

	/* parse */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* load the settings into the settings structure */
	sp::AlgSettings settings;
	settings.inFile = parser.get_val(FLAG_INPUT);
	if(parser.tag_seen(FLAG_OUTPUT))
	{
		settings.outImgFile = parser.get_val(FLAG_OUTPUT, 0);
		settings.outCoordFile = parser.get_val(FLAG_OUTPUT, 1);
		settings.outTimeFile = parser.get_val(FLAG_OUTPUT, 2);
	}
	if(parser.tag_seen(FLAG_UNITS))
	{
		settings.unitConversion = parser.get_val_as<double>(FLAG_UNITS);
	}
	if(parser.tag_seen(FLAG_RESOLUTION))
	{
		settings.imageResolution = parser.get_val_as<double>(FLAG_RESOLUTION);
	}
	settings.ignoreUncolored = parser.tag_seen(FLAG_IGNOREUNCOLORED);
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
	ret = sp::run(settings);
	if(ret)
		POST_RETURN_ERROR("Error generating pointcloud screenshot.",ret);

	/* return success */
	return 0;
}
