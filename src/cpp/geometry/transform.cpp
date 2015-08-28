#include "transform.h"

/**
 * @file transform.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the transform_t class, which is used
 * to represent a rigid-body transform between coordinate
 * systems.
 */

#include <string>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <util/rotLib.h>

using namespace std;
using namespace Eigen;

/* function implementations */

transform_t::transform_t()
{
	/* eigen structures to identity values */
	
}

transform_t::~transform_t()
{
	/* no processing necessary */
}
		
int transform_t::set(const vector<double>& tToCommon,
                     const vector<double>& rToCommon)
{
	/* verify arguments */
	if(tToCommon.size() != 3 || rToCommon.size() != 3)
		return -1; /* invalid arguments */

	/* copy translation values */
	this->T(0) = tToCommon[0];
	this->T(1) = tToCommon[1];
	this->T(2) = tToCommon[2];
		
	/* copy rotation values, creating rotation matrix */
	rotLib::rpy2rot(rToCommon[0], rToCommon[1], rToCommon[2], this->R);

	/* success */
	return 0;
}
		
void transform_t::invert()
{
	this->R = this->R.inverse();
	this->T = this->R * ((-1) * this->T);
}
		
void transform_t::preapp(const transform_t& t)
{
	this->T = (this->R * t.T) + this->T;
	this->R = (this->R * t.R);
}
		
void transform_t::cat(const transform_t& t)
{
	this->R = (t.R * this->R);
	this->T = (t.R * this->T) + t.T;
}
		
void transform_t::apply(MatrixXd& pts) const
{
	size_t i, n;

	/* apply rotation component of transform */
	pts = this->R * pts;
	
	/* iterate over columns, adding translation */
	n = pts.cols();
	for(i = 0; i < n; i++)
		pts.col(i) += this->T;
}
		
void transform_t::apply(Vector3d& p) const
{
	p = (this->R * p) + this->T;
}
		
void transform_t::apply_inverse(MatrixXd& pts) const
{
	size_t i, n;

	/* subtract translation from each column */
	n = pts.cols();
	for(i = 0; i < n; i++)
		pts.col(i) -= this->T;

	/* apply inverse rotation matrix */
	pts = this->R.inverse() * pts;
}

void transform_t::apply_inverse(Vector3d& p) const
{
	p = this->R.inverse() * (p - this->T);
}
