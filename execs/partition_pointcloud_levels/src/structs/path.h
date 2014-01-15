#ifndef PATH_H
#define PATH_H

/* path.h:
 *
 * This file contains structs and functions
 * used to manipulate poses of the scan acquisition
 * system.
 *
 * Typically this information was retrieved from a *.mad
 * file */

#include <vector>
#include <string.h>
#include "../math/transform.h"

using namespace std;

/* the following classes are defined in this file */
class path_t;
class pose_t;

/* this class represents a complete path: a list of poses */
class path_t
{
	/*** parameters ***/
	public:

	/* this pose list represents the poses in the path,
	 * in chronological order. */
	vector<pose_t> pl;

	/*** functions ***/
	public:

	/* constructors */
	path_t();
	~path_t();

	/* initializers */
	
	/* readmad:
	 *
	 * 	Reads a *.mad file, and stores results in
	 * 	specified pose list.
	 *
	 *	Will destroy any information currently in this
	 *	object.
	 *
	 * arguments:
	 *
	 * 	filename -	Local path to file to parse
	 *
	 * return value:
	 *
	 *	Returns 0 on success, non-zero on failure.
	 */
	int readmad(char* filename);

	/* closest_index:
	 *
	 *	Returns the index of the pose in pl that is closest in
	 *	time to the timestamp t.
	 *
	 * return value:
	 *
	 * 	On success, returns non-negative index of a pose
	 * 	On failure, returns negative value
	 */
	int closest_index(double t);

};

/* this struct represents a single pose -- a location in space-time */
class pose_t
{
	/*** parameters ***/
	public:

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
	double roll; /* roll */
	double pitch; /* pitch */
	double yaw; /* yaw */
	
	/* the rotation and translation of this scanner with
	 * respect to system origin
	 *
	 *	p_global = rot * p_local + [x;y;z]
	 */
	double rot[ROTATION_MATRIX_SIZE]; /* row-major order */

	/*** functions ***/
	public:

	/* constructors */
	pose_t();
	~pose_t();

	/* compute_transform:
	 *
	 * 	Will use the roll, pitch, and yaw to compute a
	 * 	rotation matrix, stored in this->rot.
	 */
	void compute_transform();

	/* operators */

	inline pose_t operator = (const pose_t& rhs)
	{
		/* copy params */
		this->timestamp = rhs.timestamp;
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
		this->roll = rhs.roll;
		this->pitch = rhs.pitch;
		this->yaw = rhs.yaw;
		memcpy(this->rot, rhs.rot, 
			ROTATION_MATRIX_SIZE*sizeof(double));

		/* return the value of this point */
		return (*this);
	};

	/* geometry */

	/* dist_sq:
	 *
	 *	Returns the square of the distance (spatial)
	 *	between two poses.
	 */
	double dist_sq(pose_t& other);
};

#endif
