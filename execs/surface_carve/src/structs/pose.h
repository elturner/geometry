#ifndef POSE_H
#define POSE_H

/* pose.h:
 *
 * This file contains structs and functions
 * used to manipulate poses of the scan acquisition
 * system.
 *
 * Typically this information was retrieved from a *.mad
 * file */

#include <vector>
#include "point.h"

using namespace std;

/* this struct represents a single pose -- a location in space-time */
typedef struct pose
{
	/* for a single pose,
	 * we are interested in
	 * temporal and spatial position */
	double timestamp;

	/* position specified in ENU coordinates */
	double x; /* meters */
	double y; /* meters */
	double z; /* meters */
	
	/* The following specify the orientation of the scanner
	 * at this pose.  Roll, pitch, yaw are stored as radian
	 * values in NED coordinates. */
	double roll, cr, sr; /* roll, cos(roll), sin(roll) */
	double pitch, cp, sp; /* pitch, cos(pitch), sin(pitch) */
	double yaw, cy, sy; /* yaw, cos(yaw), sin(yaw) */

	/* the points taken during this pose.
	 * The vector scans contains lists of points, each element
	 * scans[i] is a laser scan, composed of many laser points,
	 * taken from a different scanner */
	vector<vector<point_t> > scans;

	/* denotes the position of the laser (in world coordinates) 
	 * for each of the scans referenced in the scans parameter. */
	vector<point_t> laser_pos;
} pose_t;

/* pose_dist_sq:
 *
 *	Returns the square of the distance (spatial)
 *	between two poses.
 */
double pose_dist_sq(pose_t& a, pose_t& b);

/* pose_point_dist_sq:
 *
 *	Returns the square of the distance (spatial)
 *	between a pose and a point.
 */
double pose_point_dist_sq(pose_t& a, point_t& b);

/* pose_point_dist_sq_hori:
 *
 *	Will compute the square of the distance between
 *	a and b, not counting z-direction.
 *
 * arguments:
 *
 * 	a,b - 	The points to analyze
 *
 * return value:
 *
 * 	Returns the square of the horizontal distance
 */
double pose_point_dist_sq_hori(pose_t& a, point_t& b);

/* pose_transform_local_to_world_coords:
 *
 * 	Given a point, x, that is in a coordinate system
 * 	relative to the given pose, will perform a coordinate
 * 	transform, and output x's location in world coordinates
 * 	into the point referenced by y.
 *
 * arguments:
 *
 * 	p -	The pose that represents x's origin.
 * 	y -	The location where the result will be written.  This
 * 		is allowed to be the same reference as x.
 * 	x -	The input point, defined relative to p's coordinate
 * 		frame.
 */
void pose_transform_local_to_world_coords(pose_t& p, 
				point_t& y, point_t& x);

/* poselist_closest_index:
 *
 *	Returns the index of the pose in pl that is closest in
 *	time to the timestamp t.
 *
 * return value:
 *
 * 	On success, returns non-negative index within pl->poses
 * 	On failure, returns negative value
 */
int poselist_closest_index(vector<pose_t>& pl, double t);

/* poselist_clear_points:
 *
 *	Removes all scan points from the given pose list.
 */
void poselist_clear_points(vector<pose_t>& pl);

#endif
