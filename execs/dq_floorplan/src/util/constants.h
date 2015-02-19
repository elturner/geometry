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

/* The following represents the threshold used to remove 'sharps'
 * from the generated floorplan during the simplification process.
 *
 * A 'sharp' feature is an acute angle in the floorplan that represents
 * incorrect geometry and is aesthetically displeasing.
 *
 * We remove these features based on the angle of the corner vertex, under
 * the assumption that buildings do not have highly acute angles in
 * their true geometry.
 *
 * This value is the cosine of the threshold angle, which will be used
 * to threshold the SIGNED cosine of adjacent wall edges.
 *
 *  Examples:
 *  	 90 degrees (sharps of 90 deg or less) -->   0
 *  	120 degrees (sharps of 60 deg or less) -->  -0.5
 *  	135 degrees (sharps of 45 deg or less) -->  -0.7071
 */
#define DEFAULT_SHARPS_REMOVAL_THRESHOLD -0.19

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
