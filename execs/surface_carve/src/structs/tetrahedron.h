#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

/* tetrahedron.h:
 *
 * This file defines the struct and functions
 * associated with tetrahedra created from
 * three scans points and one pose location. */

#include "point.h"
#include "pose.h"

typedef struct tetrahedron
{
	/* pointers to the vertices of this
	 * tetrahedron */
	point_t a;
	point_t b;
	point_t c;
	point_t pose;

} tetrahedron_t;

/* inside_tet:
 *
 *	Checks if the given location is inside the specified tetrahedron.
 */
bool inside_tet(tetrahedron_t& tet, double x, double y, double z);

#endif
