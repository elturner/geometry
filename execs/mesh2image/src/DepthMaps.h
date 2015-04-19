#ifndef H_DEPTHMAPS_H
#define H_DEPTHMAPS_H

/*
	DepthMaps.h

	Defines the entry function to the depth map generation code
*/

/* includes */
#include <string>
#include <vector>

namespace DepthMaps
{
	/*
		bool generate_depth_maps(const std::string& datasetDir,
			const std::string& modelFile,
			size_t depth,
			const std::vector<std::string> >& mcdFiles,
			const std::vector<std::string> >& poseFiles,
			const std::vector<std::string> >& outDirs,
			const std::vector<std::string>& cameraTags,
			size_t numThreads,
			double dsFactor);

		Main function for depth map generation.
	*/
	bool generate_depth_maps(const std::string& datasetDir,
		const std::string& modelFile,
		size_t depth,
		const std::vector<std::string>& mcdFiles,
		const std::vector<std::string>& poseFiles,
		const std::vector<std::string>& outDirs,
		const std::vector<std::string>& cameraTags,
		size_t numThreads,
		double dsFactor);

}

#endif