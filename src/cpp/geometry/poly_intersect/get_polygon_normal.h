#ifndef POLY_INTERSECT_H
#define POLY_INTERSECT_H

/**
 * @file get_polygon_normal.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * Finds normal vector of triangle, authored by Don Hatch &
 * Melinda (Daniel) Green, January 1994.
 */

#include "pcube.h"

/*
 *  Calculate a vector perpendicular to a planar polygon.
 *  If the polygon is non-planar, a "best fit" plane will be used.
 *  The polygon may be concave or even self-intersecting,
 *  but it should have nonzero area or the result will be a zero vector
 *  (e.g. the "bowtie" quad).
 *  The length of vector will be twice the area of the polygon.
 *  NOTE:  This algorithm gives the same answer as Newell's method
 *  (see Graphics Gems III) but is slightly more efficient than Newell's
 *  for triangles and quads (slightly less efficient for higher polygons).
 */
real* get_polygon_normal(real normal[3],
		   int nverts, const real verts[/* nverts */][3]);

#endif
