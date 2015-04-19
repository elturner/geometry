
/*
	depth_maps

	This executable is responsible for creating depth maps and normal
	maps for a set of images.
*/

/* includes */
#include <iostream>
#include <string>
#include <util/cmd_args.h>

#include <boost/threadpool.hpp>

#include "DepthMaps.h"

/* namespaces */
using namespace std;

/* defines */
#define FLAG_DATASETDIR "-dir"
#define FLAG_MODEL "-model"
#define FLAG_SPEC "-i"
#define FLAG_DEPTH "-depth"
#define FLAG_NUMTHREADS "-threads"
#define FLAG_DOWNSAMPLE "-ds"

/* main function */
int main(int argc, char* argv[])
{	
	double dsFactor;
	size_t octreeDepth;
	size_t numThreads;	
	int ret;

	/* set up the argument parser */
	cmd_args_t parser;
	parser.add(FLAG_DATASETDIR, 
		"Defines the input data set directory for the data set.  This "
		"should be the directory created by the data aqusition program.",
		false, 1);
	parser.add(FLAG_MODEL,
		"The full file path of the input model file.  The program supports "
		"parsing of .ply and .obj files currently.",
		false, 1);
	parser.add(FLAG_SPEC,
		"Specifies four file names that define a ray tracing problem. "
		"The first argument is the full file of the .mcd file specifying the "
		"image files.  The second argument is the full file of the camera pose "
		"file.  The third argument is the directory where the output folders "
		"and files are to be stored.  If this folder does not exist then it "
		"will be created.  The fourth is a camera name tag for the rectified "
		"images.",
		true, 4);
	parser.add(FLAG_DEPTH,
		"Specifies the depth of the OctTree used in ray tracing.  "
		"If not specified, will set to value of 10.  This is a "
		"trade-off between memory and processing time.",
		true, 1);
	parser.add(FLAG_NUMTHREADS,
		"Specifies the number of threads used.",
		true, 1);
	parser.add(FLAG_DOWNSAMPLE,
		"Specifies the downsampling factor that will be applied to the output "
		"images.",
		true, 1);

	/* parse the arguments */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* copy out the arguments */
	string datasetDir = parser.get_val(FLAG_DATASETDIR);
	string modelFile = parser.get_val(FLAG_MODEL);
	vector<string> inPairs;
	parser.tag_seen(FLAG_SPEC, inPairs);

	/* look for optional flags */
	if(parser.tag_seen(FLAG_DEPTH))
		octreeDepth = parser.get_val_as<size_t>(FLAG_DEPTH);
	else
		octreeDepth = 10;
	if(parser.tag_seen(FLAG_NUMTHREADS))
		numThreads = parser.get_val_as<size_t>(FLAG_NUMTHREADS);
	else
		numThreads = boost::thread::hardware_concurrency();
	if(parser.tag_seen(FLAG_DOWNSAMPLE))
		dsFactor = parser.get_val_as<double>(FLAG_DOWNSAMPLE);
	else
		dsFactor = 1;

	/* check if any inputs are given */
	if(inPairs.empty())
	{
		cout << "No Image Sets Given.  Terminating." << endl;
		return 0;
	}

	/* split the inputs into the correct file types */
	vector<string> mcdFiles;
	vector<string> poseFiles;
	vector<string> outDirs;
	vector<string> camTags;
	for(size_t i = 0; i < inPairs.size(); i+=4)
	{
		mcdFiles.push_back(inPairs[i]);
		poseFiles.push_back(inPairs[i+1]);
		outDirs.push_back(inPairs[i+2]);
		camTags.push_back(inPairs[i+3]);
	}

	 /* run the depth map generation code */
	if(!DepthMaps::generate_depth_maps(datasetDir,
		modelFile,
		octreeDepth,
		mcdFiles,
		poseFiles,
		outDirs,
		camTags,
		numThreads,
		dsFactor))
	{
		cerr << "Depth Map Generation Failed" << endl;
		return 2;
	}

	/* return success */
	return 0;
}
