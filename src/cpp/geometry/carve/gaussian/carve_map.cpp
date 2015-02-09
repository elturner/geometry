#include "carve_map.h"
#include <iostream>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <cmath>

/**
 * @file carve_map.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The carve_map_t class is implemented here.  This class is used
 * to compute the result of a carve map, given a position in
 * continuous 3D space.  A carve map is generated by a single
 * range scan, and assigns a value to all of 3D space.  This
 * value is an estimate of the probability that a given point
 * in space is 'interior'.  A value of 0.5 represents no knowledge
 * of the environment, a value of 1.0 indicates certainty that
 * the point is interior, and a value of 0.0 indicates certainty
 * that the point is exterior.
 *
 * This class requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

/* definitions used for these functions */

/* definition of PI, which should be in cmath already */
#ifndef M_PI
#define M_PI 3.14159265358979323846264
#endif

/* The following are probability values to average, based
 * on different locations relative to a ray */
#define PROBABILITY_INTERIOR 1.0 /* a voxel that is interior */
#define PROBABILITY_TOOFAR   0.0 /* a voxel past the scan (exterior) */
#define PROBABILITY_A_PRIORI 0.5 /* a voxel with no information */

/**
 * Computes the gaussian PDF for 1D
 *
 * @param mu    The mean of this distribution
 * @param var   The variance of this distribution
 * @param x     The position to evaluate this distribution
 *
 * @return      Returns PDF evaluated at x
 */
inline double gauss_pdf(double mu, double var, double x)
{
	double xm = (x-mu);
	double tv = 2*var;

	/* unsimplified expression:
	 *
	 * (1/sqrt(2*pi*var)) * exp( -(x-mu)^2 / (2*var) )
	 */
	return exp( -xm*xm/tv ) / sqrt(M_PI*tv);
}

/* function implementations */

carve_map_t::carve_map_t()
{
	/* initialize default values */
	this->sensor_mean            = Vector3d::Zero();
	this->sensor_cov             = Matrix3d::Zero();
	this->scanpoint_mean         = Vector3d::Zero();
	this->scanpoint_cov          = Matrix3d::Zero();
	this->planar_prob            = 0.0;
	this->corner_prob            = 0.0;

	/* the following are cached parameters */
	this->ray                        = Vector3d::Zero();
	this->range                      = 0.0;
	this->sensor_norm                = Vector3d::Zero();
	this->sensor_dot                 = 0.0;
	this->sensor_var                 = 0.0;
	this->sensor_neg_inv_sqrt_2v     = 0.0; /* -1 / sqrt(2*var) */
	this->scanpoint_norm             = Vector3d::Zero();
	this->scanpoint_dot              = 0.0;
	this->scanpoint_var              = 0.0;
	this->scanpoint_neg_inv_sqrt_2v  = 0.0; /* -1 / sqrt(2*var) */
}
		
carve_map_t::carve_map_t(const Vector3d& s_mean,
                         const Matrix3d& s_cov,
                         const Vector3d& p_mean,
                         const Matrix3d& p_cov)
{
	/* initialize this object */
	this->init(s_mean, s_cov, p_mean, p_cov);
}
		
