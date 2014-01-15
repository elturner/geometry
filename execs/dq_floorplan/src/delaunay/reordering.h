#ifndef REORDERING_H
#define REORDERING_H

#include "triangulation/triangulation.h"

/* reordering.h
 *
 * This package contains functions that
 * will modify the order of vertices within
 * a triangulation.
 *
 * This step is useful to ensure randomized
 * insertion, or spatially close insertion
 * orders. */

/* reorder_BRIO:
 *
 * 	Will reorder the points in the specified triangulation
 * 	using Biased Randomized Insertion Order, sorting
 * 	each round with a z-order curve.
 *
 * 	The triangulation must not contain any triangles or edges.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int reorder_BRIO(triangulation_t* tri);

/* reorder_randomize:
 *
 * 	Will randomize the order of vertices in the
 * 	triangulation.
 *
 * 	Must only be called if no triangles have been created.
 *
 * arguments:
 *
 * 	tri -	The triangulation to modify
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int reorder_randomize(triangulation_t* tri);

/* reorder_maximize_first_area:
 *
 *	Will (possibly) change which vertex is listed
 *	third so that the area of the first triangle
 *	(made up of the first three vertices listed)
 *	is maximized.
 *
 * arguments:
 *
 * 	tri -	Triangulation to modify
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int reorder_maximize_first_area(triangulation_t* tri);

/* reorder_z_order_sort:
 *
 *	Will perform z-order sorting on a list of vertices
 *
 * arguments:
 *
 * 	list -	The array to sort
 * 	len -	Number of vertices in array
 *
 * note:
 *
 * 	This function is NOT thread-safe.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int reorder_z_order_sort(vertex_t* list, int len);

/* reorder_z_order_comp:
 *
 *	A comparison function for z-order sorting
 *	vertices.
 *
 * arguments:
 *
 * 	pp, qp -	Pointers to vertices being compared.
 *
 * return value:
 *
 * 	<  0	iff	pp <  qp
 * 	== 0	iff	pp == qp
 * 	>  0	iff	pp >  qp
 */
int reorder_z_order_comp(const void* pp, const void* qp);

#endif
