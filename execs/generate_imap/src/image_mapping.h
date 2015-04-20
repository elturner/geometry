#ifndef H_IMAGE_MAPPING_H
#define H_IMAGE_MAPPING_H

/*
	image_mapping.h this file supports the image mapping functionality
*/

/* includes */
#include <string>
#include <vector>

/* the image mapping namespace */
namespace ImageMapping
{
	/*
		int map_images(const std::string& datasetDir,
			const std::vector<std::string>& poseFiles,
			const std::vector<std::string>& depthmaps,
			cosnt std::vector<std::string>& normalmaps,
			const std::string& imapFileName,
			const std::string& keyFileName,
			double resolution);

		The main entry point of the image mapping code
	*/
	int map_images(const std::string& datasetDir,
		const std::vector<std::string>& poseFiles,
		const std::vector<std::string>& depthmaps,
		const std::vector<std::string>& normalmaps,
		const std::string& imapFileName,
		const std::string& keyFileName,
		double resolution);
}

#endif