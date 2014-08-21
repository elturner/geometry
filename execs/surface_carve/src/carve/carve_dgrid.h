#ifndef TET_CARVE_DGRID_H
#define TET_CARVE_DGRID_H

/* carve_dgrid.h:
 *
 * 	Uses the scans and poses to create
 * 	tetrahedra with which to carve the
 * 	grid voxels.
 */

#include <vector>
#include "../structs/dgrid.h"
#include "../structs/point.h"
#include "../structs/pose.h"

using namespace std;

/* carve_slice:
 *
 *	Carves the voxels covered by the given slice
 *	using ray-tracing.
 *
 *	A slice represents a single segment of the path, and the
 *	volume enclosed by four adjacent scan points (two from
 *	the first pose, two from the second).
 *
 * arguments:
 *
 * 	g -	The grid to modify
 * 	
 * 	curr, next -	The location of the two poses that compose 
 * 			this slice
 *
 * 	c0, c1 -	The scan points of curr that compose this slice
 * 	n0, n1 -	The scan points of next that compose this slice
 */
void carve_slice(dgrid_t& g, point_t& curr, point_t& next,
			point_t& c0, point_t& c1, point_t& n0, point_t& n1);

/* carve_scan:
 *
 *	Carves the voxels covered by the triangulation between
 *	the current scan and the next scan.
 *
 *	Assumes the scan points for each pose are ordered.
 *
 * arguments:
 *
 * 	g -	The grid to modify
 *	curr -	The current scan
 *	next -	The next scan
 *	sn -	Scanner number of each pose to use.
 */
void carve_scan(dgrid_t& g, pose_t& curr, pose_t& next, int sn);

/* carve_path:
 *
 *	Carves all path segments.
 *
 * arguments:
 *
 * 	g -		The grid to modify
 * 	poselist -	The poses to use to carve
 * 	begin_pose -	The pose at which to begin.  Negative
 * 			value starts at first pose.
 * 	num_poses -	How many poses to use.  Negative value
 * 			uses all poses.
 */
void carve_path(dgrid_t& g, vector<pose_t>& poselist, int begin_pose,
						int num_poses);

#endif