void carve_map_t::init(const Vector3d& s_mean,
                       const Matrix3d& s_cov,
                       const Vector3d& p_mean,
                       const Matrix3d& p_cov)
{
	Matrix<double, 1, 3> rt;

	/* copy input parameters */
	this->sensor_mean    = s_mean;
	this->sensor_cov     = s_cov;
	this->scanpoint_mean = p_mean;
	this->scanpoint_cov  = p_cov;
	this->planar_prob    = 0.0;
	this->corner_prob    = 0.0;

	/* compute cached values about ray */
	this->ray = p_mean - s_mean;
	this->range = this->ray.norm(); /* mean distance between ends */
	this->ray /= this->range; /* unit vector of ray of scan */

	/* in order to compute the plane modeled for each endpoint
	 * of the ray, we need to find the principal components
	 * of each distribution that are aligned with the ray */
	this->sensor_dot = carve_map_t::find_aligned_eig(
	                   this->sensor_norm, this->ray,
	                   this->sensor_cov);
	this->scanpoint_dot = carve_map_t::find_aligned_eig(
	                      this->scanpoint_norm, this->ray,
	                      this->scanpoint_cov);
	this->scanpoint_dot *= -1;
	this->scanpoint_norm *= -1;

	/* get the variance of each endpoint's distribution along
	 * the ray defined by this->ray.  regardless of the offset
	 * of this ray in space, the marginalized variance will be
	 * the same since it is gaussian. */
	rt = this->ray.transpose();
	this->sensor_var = rt * this->sensor_cov * this->ray;
	this->scanpoint_var = rt * this->scanpoint_cov * this->ray;

	/* initialize cached parameters that depend on the variance */
	this->sensor_neg_inv_sqrt_2v = (-1/sqrt(2*this->sensor_var));
	this->scanpoint_neg_inv_sqrt_2v = (-1/sqrt(2*this->scanpoint_var));

	/* compute cached values that are used to perform computations
	 * with the scanpoint's 3D pdf.  This is a multivariate gaussian,
	 * which means we can cache the normalization factor and
	 * the inverse covariance matrix:
	 *
	 * pdf(x) = (2*pi)^(-3/2)*det(cov)^(-1/2)
	 * 		*exp(-0.5*(x-mu)'*inv(cov)*(x-mu))
	 */
	
	/* (2*pi)^(-3/2)*det(scanpoint_cov)^(-1/2) */
	this->scanpoint_pdf_coef = 0.06349363593 
			* pow(this->scanpoint_cov.determinant(), -0.5);
	
	/* -0.5 * inv(cov) */
	this->mh_scanpoint_inv_cov = -0.5 * this->scanpoint_cov.inverse();
}

double carve_map_t::compute(const Vector3d& x, double xsize) const
{
	double w;
	MARK_USED(w);

	/* call the overloaded version of this function */
	return this->compute(x, xsize, w);
}

