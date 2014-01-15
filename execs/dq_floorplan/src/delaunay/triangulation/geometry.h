#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "vertex.h"

/* geometry.h
 *
 * This package defines functions that are useful
 * for 2D geometry in the plane.  2D points are
 * represented with the vertex_t type.
 */

/* geom_dist_sq:
 *
 * 	Computes the distance between
 * 	two vertices.
 */
double geom_dist_sq(vertex_t* p, vertex_t* q);

/* geom_triangle_area:
 *
 * 	Computes the area of the specified triangle.
 */
double geom_triangle_area(vertex_t* p, vertex_t* q, vertex_t* r);

/* geom_line_intersect:
 *
 *	Given two line segments (v0,v1) and (w0,w1), will determine
 *	if and where they intersect.
 *
 * arguments:
 *
 *	(v0,v1) -	Line segment to analyze
 * 	(w0,w1) -	Other line segment to analyze
 *	x -		If the segments do intersect, will store the
 *			intersection point here.  Ignored if null.
 *
 * return value:
 *
 * 	Returns 0 if no intersection, positive if intersection occurs,
 * 	and negative if error occurs.
 */
int geom_line_intersect(vertex_t* v0, vertex_t* v1, 
				vertex_t* w0, vertex_t* w1, vertex_t* x);

/* geom_orient2D:
 *
 * 	Returns signed-area of parallelogram defined by angle pqr.
 *
 * 	If this value is positive, then pqr are oriented counter-clockwise,
 * 	if negative, then pqr oriented clockwise.  If zero, then these
 * 	points are colinear.
 *
 * return value:
 *
 * 	On success, returns values as described above.  If error occurs,
 * 	returns 0.
 */
double geom_orient2D(vertex_t* p, vertex_t* q, vertex_t* r);

/* geom_incircle:
 *
 * 	Given points p,q, and r, will determine if point s is
 * 	inside the circumcircle described.
 *
 * 	Note p,q,r must be listed in counter-clockwise order.
 *
 * return value:
 *
 * 	If return value is positive, then s is inside circle.
 * 	
 * 	If return value is negative, then s is outside circle.
 * 	
 * 	If return value is zero, then s resides on the circle,
 * 	or an error has occurred.
 */
double geom_incircle(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s);

/* geom_intriangle:
 *
 * 	Given a triangle, will determine if a given point resides inside
 * 	the triangle.
 *
 * arguments:
 *
 * 	p,q,r -	The points of a triangle, in counter-clockwise order.
 * 	s -	The point to examine
 *
 * return value:
 *
 * 	Returns non-zero when s is inside pqr.  Zero if outside.
 */
int geom_intriangle(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s);

/* geom_ontriangleedge:
 *
 * 	Will test if a point lies on the edge of a triangle.
 *
 * arguments:
 *
 * 	p,q,r -	Triangle to test, these points must be listed in counter-
 * 		clockwise order.
 * 	s -	The point to test
 *
 * return value:
 *
 * 	Returns [0,1,2] if s is on the edge opposite [p,q,r], respectively
 * 	Returns 3 if not on triangle edge
 *	Returns negative on error
 */
int geom_ontriangleedge(vertex_t* p, vertex_t* q, vertex_t* r,
							vertex_t* s);

/* geom_center:
 *
 * 	Calculates the average of the vertex positions given, ignores
 * 	any that are null.
 *	
 * 	Stores the average position of p,q,r into s
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int geom_center(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s);

/* geom_inline:
 *
 *	Will return true iff the points p,q,r are colinear, and
 *	q is between p and r.
 */
int geom_inline(vertex_t* p, vertex_t* q, vertex_t* r);

/* geom_circumcenter:
 *
 * 	Computes the circumcenter of the triangle p,q,r.  This
 * 	center is stored in s.
 *
 *	If s is NULL, then will simply return the radius without
 *	modification to arguments.
 *
 * 	The radius of this circumcircle is returned.
 *
 * return value:
 *
 * 	On success, returns radius of circumcircle of p,q,r.
 * 	On failure, returns negative value.
 */
double geom_circumcenter(vertex_t* p, vertex_t* q, vertex_t* r, 
						vertex_t* s);

#endif
