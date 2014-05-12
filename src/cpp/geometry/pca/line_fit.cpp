#include "line_fit.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <iostream>

/**
 * @file line_fit.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Provides functions that fit lines to 3D points using PCA
 *
 * @section DESCRIPTION
 *
 * This file contains routines that find the best-fit line to a set
 * of 3D points, by using Principal Components Analysis (PCA).
 *
 * Requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

/* function implmenentations */

void line_fit_t::fit(const vector<const Vector3d*>& P)
{
	SelfAdjointEigenSolver<Matrix3d> eig;
	Matrix3d C;
	unsigned int i, i_max, n;

	/* prepare the mean vector and covariance matrix */
	n = P.size(); /* get number of points */
	this->p << 0,0,0; /* initialize mean to zero */
	C << 0,0,0, 0,0,0, 0,0,0; /* initialize Cov to zero */
	for(i = 0; i < n; i++)
	{
		/* incorporate current point in mean estimate */
		this->p += (*P[i]);

		/* incorporate into (lower triangle of) covariance matrix */
		C(0,0) += (*P[i])(0) * (*P[i])(0);
		C(0,1) += (*P[i])(0) * (*P[i])(1);
		C(0,2) += (*P[i])(0) * (*P[i])(2);
		C(1,1) += (*P[i])(1) * (*P[i])(1);
		C(1,2) += (*P[i])(1) * (*P[i])(2);
		C(2,2) += (*P[i])(2) * (*P[i])(2);
	}

	/* normalize */
	this->p /= n;
	C /= n;
	C(0,0) -= this->p(0) * this->p(0);
	C(0,1) -= this->p(0) * this->p(1);
	C(0,2) -= this->p(0) * this->p(2);
	C(1,1) -= this->p(1) * this->p(1);
	C(1,2) -= this->p(1) * this->p(2);
	C(2,2) -= this->p(2) * this->p(2);
	
	/* populate other half of covariance matrix */
	C(1,0) = C(0,1);
	C(2,0) = C(0,2);
	C(2,1) = C(1,2);

	/* find the dominant eigenvector */
	eig.compute(C);
	i_max = 0;
	for(i = 1; i < eig.eigenvalues().rows(); i++)
		if(eig.eigenvalues()[i_max] < eig.eigenvalues()[i])
			i_max = i;
	
	/* store in object */
	this->dir(0) = eig.eigenvectors()(0,i_max);
	this->dir(1) = eig.eigenvectors()(1,i_max);
	this->dir(2) = eig.eigenvectors()(2,i_max);
}
		
double line_fit_t::distance(const Vector3d& p) const
{
	Vector3d d, n;

	/* compute displacement from line point */
	d = p - this->p;

	/* distance is the orthogonal distance from the line */
	n = d - ( this->dir * d.dot(this->dir) );

	/* return the distance to line */
	return n.norm();
}
