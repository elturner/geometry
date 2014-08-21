#include "pose.h"

#include <vector>
#include "../util/error_codes.h"

using namespace std;

double pose_dist_sq(pose_t& a, pose_t& b)
{
	double x, y, z;

	/* compute distance */
	x = a.x - b.x;
	y = a.y - b.y;
	z = a.z - b.z;
	return x*x + y*y + z*z;
}

double pose_point_dist_sq(pose_t& a, point_t& b)
{
	double x, y, z;

	/* compute distance */
	x = a.x - b.x;
	y = a.y - b.y;
	z = a.z - b.z;
	return x*x + y*y + z*z;	
}

double pose_point_dist_sq_hori(pose_t& a, point_t& b)
{
	double dx, dy;

	/* compute distance */
	dx = (a.x) - (b.x);
	dy = (a.y) - (b.y);
	return (dx*dx) + (dy*dy);
}

void pose_transform_local_to_world_coords(pose_t& p, 
				point_t& y, point_t& x)
{
	double a, b, c;

	/* This transformation is defined by the matrix equation:
	 *
	 * 	y = R_pose2world * x + p
	 *
	 * where the matrix R_pose2world is a rotation matrix based on
	 * the pose's orientation.  Since the angles of the pose are
	 * stored in NED coordinates, this rotation matrix is:
	 *
	 * [ cp*sy, cr*cy + sp*sr*sy, cr*sp*sy - cy*sr ;
	 *   cp*cy, cy*sp*sr - cr*sy, cr*cy*sp + sr*sy ;
	 *      sp,       -cp*sr,     -cp*cr           ];
	 */
	
	/* compute the value of y, but in temporary variables, for
	 * the case where x and y are referencing the same point */
	a = (p.cp*p.sy)*x.x + (p.cr*p.cy+p.sp*p.sr*p.sy)*x.y 
			+ (p.cr*p.sp*p.sy-p.cy*p.sr)*x.z + p.x;

	b = (p.cp*p.cy)*x.x + (p.cy*p.sp*p.sr-p.cr*p.sy)*x.y
			+ (p.cr*p.cy*p.sp+p.sr*p.sy)*x.z + p.y;

	c = (p.sp)*x.x - (p.cp*p.sr)*x.y - (p.cp*p.cr)*x.z + p.z;

	/* store final result in y */
	y.x = a;
	y.y = b;
	y.z = c;
}

int poselist_closest_index(vector<pose_t>& pl, double t)
{
	unsigned int low, high, mid, last;
	double comp;

	/* check arguments */
	if(pl.empty())
		return -1;

	/* check outer bounds */
	if(t < pl[0].timestamp)
		return 0;
	last = pl.size() - 1;
	if(t > pl[last].timestamp)
		return last;

	/* assume poses are in order, and perform binary search */
	low = 0;
	high = pl.size();
	while(low <= high)
	{
		/* check middle of range */
		mid = (low + high)/2;
		comp = pl[mid].timestamp - t;

		/* subdivide */
		if(comp < 0)
			low = mid + 1;
		else if(comp > 0)
		{
			high = mid - 1;
			
			/* check if high is now less than t */
			if(mid != 0 && pl[high].timestamp < t)
			{
				low = high;
				break;
			}
		}
		else
			return mid; /* exactly the right time */
	}

	/* we know t falls between indices low and low+1 */
	if(t - pl[low].timestamp > pl[low+1].timestamp - t)
		return low+1;
	return low;
}

void poselist_clear_points(vector<pose_t>& pl)
{
	int i, n;

	/* iterate over pose list */
	n = pl.size();
	for(i = 0; i < n; i++)
	{
		/* remove all laser information */
		pl[i].scans.clear();
		pl[i].laser_pos.clear();
	}
}
