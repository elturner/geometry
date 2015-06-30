#ifndef H_SCREENSHOTPOINTCLOUD_H
#define H_SCREENSHOTPOINTCLOUD_H

/*
	screenshot_pointcloud.h

	Specifies the settings structure and entry function for the 
	screenshot_pointcloud code
*/

/* includes */
#include <string>

/* sp namespace */
namespace sp
{

	/* the AlgSettings class */
	class AlgSettings
	{
	public:

		// The name of the input point cloud
		std::string inFile;

		// The name of the output image file
		std::string outImgFile;

		// The name of the output coordinate file
		std::string outCoordFile;

		// The name of the output time mapping file
		std::string outTimeFile;

		// The unit converstion to meters for the point cloud file
		double unitConversion;

		// The resolution of the generated image
		double imageResolution;

		// Specifies the background color
		unsigned char backgroundColor[3];

		// Flag if we are going to ignore uncolored points
		bool ignoreUncolored;

		AlgSettings() :
			inFile(""),
			outImgFile("image.png"),
			outCoordFile("coordinate_mapping.txt"),
			outTimeFile("time_map.png"),
			unitConversion(1.0),
			imageResolution(0.1),
			ignoreUncolored(false) 
		{
			backgroundColor[0] = 0;
			backgroundColor[1] = 0;
			backgroundColor[2] = 0;
		};

	};


	/* 
		int run(const AlgSettings& settings)

		Runs the screenshot generation code
	*/
	int run(const AlgSettings& settings);
}

#endif
