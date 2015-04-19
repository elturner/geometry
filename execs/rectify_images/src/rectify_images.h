#ifndef H_RECIFYIMAGES_H
#define H_RECIFYIMAGES_H

/*
	rectify_images.h
*/

/* includes */
#include <string>

/* rectify_images namespace elements */
namespace rectify_images
{

	/* class to hold the input parameters */
	class InParams
	{
	public:

		/* this holds the dataset root directory */
		std::string datasetDirectory;

		/* holds the input metadata file name */
		std::string metaDataFile;

		/* holds the input camera calibration dat file */
		std::string cameraCalibrationFile;

		/* holds the image mask file name */
		std::string imageMaskFile;

		/* holds the rotation from actual camera to virtual camera in radians */
		double rVcam[3];

		/* holds the actual camera to common transform in radians and millimeters */
		double eTransform[6];

		/* holds the K matrix in row column order */
		double KMatrix[9];

		/* holds the desired image size */
		size_t imgSize[2];

		/* holds the name of the desired output directory which is relative */
		/* to the folder containing the inputMetaDataFile */
		std::string outputDirectory;

		/* holds the desired vcam serial number */
		std::string vcamSerialNumber;

		/* number of threads to use */
		size_t numThreads;
	};

	/*
		int run(const InParams& params)

		Runs the rectification process.  See above for a description of the
		parameters to the code.

		Returns zero on success and non-zero on failure
	*/
	int run(const InParams& params);


}

#endif