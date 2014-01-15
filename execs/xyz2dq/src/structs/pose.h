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

} pose_t;

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

#endif
