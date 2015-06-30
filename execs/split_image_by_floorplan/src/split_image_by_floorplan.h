#ifndef H_SPLITIMAGEBYFLOORPLAN_H
#define H_SPLITIMAGEBYFLOORPLAN_H

/*
	split_image_by_floorplan

	This splits an input point cloud image by floorplan
*/

/* includes */
#include <string>

/* sf namespace */
namespace sf
{

	class AlgSettings
	{
	public:

		// This is the input pointcloud image file
		std::string pointcloud_image_file;

		// This is the coordinate mapping file
		std::string coord_mapping_file;

		// This is input floorplan file
		std::string floorplan_file;

		// This is the time map file
		std::string time_map_file;

		// This is the desired output prefix
		std::string output_prefix;

		// Sets the background color
		unsigned char backgroundColor[3];

		// Constructor with default arguments
		AlgSettings() :
			pointcloud_image_file(""),
			coord_mapping_file(""),
			floorplan_file(""),
			time_map_file(""),
			output_prefix("") 
		{ 
			backgroundColor[0] = 0;
			backgroundColor[1] = 0;
			backgroundColor[2] = 0;
		};
	};


	/*
		int run(const sf::AlgSettings& settings);

		Runs the code with the settings.  Returns zero on success and non-zero
		on failure
	*/
	int run(const sf::AlgSettings& settings);

}

#endif
