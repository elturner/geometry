#include "plane.h"
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <Eigen/Eigenvalues>

/**
 * @file   plane.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines the geometry of a plane in 3D space
 *
 * @section DESCRIPTION
 *
 * This file contains the plane_t class, which represents the shape
 * of a plane in 3D space.  It extends the functionality of the
 * shape_t interface class, for convenience.
 */

using namespace std;
using namespace Eigen;

/* function implementations */
		
plane_t::plane_t()
{
	this->normal << 0,0,1;
	this->point  << 0,0,0;
};

void plane_t::fit(const std::vector<Eigen::Vector3d,
		Eigen::aligned_allocator<Eigen::Vector3d> >& P)
{
	SelfAdjointEigenSolver<Matrix3d> eig;
	Matrix3d C;
	unsigned int i, i_min, n;

	/* prepare the mean vector and covariance matrix */
	n = P.size(); /* get number of points */
	this->point << 0,0,0; /* initialize mean to zero */
	C << 0,0,0, 0,0,0, 0,0,0; /* initialize Cov to zero */
	for(i = 0; i < n; i++)
	{
		/* incorporate current point in mean estimate */
		this->point += P[i];

		/* incorporate into (lower triangle of) covariance matrix */
		C(0,0) += P[i](0) * P[i](0);
		C(0,1) += P[i](0) * P[i](1);
		C(0,2) += P[i](0) * P[i](2);
		C(1,1) += P[i](1) * P[i](1);
		C(1,2) += P[i](1) * P[i](2);
		C(2,2) += P[i](2) * P[i](2);
	}

	/* normalize */
	this->point /= n;
	C /= n;
	C(0,0) -= this->point(0) * this->point(0);
	C(0,1) -= this->point(0) * this->point(1);
	C(0,2) -= this->point(0) * this->point(2);
	C(1,1) -= this->point(1) * this->point(1);
	C(1,2) -= this->point(1) * this->point(2);
	C(2,2) -= this->point(2) * this->point(2);
	
	/* populate other half of covariance matrix */
	C(1,0) = C(0,1);
	C(2,0) = C(0,2);
	C(2,1) = C(1,2);

	/* find the dominant eigenvector */
	eig.compute(C);
	i_min = 0;
	for(i = 1; i < eig.eigenvalues().rows(); i++)
		if(eig.eigenvalues()[i_min] > eig.eigenvalues()[i])
			i_min = i;
	
	/* store in object */
	this->normal(0) = eig.eigenvectors()(0,i_min);
	this->normal(1) = eig.eigenvectors()(1,i_min);
	this->normal(2) = eig.eigenvectors()(2,i_min);
}
		
void plane_t::writeobj(std::ostream& os) const
{
	Vector3d a, b;
	int i, i_min;

	/* get minimum component of normal */
	i_min = 0;
	for(i = 1; i < 3; i++)
		if(abs(this->normal(i)) < abs(this->normal(i_min)))
			i_min = i;

	/* get coordinates along the plane */
	a << 0,0,0; a(i_min) = 1;
	b = this->normal.cross(a).normalized();
	a = b.cross(this->normal);

	/* export vertices and facecs */
	os << "#" << endl
	   << "# Plane Definition: " << endl
	   << "# \tnormal : " << this->normal.transpose() << endl
	   << "# \tcenter : " << this->point.transpose() << endl
	   << "# \ta      : " << a.transpose() << endl
	   << "# \tb      : " << b.transpose() << endl
	   << "v " << (this->point + 0.1*( a + b)).transpose() << endl
	   << "v " << (this->point + 0.1*( a - b)).transpose() << endl
	   << "v " << (this->point + 0.1*(-a - b)).transpose() << endl
	   << "v " << (this->point + 0.1*(-a + b)).transpose() << endl
	   << "f -1 -2 -3 -4" << endl << endl;
}
