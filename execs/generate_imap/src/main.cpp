/*
	create_image_mapping

	This executable creates a rank ordered list of images that can see each
	xy position in a floorplan
*/

/* includes */
#include <string>
#include <iostream>
#include <vector>
#include <util/cmd_args.h>
#include "image_mapping.h"

/* namespaces */
using namespace std;

/* defines */
#define FLAG_DATASETDIR "-dir"
#define FLAG_SPEC "-i"
#define FLAG_RESOLUTION "-r"
#define FLAG_OUTPUTFILE "-o"

/* main function */
int main(int argc, char * argv[])
{
	int ret;

	/* create an argument parser */
	cmd_args_t parser;
	parser.add(FLAG_DATASETDIR, 
		"Defines the input data set directory for the data set.  This "
		"should be the directory created by the data aqusition program.",
		false, 1);
	parser.add(FLAG_SPEC,
		"Defines the input sets.  The input sets should be given in triplet "
		"sets.  The first argument is the full file path to the camera pose "
		"file.  The second is the depth map log file for the camera.  The third "
		" is the normal map log file for the camera",
		true, 3);
	parser.add(FLAG_RESOLUTION,
		"Sets the resolution of the output image map in meters.",
		false, 1);
	parser.add(FLAG_OUTPUTFILE,
		"Sets the desired name of the output files.  The first is the name "
		"of the imap file and the second is the name of the key file.",
		false, 2);

	/* parse the arguments */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* copy the arguments out of the parser */
	string datasetDir = parser.get_val(FLAG_DATASETDIR);
	double resolution = parser.get_val_as<double>(FLAG_RESOLUTION);
	string imapFile = parser.get_val(FLAG_OUTPUTFILE, 0);
	string keyFile = parser.get_val(FLAG_OUTPUTFILE, 1);
	vector<string> specs;
	if(!parser.tag_seen(FLAG_SPEC, specs))
	{
		cerr << "No input triplets given.  Aborting." << endl;
		return 0;
	}

	vector<string> poseFiles;
	vector<string> depthLogs;
	vector<string> normalLogs;
	for(size_t i = 0; i < specs.size(); i+=3)
	{
		poseFiles.push_back(parser.get_val(FLAG_SPEC,i));
		depthLogs.push_back(parser.get_val(FLAG_SPEC,i+1));
		normalLogs.push_back(parser.get_val(FLAG_SPEC,i+2));
	}

	/* run the code */
	ret = ImageMapping::map_images(datasetDir,
		poseFiles,
		depthLogs,
		normalLogs,
		imapFile,
		keyFile,
		resolution);
	if(ret)
	{
		cerr << "Image mapping failed with error code : " << ret << endl;
		return 2; 
	}

	/* return success */
	return 0;
}
