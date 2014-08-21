#include "carve_dgrid.h"
#include <vector>
#include <math.h>
#include "../structs/dgrid.h"
#include "../structs/pose.h"
#include "../structs/point.h"
#include "../util/error_codes.h"
#include "../util/progress_bar.h"
#include "../util/parameters.h"

using namespace std;

void carve_slice(dgrid_t& g, point_t& curr, point_t& next,
			point_t& c0, point_t& c1, point_t& n0, point_t& n1)
{
	double d, db, dt, dh, dv;
	int i, j, nh, nv;
	point_t pp, sp, bp, tp;

	/*      ^ j/nv
	 *      |
	 *      |       t
	 *   c1 +--------------->+ n1
	 *      ^                ^
	 *     p|                |s
	 *      |                |
	 *   c0 +--------------->+ n0 ---------------> i/nh
	 *              b
	 *
	 * The boundary edges of this slice are represented with
	 * incrementing vectors, in order to interpolate all interior
	 * area.  The above diagram shows the naming scheme for these
	 * vectors.
	 */

	/* first, compute lengths of each edge (roughly) parallel
	 * to the path segment */
	db = sqrt(dist_sq(c0, n0));
	dt = sqrt(dist_sq(c1, n1));

	/* determine the minimum number of steps horizontally
	 * across these edges to guarantee that each step
	 * is no more than a voxel-size. */
	dh = (db > dt) ? db : dt;
	nh = (int) ceil(dh / (g.vs));
	if(nh <= 0)
		nh = 1;

	/* next, we can iterate over the convex outer surface by
	 * using partition of unity. Note that the path segment is also
	 * iterated over by the variable i. */
	for(i = 0; i <= nh; i++)
	{
		/* get points along t and b segments of outer range */
		bp.x = ( (nh-i)*c0.x + (i)*n0.x ) / nh;
		bp.y = ( (nh-i)*c0.y + (i)*n0.y ) / nh;
		bp.z = ( (nh-i)*c0.z + (i)*n0.z ) / nh;
		
		tp.x = ( (nh-i)*c1.x + (i)*n1.x ) / nh;
		tp.y = ( (nh-i)*c1.y + (i)*n1.y ) / nh;
		tp.z = ( (nh-i)*c1.z + (i)*n1.z ) / nh;

		/* determine sampling rate for this vertical segment */
		dv = sqrt(dist_sq(bp, tp));
		nv = (int) ceil(dv / (g.vs));
		if(nv <= 0)
			nv = 1;

		/* pose position along path based on i */
		pp.x = ( (nh-i)*curr.x + (i)*next.x ) / nh;
		pp.y = ( (nh-i)*curr.y + (i)*next.y ) / nh;
		pp.z = ( (nh-i)*curr.z + (i)*next.z ) / nh;
	
		/* iterate vertically for this path position */
		for(j = 0; j <= nv; j++) /* vertical on inner-loop */
		{
			/* bilinear interpolation of scan point */
			sp.x = ( (nv-j)*bp.x + (j)*tp.x ) / nv;
			sp.y = ( (nv-j)*bp.y + (j)*tp.y ) / nv;
			sp.z = ( (nv-j)*bp.z + (j)*tp.z ) / nv;

			/* move scan point a voxel size closer
			 * to the path.  This removes the bias in
			 * carving that creates a surfaces that is
			 * farther out than the actual points */
			d = sqrt(dist_sq(pp, sp));
			d = (d - VOXEL_BIAS_FRACTION*g.vs) / d;
			if(d > 0)
			{
				sp.x = pp.x + d*(sp.x - pp.x);
				sp.y = pp.y + d*(sp.y - pp.y);
				sp.z = pp.z + d*(sp.z - pp.z);
			}

			/* carve from interpolated path to the
			 * interpolated scan.  If there are any
			 * elements in this->points, we want to
			 * watch for them */
			g.carve_segment(pp, sp, false);
		}
	}
}

void carve_scan(dgrid_t& g, pose_t& curr, pose_t& next, int sn)
{
	int ic, in, ic_next, in_next, nc, nn;
	point_t cp, np;

	/* get attributes of each scan */
	nc = curr.scans[sn].size();
	nn = next.scans[sn].size();

	/* Initialize starting indices */
	ic = 0;
	in = 0;

	/* iterate over the two scan lines */
	while( ic < nc-1 || in < nn-1 )
	{
		/* get next indices on both sides */
		ic_next = ic + 1;
		in_next = in + 1;

		/* TODO, try to reduce skew when selecting indices */

		/* make sure not to go out of bounds */
		if(ic_next >= nc)
			ic_next = ic;
		if(in_next >= nn)
			in_next = in;

		/* get the pose locations as points */
		cp.x = curr.x ; cp.y = curr.y ; cp.z = curr.z;
		np.x = next.x ; np.y = next.y ; np.z = next.z;

		/* to be safe, carve the path segment itself */
		g.carve_segment(cp, np, true);

		/* also carve from each pose to the laser position */
		g.carve_segment(cp, curr.laser_pos[sn], true);
		g.carve_segment(np, next.laser_pos[sn], true);

		/* carve slice */
		carve_slice(g, curr.laser_pos[sn], next.laser_pos[sn], 
			curr.scans[sn][ic], curr.scans[sn][ic_next],
			next.scans[sn][in], next.scans[sn][in_next]);
		
		/* increment indices */
		ic = ic_next;
		in = in_next;
	}
}

void carve_path(dgrid_t& g, vector<pose_t>& poselist, int begin_pose,
						int num_poses)
{
	unsigned int i, j, k, n, ns;
	int end_pose;

	/* check the begin pose index */
	if(begin_pose < 0)
		begin_pose = 0;

	/* iterate over all poses and carve each segment
	 * of the path */
	end_pose = begin_pose + num_poses;
	if(num_poses < 0 || end_pose >= (int) poselist.size())
		n = (poselist.size() - 1);
	else
		n = (unsigned int) end_pose;

	/* leave space for a progress bar */
	reserve_progress_bar();

	/* iterate over specified portion of path */
	for(i = begin_pose; i < n; i++)
	{
		/* user is impatient */
		progress_bar("carving", ((double) (i - begin_pose)) 
						/ (n - begin_pose));

		/* iterate over the scanners in this pose */
		ns = poselist[i].scans.size();
		for(k = 0; k < ns; k++)
		{
			/* check that we have sufficient scans */
			if(poselist[i].scans[k].empty())
				continue;

			/* get next non-empty pose */
			for(j = i+1; j <=n && poselist[j].scans[k].empty(); 
					j++) {}	
			if(j > n)
				break;

			/* run current scan */
			carve_scan(g, poselist[i], poselist[j], k);
		}
	}

	/* remove progress bar from screen */
	delete_progress_bar();
}
