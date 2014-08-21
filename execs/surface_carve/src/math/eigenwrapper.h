#ifndef EIGEN_WRAPPER_H
#define EIGEN_WRAPPER_H

/* eigenwrapper.h:
 *
 * This file contains functions that will
 * use the Eigen library to compute matrix
 * operations.
 */

/* svd3_min_vect:
 *
 * 	Will compute the SVD decomposition
 * 	of the specified 3x3 matrix.  Will
 * 	return the eigenvector corresponding
 * 	to the minimum eigenvalue.
 *
 * arguments:
 *
 * 	vec -	Where to store the found vector
 * 	mat -	The 3x3 matrix, in row-major order.
 *
 * return value:
 *
 * 	returns 0 on success, non-zero on failure
 */
int svd3_min_vect(double* vec, double* mat);

#endif
