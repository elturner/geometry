#ifndef CONSTANTS_H
#define CONSTANTS_H

/*** GEOMETRY ***/

#define NUM_EDGES_PER_SQUARE 4
#define NUM_VERTS_PER_SQUARE 4

#define NUM_VERTS_PER_TRI 3
#define NUM_EDGES_PER_TRI 3

#define NUM_VERTS_PER_EDGE 2

/*** Approximate Calculations ***/

#define PARALLEL_THRESHOLD 0.95 /* 0.9998 corresponds to ~1 degree */
#define PERPENDICULAR_THRESHOLD 0.09 /* 0.087 corresponds to ~85 degrees */
#define APPROX_ZERO 0.0000001

/*** Simplification ***/

/* the error matrix is a symmetric 3x3 matrix, so only
 * need to store upper triangle, which has six values
 * that are stored in the following order:
 *
 *	0	1	2
 *	-	3	4
 *	-	-	5
 */
#define ERROR_MATRIX_SIZE 6

/* the following is the default threshold used for simplification.
 * Represents the sum of the errors of a point from all lines it is
 * adjacent to. */
#define DEFAULT_SIMPLIFY_THRESHOLD 0.05 /* units: meters */

/*** Visualization ***/

/* the following is the assumption about heights of walls. This is
 * not meant to be a quantitative measurement, just a useful assumption
 * for visualization purposes only. */
#define ASSUMED_WALL_HEIGHT 3.0 /* units: meters */

/* for texturing purposes, walls need to be coalesced based on a
 * minimum distance threshold. That is, we don't want walls that
 * are too short horizontally. */
#define REGION_COALESCE_MIN_WALL_LENGTH 0.05

#endif
