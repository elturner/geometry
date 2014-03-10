#include "noisy_scanpoint.h"
#include <util/randLib.h>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Geometry>

/**
 * @file noisy_scanpoint.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the noisy_scanpoint_t class, which is used to
 * represent uncertainty in scan points, in the scanner's frame of
 * reference.
 *
 * This class requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

/* defines used by this class */
#define MAX_ALLOWED_NOISE 1000 /* units: meters */

/* function implementations */

noisy_scanpoint_t::noisy_scanpoint_t()
{
	/* default values */
	this->set(0.0, 0.0, 0.0, 0.0, 0.0);
}
		
noisy_scanpoint_t::noisy_scanpoint_t(double x, double y, double z, 
                           double sr, double sl)
{
	/* set values */
	this->set(x, y, z, sr, sl);
}
		
void noisy_scanpoint_t::set(const Eigen::Vector3d& p, const Eigen::Matrix3d& c)
{
	this->P = p;
	this->C = c;
}
		
void noisy_scanpoint_t::set(double x, double y, double z, double sr, double sl)
{
	Vector3d a, b, c, r;
	Matrix3d A, S, M;
	double da, db;

	/* check for finite noise */
	if(sr > MAX_ALLOWED_NOISE || sl > MAX_ALLOWED_NOISE)
	{
		this->finite_noise = false;
		return;
	}
	else
		this->finite_noise = true;

	/* save mean position */
	this->P(0) = x;
	this->P(1) = y;
	this->P(2) = z;

	/* get unit vector in direction of range */
	r = this->P.normalized();

	/* get orthogonal directions to this vector */
	a << 1,0,0; /* default unit vectors */
	b << 0,1,0; /* default unit vectors */
	da = r.dot(a);
	db = r.dot(b);
	if(da > db)
	{
		/* make sure that our vector 'a' is reasonably
		 * orthagonal to the range vector r */
		a = b;
		da = db;
	}

	/* make the vectors a and b to be unit-length, orthogonal to r */
	c = a - da*r;
	c.normalize();
	a = c;
	b = r.cross(a);
	A << a,b,r; /* orthonormal basis for point -> sensor transform */

	/* record std. devs in each direction along ray */
	S << sl, 0, 0,
	     0, sl, 0,
	     0, 0, sr;

	/* prepare coefficient matrix */
	M = A * S;
	this->C = M * M.transpose();
}
		
Vector3d noisy_scanpoint_t::generate_sample() const
{
	vector<double> samples;
	Vector3d p, q;

	/* get some standard normal samples */
	randLib::randn(samples, 3);
	p(0) = samples[0];
	p(1) = samples[1];
	p(2) = samples[2];

	/* correlate these samples */
	q = this->C * p + this->P;
	return q;
}
