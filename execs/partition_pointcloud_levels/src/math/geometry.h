#ifndef GEOMETRY_H
#define GEOMETRY_H

/* geometry.h:
 *
 * 	This file defines useful functions
 * 	for geometric computing.
 */

#include "../structs/point.h"

/* geom_center:
 *
 * 	Computes center of the specified points.
 *
 * arguments:
 *
 *	r -	Where to store the center
 *	a,b,c -	The points to use to compute the center
 */
void geom_center(point_t& r, point_t& a, point_t& b, point_t& c);

/* geom_orient2D:
 *
 * 	Computes the signed-area of the parallelogram defined by
 * 	angle pqr.
 *
 * 	If this value is positive, then pqr is oriented counter-clockwise,
 * 	if negative, then pqr oriented clockwise.  If zero, then these
 * 	points are colinear.
 */
double geom_orient2D(point_t& p, point_t& q, point_t& r);

/* geom_line_intersect:
 *
 * 	Finds the intersection point between two line segments.
 *
 * arguments:
 *
 * 	(v0,v1) -	Line segment v
 * 	(w0,w1) -	Line segment w
 *
 * return value:
 *
 *	Returns fraction along v that intersection point occurs, so
 *	that the interesction point is x = v0 + (v1-v0)*<return value>.
 *
 *	Returns DBL_MAX if parallel.
 */
double geom_line_intersect(point_t& v0,point_t& v1,point_t& w0,point_t& w1);

#endif
