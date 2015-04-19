#include "cam_pose_file.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

/**
 * @file    cam_pose_file.cpp
 * @author  Nick Corso, edited by Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   I/O functionality for camera pose files
 *
 * @section DESCRIPTION
 *
 * Has a representation of camera pose files.  Contains the cam_pose_file_t
 * class, which can read camera pose files.
 */

using namespace std;

/*----------------------*/
/* function definitions */
/*----------------------*/

bool cam_pose_file_t::read(const std::string& filename)
{
	ifstream inStream(filename.c_str());
	if(!inStream.is_open())
		return false;

	string line;
	double data[7];
	while(inStream.good())
	{
		/* read line from file */
		getline(inStream, line);

		/* if its empty continue */
		if(line.empty())
			continue;

		/* throw it in a string stream */
		stringstream ss(line);

		/* pull what we need to out of it */
		for(size_t i = 0; i < 7; i++)
			ss >> data[i];

		/* push the data */
		_timestamps.push_back(data[0]);
		_poses.push_back(Pose(data[0],
			data[1],
			data[2],
			data[3],
			data[4],
			data[5],
			data[6]));
	}

	/* return success */
	return true;
}
