#ifndef VERTEX_H
#define VERTEX_H

/* vertex.h
 *
 * Defines the struct used
 * to store the position
 * of a 2D vertex */

#include "../../structs/parameters.h"

#define VERTEX_X_IND 0
#define VERTEX_Y_IND 1

typedef struct vertex
{
	/* the (x,y) position
	 * in 2D space */
	double pos[NUM_DIMS];

	/* The vertices may be reordered
	 * during computation.  This stores
	 * the index number from the original
	 * input file. */
	void* orig_data;

	/* vertices can be sorted based on a
	 * z-order curve.  The following value
	 * represents the indexing for this sort. */
	unsigned int z_order_index;
} vertex_t;

/* vertex_set:
 *
 * 	Set the value of a 2D vertex's location
 * 	in space.
 *
 * arguments:
 *
 * 	v -	vertex to modify
 * 	x -	horizontal position
 * 	y -	vertical position
 *
 * return value:
 * 
 * 	Returns 0 on success, non-zero on failure.
 */
int vertex_set(vertex_t* v, double x, double y);

/* vertex_copy:
 *
 * 	Copies the value of one vertex to another.
 *
 * arguments:
 *
 * 	dest -	The vertex to modify
 * 	src -	The vertex whose value will be duplicated
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int vertex_copy(vertex_t* dest, vertex_t* src);

/* vertex_print:
 *
 *	Prints vertex for debugging purposes.
 */
void vertex_print(vertex_t* p);

#endif
