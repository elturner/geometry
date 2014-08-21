#ifndef NORMAL_H
#define NORMAL_H

/* normal.h:
 *
 * 	This file contains structs and functions
 * 	used for finding normal vectors of objects
 * 	in triangulations.
 */

#include "triangulation.h"
#include "point.h"

/* This struct defines a normal vector */
typedef struct normal
{
	/* direction (assumed to be unit-length) */
	double x;
	double y;
	double z;

} normal_t;

/* normal_of_tri:
 *
 * 	Computes the normal vector of the given triangle in the
 * 	given triangulation.
 *
 * arguments:
 *
 * 	norm -	Where to store the computed normal.
 * 	t -	The triangle to analyze.
 */
void normal_of_tri(normal_t& norm, triangle_t* t);

/* normal_average:
 *
 * 	Computes the average direction between two
 * 	normal directions.  Undefined if a == -b.
 *
 * 	avg is allowed to be the same as either a or b.
 *
 * arguments:
 *
 * 	avg -	Where to store the average normal.
 * 	a, b -	The normals of which to compute the average.
 * 	aw,bw -	How much to weight a and b, respectively.
 */
void normal_average(normal_t& avg, normal_t& a, double aw, 
					normal_t& b, double bw);

/* height_from_plane:
 *
 *	Given a point and the definition of a plane, will compute
 *	the distance of that point from the specified plane.
 *
 * arguments:
 *
 * 	p -	The point to analyze
 * 	pn -	The normal of the plane
 * 	pp -	A location on the surface of the plane
 *
 * return value:
 *
 * 	Returns signed distance of p from plane.
 */
double height_from_plane(point_t& p, normal_t& pn, point_t& pp);

/* project_point_to_plane:
 *
 *	Given a point r and a plane p, will find the closest point
 *	s to r that lies on p.
 *
 *	Note r and s can reference the same point_t.
 *
 * arguments:
 *
 * 	s -	Where to store the result
 * 	r -	The point to project onto the plane
 * 	pn -	The normal of the plane p
 * 	pp -	Any point on the surface of p
 */
void project_point_to_plane(point_t& s, point_t& r, 
				normal_t& pn, point_t& pp);

/* project_point_to_plane_plane:
 *
 *	Given a point specified by src, will project src onto
 *	the closest location given by the line of intersection
 *	between the planes p and q, each defined by a normal and
 *	point on plane.
 *
 * 	Note dest and src can reference the same location in memory.
 *
 * arguments:
 *
 * 	dest -	Where to store the result
 * 	src -	The point to project
 * 	n1 -	The normal of the plane p
 * 	p1 -	A point on the plane p
 * 	n2 -	The normal of the plane q
 * 	p2 -	A point on the plane q
 */
void project_point_to_plane_plane(point_t& dest, point_t& src,
					normal_t& n1, point_t& p1,
					normal_t& n2, point_t& p2);

/* intersect_three_planes:
 *
 * 	Finds the intersection of three planes.  If two
 * 	or more of the planes are parallel, behavior is
 * 	undefined.
 *
 * arguments:
 *
 * 	dest -	Where to store the point of intersection.
 * 	n1,p1 -	Definition of first plane
 * 	n2,p2 -	Definition of second plane
 * 	n3,p3 - Definition of third plane
 */
void intersect_three_planes(point_t& dest, normal_t& n1, point_t& p1,
					normal_t& n2, point_t& p2,
					normal_t& n3, point_t& p3);

/* normal_dot:
 *
 *	Computes the dot-product of two normal vectors.
 *
 * arguments:
 * 
 * 	n1,n2 -	The normal vectors to dot-product.
 *
 * return value:
 *
 * 	Returns dot-product of n1 and n2.
 */
#define NORMAL_DOT(n1,n2) ( ( (n1).x * (n2).x ) \
				+ ( (n1).y * (n2).y ) \
				+ ( (n1).z * (n2).z ) )

/* NORMAL_MAGNITUDE:
 *
 *	Computes the mangitude of a normal vector.
 */
#define NORMAL_MAGNITUDE(n) (sqrt((n).x*(n).x + (n).y*(n).y + (n).z*(n).z))

#endif