double carve_map_t::compute(const Eigen::Vector3d& x,
					double xsize, double& w) const
{
	Vector3d E, lat;
	Matrix3d C;
	double ms_dist, mp_dist, p_forward, p_inrange;
	double f, omf, latdist, varlat, p_lat, p_fl, p_total;

	/* compute distances of x from each endpoint plane */
	ms_dist = this->sensor_norm.dot(this->sensor_mean - x)
	                                    / this->sensor_dot;
	mp_dist = this->scanpoint_norm.dot(this->scanpoint_mean - x)
	                                    / this->scanpoint_dot;
	
	/* get the probability value that x appears after the sensor
	 * position and that x is before the scanpoint position along
	 * the ray.
	 *
	 * In these along-the-ray marginal distributions, the position
	 * of 'x' is at distance zero along the line. */

	/* CDF:  mu = ms_dist, var = this->sensor_var, x = 0
	 *
	 * CDF(mu,var,x) = 0.5*(1 + erf( (x - mu) / sqrt(2*var) ));
	 */
	p_forward = 0.5*(1 + erf(ms_dist*this->sensor_neg_inv_sqrt_2v));

	/* 1 - CDF:   mu = mp_dist, var = this->scanpoint_var, x = 0 */
	p_inrange = 0.5*(1 - erf(mp_dist*this->scanpoint_neg_inv_sqrt_2v));

	/* get the lateral position of this point away from the mean ray
	 * position of this scan.  This can be represented by a 1D
	 * marginal distribution in the direction orthogonal to the
	 * ray that passes through point x.  To find this distribution,
	 * we need to find the lateral variance at point x. */

	/* fractional position of x between sensor and scanpoint. */
	f = -ms_dist / (mp_dist - ms_dist); /* 0 = sensor, 1 = scanpoint */
	
	/* restrict f to be between 0 and 1 */
	if(f > 1)	f = 1;
	else if(f < 0)	f = 0;

	/* for weighted averaging purposes, it is also good to have
	 * the weight on the sensor-side, which is (1-f) */
	omf = (1-f); /* 0 = scanpoint, 1 = sensor */

	/* compute blended distribution that occurs at this midpoint
	 * of the ray, which is a linear average of the distributions at
	 * the two endpoints */
	E = omf*this->sensor_mean + f*this->scanpoint_mean; /* blend mean */
	C = omf*this->sensor_cov + f*this->scanpoint_cov; /* blended cov */
	
	/* find lateral distance of x to the ray */
	lat = (x - E);
	latdist = lat.norm();
	lat /= latdist;

	/* find variance at this lateral distance */
	varlat = lat.transpose() * C * lat;

	/* get the probability value that position x is laterally
	 * intersected by the ray.  This value requires knowledge of the
	 * width of x, in order to integrate the probability density
	 * to a probability value.  Note that this 'integration' makes
	 * some extreme simplifying assumptions about the distribution */

	/* This value is the PDF(mu, var, x), where:
	 *
	 * mu = 0, var = varlat, x = latdist */
	p_lat = gauss_pdf(0, varlat, latdist) * xsize;
	p_fl = p_forward * p_lat;
	
	/* the output probability is a bournoulli expected value, which
	 * we can estimate by a weighted average of different states,
	 * using the probabilities of each possible state */
	p_total =   ( p_fl *     p_inrange) * PROBABILITY_INTERIOR
	          + ( p_fl * (1-p_inrange)) * PROBABILITY_TOOFAR
		  + ( 1 - p_fl )            * PROBABILITY_A_PRIORI;

	/* error check this output value */
	if(!isfinite(p_total))
	{
		/* export data for debugging purposes */
		cerr << "[carve_map_t::compute]\tWARNING: found non-finite"
		     << " output." << endl
		     << "\tp_total = " << p_total << endl
		     << "\tp_fl = " << p_fl << endl
		     << "\tp_inrange = " << p_inrange << endl
		     << "\tp_lat = " << p_lat << endl
		     << "\tp_forward = " << p_forward << endl
		     << "\tvarlat = " << varlat << endl
		     << "\tlatdist = " << latdist << endl
		     << "\tlat = " << lat.transpose() << endl
		     << "\tf = " << f << endl
		     << "\tmp_dist = " << mp_dist << endl
		     << "\tms_dist = " << ms_dist << endl;
		this->print_params(cerr);
		cerr << endl;
	}

	/* the weighting of the return value is just the lateral
	 * probability, since this indicates how far off the scan
	 * line the query point is */
	w = p_lat;

	/* return the final probability */
	return p_total;
}
		
double carve_map_t::get_surface_prob(const Vector3d& x, double xsize) const
{
	Vector3d m;
	double p, v, e;

	/* compute relative position of x with respect to the
	 * mean of this distribution */
	m = x - this->scanpoint_mean;

	/* compute probability density at x */
	e = m.transpose() * mh_scanpoint_inv_cov * m;
	p = this->scanpoint_pdf_coef * exp(e);

	/* compute volume around x */
	v = xsize*xsize*xsize; /* assume a cube */

	/* approximate probility as constant for this volume.  This
	 * is not strictly correct, but works well if the standard
	 * deviation of the pdf is larger than xsize */
	return v*p; 
}

/* debugging functions */

void carve_map_t::print_params(std::ostream& os) const
{
	/* print some info about this map */
	os << "map info:" << endl
	   << "---------" << endl
	   << this->sensor_mean.transpose() << endl
	   << this->sensor_cov << endl
	   << this->scanpoint_mean.transpose() << endl
	   << this->scanpoint_cov << endl
	   << endl;
}

