/* 
	recitfy_images

	This executable is responsible for turning the fisheye images
	into rectilinear images
*/

/* includes */
#include <iostream>
#include <string>

#include <util/cmd_args.h>
#include <util/tictoc.h>

#include "rectify_images.h"

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

/* namespaces */
using namespace std;

/* defines */
#define INPUT_DATASETDIR_TAG "-id"
#define INPUT_METADATA_TAG "-im"
#define INPUT_CALIB_TAG "-ic"
#define INPUT_ROTATION_TAG "-ir"
#define INPUT_EXTRINSIC_TAG "-ie"
#define INPUT_KMATRIX_TAG "-ik"
#define INPUT_IMAGESIZE_TAG "-is"
#define INPUT_MASK_TAG "-iv"
#define OUTPUT_DIRECTORY_TAG "-od"
#define OUTPUT_VCAMSERIAL_TAG "-os"
#define NUM_THREADS_TAG "-t"

/* function prototypes */

/*
	void copy_params()

	This function just copies the params from the arg parser into the 
	params class
*/
void copy_params(cmd_args_t& parser, rectify_images::InParams& params);

/* the main function */
int main(int argc, char * argv[])
{
	int ret;

	/* set up an argument parser */
	cmd_args_t parser;
	parser.set_program_description("This program is responsible for generating "
		"rectified images for Peter Cheng's texture mapping code. In addition "
		"it should automatically generate the required .mcd files also "
		"required by the texture mapping code.");
	parser.add(INPUT_DATASETDIR_TAG, "Input data directory. This argument "
		"declares the dataset base directory.", false, 1);
	parser.add(INPUT_METADATA_TAG, "Metadata file. This metadata file "
		"specifies which images we will rectify. This should be the absolute "
		"file name of the desired file.", false, 1);
	parser.add(INPUT_CALIB_TAG, "Ocam calibration .dat file. This should be "
		"the absolute file path of the ocam calibration file for the camera "
		"who's images are being rectified.", false, 1);
	parser.add(INPUT_ROTATION_TAG, "Rotation applied to get from actual "
		"camera coordinates to virtual camera coordinates.  This should "
		"be given as 3 Euler angles in degrees.  The chosen Euler order "
		"convention is 3-2-1.  This means that the overall rotation matrix "
		"is given by:\n"
		"\tR = Rz*Ry*Rx.", false, 3);
	parser.add(INPUT_EXTRINSIC_TAG, "Extrinsic Calibration from actual camera "
		"coordinates to common coordinates for the magneto system.  This is "
		"used for writing the correct transform into the created .mcd file. "
		"Six values should follow this tag.  They should be given in the "
		"following order : roll, pitch, yaw, x, y, z.  Units degrees and "
		"millimeters. Follows 3-2-1 Euler angle convention.", false, 6);
	parser.add(INPUT_KMATRIX_TAG, "Desired K Matrix. Nine values are required "
		"to follow this input.  The 3x3 K matrix should be given in row major "
		"ordering.", false, 9);
	parser.add(INPUT_IMAGESIZE_TAG, "Desired Image Size. Two values are "
		"required. The image size is given in the order height then width. "
		"For example \"" INPUT_IMAGESIZE_TAG " 2000 3000\" would result in a "
		"2000x3000 image.", false, 2);
	parser.add(INPUT_MASK_TAG, "Mask file.  This is the mask file for the "
		"given rotation.  This mask will be correctly scaled for the target "
		"image size automatically and copied into the ouput directory.",
		true, 1);
	parser.add(OUTPUT_DIRECTORY_TAG, "Output directory.  Specifies the desired "
		"output directory.  If it does not exist it will be created if "
		"possible.", false, 1);
	parser.add(OUTPUT_VCAMSERIAL_TAG, "Virtual camera serial number.  This sets "
		"the virtual camera serial number of the output images.", false, 1);
	parser.add(NUM_THREADS_TAG, "Sets the number of threads to use for "
		"undistorting the images.  If not given it will default to the "
		"hardware concurrency reported by the machine.",
		true, 1);

	/* parse */
	ret = parser.parse(argc, argv);
	if(ret)
		return 1;

	/* read out the parameters from the parser */
	rectify_images::InParams params;
	copy_params(parser, params);

	/* run the code */
	ret = rectify_images::run(params);
	if(ret)
	{
		cerr << "[main] - rectify_images::run returned error code " 
			<< ret << endl;
		return 2;
	}

	/* return success */
	return 0;
}

/*
	void copy_params()

	This function just copies the params from the arg parser into the 
	params class
*/
void copy_params(cmd_args_t& parser, rectify_images::InParams& params)
{
	/* copy all the givens */
	params.datasetDirectory = parser.get_val(INPUT_DATASETDIR_TAG, 0);
	params.metaDataFile = parser.get_val(INPUT_METADATA_TAG, 0);
	params.cameraCalibrationFile = parser.get_val(INPUT_CALIB_TAG, 0);
	params.imageMaskFile = parser.get_val(INPUT_MASK_TAG, 0);
	for(size_t i = 0; i < 3; i++)
		params.rVcam[i] = parser.get_val_as<double>(INPUT_ROTATION_TAG, i);
	for(size_t i = 0; i < 6; i++)
		params.eTransform[i] = parser.get_val_as<double>(INPUT_EXTRINSIC_TAG, i);
	for(size_t i = 0; i < 9; i++)
		params.KMatrix[i] = parser.get_val_as<double>(INPUT_KMATRIX_TAG, i);
	for(size_t i = 0; i < 2; i++)
		params.imgSize[i] = parser.get_val_as<size_t>(INPUT_IMAGESIZE_TAG, i);	
	params.outputDirectory = parser.get_val(OUTPUT_DIRECTORY_TAG, 0);
	params.vcamSerialNumber = parser.get_val(OUTPUT_VCAMSERIAL_TAG, 0);

	/* Convert things from degrees to radians as needed */
	const double degToRad = M_PI/180.0;
	for(size_t i = 0; i < 3; i++)
		params.rVcam[i] *= degToRad;
	for(size_t i = 0; i < 3; i++)
		params.eTransform[i] *= degToRad;

	/* deduce number of threads */
	params.numThreads = parser.tag_seen(NUM_THREADS_TAG) ? 
		parser.get_val_as<size_t>(NUM_THREADS_TAG, 0) :
		boost::thread::hardware_concurrency();
}
