#include "noisy_scan.h"
#include <util/randLib.h>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Geometry>

/**
 * @file noisy_scan.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the noisy_scan_t class, which is used to
 * represent uncertainty in scan points, in the scanner's frame of
 * reference.
 *
 * This class requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

noisy_scan_t::noisy_scan_t()
{
	/* default values */
	this->set(0.0, 0.0, 0.0, 0.0, 0.0);
}
		
noisy_scan_t::noisy_scan_t(double x, double y, double z, 
                           double sr, double sl)
{
	/* set values */
	this->set(x, y, z, sr, sl);
}
		
void noisy_scan_t::set(const Eigen::Vector3d& p, const Eigen::Matrix3d& c)
{
	this->P = p;
	this->C = c;
}
		
void noisy_scan_t::set(double x, double y, double z, double sr, double sl)
{
	Vector3d a, b, c, r;
	double da, db;

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

	/* prepare coefficient matrix */
	this->C(0,0)=sl*a(0); this->C(0,1)=sl*b(0); this->C(0,2)=sr*r(0);
	this->C(1,0)=sl*a(1); this->C(1,1)=sl*b(1); this->C(1,2)=sr*r(1);
	this->C(2,0)=sl*a(2); this->C(2,1)=sl*b(2); this->C(2,2)=sr*r(2);
}
		
Vector3d noisy_scan_t::generate_sample() const
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
