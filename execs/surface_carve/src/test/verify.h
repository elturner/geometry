#ifndef VERIFY_H
#define VERIFY_H

/* verify.h:
 *
 * This file represents functions that are meant
 * to test invarients of various structures in this
 * code, to make sure that no errors have occurred.
 */

#include "../structs/triangulation.h"

/* verify_triangulation:
 *
 * 	Will check the invariants of the given
 * 	triangulation.  If an error is found, a
 * 	message is printed to the screen describing
 * 	the problem.
 *
 * return value:
 *
 *	Returns true only if the structure passes all checks.
 */
bool verify_triangulation(triangulation_t& tri);

/* verify_triangle:
 *
 * 	Checks invariants for the given triangle.
 *
 * arguments:
 *
 * 	tri -	The triangulation that contains this triangle.
 * 	t -	The triangle to check.
 *
 * return value:
 *
 * 	Returns true only if the triangle passes all checks.
 */
bool verify_triangle(triangulation_t& tri, triangle_t* t);

/* verify_vertex:
 *
 *	Checks invariants for the given vertex.
 *
 * arguments:
 *
 * 	tri -	The triangulation that contains this vertex.
 * 	v -	The vertex to check.
 *
 * return value:
 *
 * 	Returns true only if the triangle passes all checks.
 */
bool verify_vertex(triangulation_t& tri, vertex_t* v);

#endif
