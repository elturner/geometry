#include <math.h>
#include "vec.h"
#include "pcube.h"

/**
 * @file get_polygon_normal.c
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * Finds normal vector of triangle, authored by Don Hatch &
 * Melinda (Daniel) Green, January 1994.
 */

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
real *
get_polygon_normal(real normal[3],
		   int nverts, const real verts[/* nverts */][3])
{
    int i;
    real tothis[3], toprev[3], cross[3];

    /*
     * Triangulate the polygon and sum up the nverts-2 triangle normals.
     */
    ZEROVEC3(normal);
    VMV3(toprev, verts[1], verts[0]);  	/* 3 subtracts */
    for (i = 2; i <= nverts-1; ++i) {   /* n-2 times... */
	VMV3(tothis, verts[i], verts[0]);    /* 3 subtracts */
	VXV3(cross, toprev, tothis);         /* 3 subtracts, 6 multiplies */
	VPV3(normal, normal, cross);         /* 3 adds */
	SET3(toprev, tothis);
    }
    return normal;
}
