#ifndef TRANSFORM_H
#define TRANSFORM_H

/* transform.h:
 *
 * 	This file specifies functions used for
 * 	3D rigid-body transformations
 */

#include "../structs/point.h"
#include "../util/parameters.h"

/* the following specify the format of matrices and vectors */
#define ROTATION_MATRIX_SIZE    (NUM_DIMS*NUM_DIMS)
#define TRANSLATION_VECTOR_SIZE  NUM_DIMS

/* affine_transform:
 * 
 * 	Performs an affine transform on a point.
 *	Computes the following:
 *
 *		y = R*x + T
 *
 * arguments:
 *
 * 	y -	Where to store the result (this can be the same point as x)
 * 	R -	The rotation matrix, stored as a double array in row-major
 * 		order, of length ROTATION_MATRIX_SIZE
 * 	x -	The input point (this can be the same point as y)
 * 	T -	The translation vector, of length TRANSLATION_VECTOR_SIZE
 */
void affine_transform(point_t& y, double* R, point_t& x, double* T);

#endif
