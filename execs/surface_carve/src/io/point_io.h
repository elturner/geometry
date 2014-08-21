#ifndef POINT_IO_H
#define POINT_IO_H

/* point_io.h:
 *
 * 	Defines functions to read points from various file formats.
 */

#include <istream>
#include <vector>
#include "../structs/point.h"
#include "../structs/pose.h"

using namespace std;

/* readxyz:
 *
 * 	Reads the specified *.xyz ascii file, and stores the points
 *	in the given list.
 *
 *	Each line of the file represents a point, and should be formatted
 *	as:
 *
 *	<x> <y> <z> <r> <g> <b> <id> <time> <seriel>
 *
 * 	Assumes that x,y,z are given in units of millimeters.
 *
 * arguments:
 *
 * 	filename -	The local path to *.xyz file
 *
 * 	pts -		Where to store the point-cloud.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int readxyz(char* filename, vector<point_t>& pts);

/* readxyz_to_pose:
 *
 * 	Reads the *.xyz.  Stores points in pose-list provided.
 *
 * arguments:
 *
 * 	filename -		The local path to *.xyz file
 * 	pl -			Pose-list (sorted) to store each point.
 *	bbox -			Where to store bounding box information
 *				about these points.
 *	laser_pos -		The position of the laser that generated
 *				this point-cloud, in the coordinate
 *				frame of the pose.
 *	downsample_rate -	Only read one point in every this many.
 *	range_limit_sq -	If a point is more than this distance
 *				squared from its respective scanner pose,
 *				truncate the distance.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int readxyz_to_pose(char* filename, vector<pose_t>& pl,
				boundingbox_t& bbox,
				point_t& laser_pos,
				int downsample_rate,
				double range_limit_sq); 

/* readxyz_subset_to_pose:
 *
 * 	Reads the *.xyz file, within the range [start, end).
 * 	Stores the points in the pose-list provided.
 *
 * arguments:
 *
 * 	filename -		The local path to *.xyz file
 * 	start, end -		The range of the file to read.
 * 	pl -			Pose-list (sorted) to store each point.
 *	bbox -			Where to store bounding box information
 *				about these points.
 *	laser_pos -		The position of the laser that generated
 *				this point-cloud, in the coordinate
 *				frame of the pose.
 *	downsample_rate -	Only read one point in every this many.
 *	range_limit_sq -	If a point is more than this distance
 *				squared from its respective scanner pose,
 *				truncate the distance.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 *
 */
int readxyz_subset_to_pose(char* filename, streampos start, streampos end,
				vector<pose_t>& pl, boundingbox_t& bbox,
				point_t& laser_pos,
				int downsample_rate,
				double range_limit_sq);

/* readxyz_index_scans:
 *
 *	Given an *.xyz file whose points are labeled by scan
 *	and where each scan is stored in order, will parse
 *	the file to determine the line numbers where each scan
 *	starts.
 *
 * arguments:
 *
 * 	filename -	The file to read. The points must be labeled
 * 			by scan, and each scan must have all points
 * 			appear together.  The scans must be in temporal
 * 			order.
 *
 * 	sssp -		The scan start seek positions.  Any information
 * 			in this vector will be replaced with this
 * 			output value.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int readxyz_index_scans(char* filename, vector<streampos>& sssp);

#endif