void carve_map_t::print_sampling(ostream& os) const
{
	Vector3d x;
	double xsize, f, d;
	unsigned int i, n;

	/* print samples at mean endpoint positions */
	xsize = 0.01;
	x = this->sensor_mean;
	f = this->compute(x, xsize);
	os << x(0) << " " << x(1) << " " << x(2) << " " << f << endl;
	x = this->scanpoint_mean;
	f = this->compute(x, xsize);
	os << x(0) << " " << x(1) << " " << x(2) << " " << f << endl;

	/* sample a bunch */
	n = 100;
	for(i = 0; i < n; i++)
	{
		/* pick a point and sample it */
		d = ((double)i) / ((double) n)*1.2 - 0.1;
		x = d * this->scanpoint_mean + (1-d)*this->sensor_mean;
		f = this->compute(x, xsize);
		os << x(0) << " " << x(1) << " " << x(2) << " " << f
		   << endl;
	}
}

void carve_map_t::writeobj(ostream& out) const
{
	int i;

	/* write points to file */
	out << "v " << this->scanpoint_mean(0)
	    <<  " " << this->scanpoint_mean(1)
	    <<  " " << this->scanpoint_mean(2)
	    <<  " 255 0 0" << endl;

	/* sample the distribution */
	JacobiSVD<Matrix3d> solver(this->scanpoint_cov,
	                           Eigen::ComputeFullU);
	Matrix3d U = solver.matrixU();
	Vector3d s = solver.singularValues();
	s(0) = sqrt(s(0)); s(1) = sqrt(s(1)); s(2) = sqrt(s(2));

	/* print two standard deviations in each direction */
	for(i = 0; i < 3; i++)
	{
		/* forward */
		out << "v " << (this->scanpoint_mean(0)
					+ 2*s(i)*U(0,i))
		    <<  " " << (this->scanpoint_mean(1)
		    			+ 2*s(i)*U(1,i))
		    <<  " " << (this->scanpoint_mean(2)
		    			+ 2*s(i)*U(2,i))
		    <<  " 0 0 255" << endl;

		/* backward */
		out << "v " << (this->scanpoint_mean(0)
					- 2*s(i)*U(0,i))
		    <<  " " << (this->scanpoint_mean(1)
		    			- 2*s(i)*U(1,i))
		    <<  " " << (this->scanpoint_mean(2)
		    			- 2*s(i)*U(2,i))
		    <<  " 0 0 255" << endl;
	}

	/* export some faces */
	out << "f -6 -4 -2" << endl
	    << "f -4 -5 -2" << endl
	    << "f -5 -3 -2" << endl
	    << "f -3 -6 -2" << endl
	    << "f -4 -6 -1" << endl
	    << "f -5 -4 -1" << endl
	    << "f -3 -5 -1" << endl
	    << "f -6 -3 -1" << endl;
}

void carve_map_t::writexyz(ostream& out) const
{
	out << this->scanpoint_mean(0) << " "
	    << this->scanpoint_mean(1) << " "
	    << this->scanpoint_mean(2) << " "
	    << ((unsigned int) (255*this->planar_prob)) << " "
	    << ((unsigned int) (255*this->corner_prob)) << " "
	    << ((unsigned int) (255*(1-this->planar_prob))) << endl;
}

/* helper functions */

double carve_map_t::find_aligned_eig(Eigen::Vector3d& eig,
                                     const Eigen::Vector3d& in,
                                     const Eigen::Matrix3d& M)
{
	Matrix<double, 1, 3> ds;
	int i, i_max;
	double d, d_max;

	/* perform the singular value decomposition on the input matrix */
	JacobiSVD<Matrix3d> solver(M, Eigen::ComputeFullU);
	Matrix3d U = solver.matrixU();

	/* find all dot products */
	ds = in.transpose() * U;

	/* iterate over the basis vectors */
	i_max = 0;
	d_max = fabs(ds(0));
	for(i = 1; i < 3; i++)
	{
		/* compare current value to max */
		d = fabs(ds(i));
		if(d > d_max)
		{
			/* found new max */
			i_max = i;
			d_max = d;
		}
	}

	/* export the result */
	eig = U.block(0,i_max,3,1);
	if(ds(i_max) < 0)
	{
		/* because the dot product is negative,
		 * we need to reverse the column when
		 * exporting */
		eig *= -1;
	}
	
	/* return the dot product */
	return d_max;
}
