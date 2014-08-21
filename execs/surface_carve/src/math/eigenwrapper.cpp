#include "eigenwrapper.h"
#include <Eigen/Dense>

using namespace Eigen;

int svd3_min_vect(double* vec, double* mat)
{
	Matrix3d A, vects;
	Vector3d vals;
	int i, k;

	/* fill A */
	A <<	mat[0], mat[1], mat[2],
		mat[3], mat[4], mat[5],
		mat[6], mat[7], mat[8];
	
	/* solve for the eigensystem */
	SelfAdjointEigenSolver<Matrix3d> eigensolver(A);
	if(eigensolver.info() != Success)
		return -1;

	/* get the values and vectors */
	vals = eigensolver.eigenvalues();
	vects = eigensolver.eigenvectors();

	/* find the minimum eigenvalue */
	k = 0;
	for(i = 1; i < 3; i++)
		if(vals(i) < vals(k))
			k = i;

	/* copy over the corresponding eigenvector */
	for(i = 0; i < 3; i++)
		vec[i] = vects(i,k);
	
	/* success */
	return 0;
}
