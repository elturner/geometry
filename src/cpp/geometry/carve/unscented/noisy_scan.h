#ifndef NOISY_SCAN_H
#define NOISY_SCAN_H

/**
 * @file noisy_scan.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the noisy_scan_t class, which represents the
 * probability distribution for a single line-of-sight scan, where
 * the position of the scanner and the noise within the scan are modeled
 * with some uncertainty.
 *
 * This model accounts for noise that is internal to the scanner, which
 * is given as uncertainty in range or lateral position.
 *
 * This class requires the Eigen framework.
 */

#include <Eigen/Dense>
#include <Eigen/Geometry>

/**
 * The scan distribution class models uncertainty from scanner internals
 */
class noisy_scan_t
{
	/* parameters */
	private:

		/* mean position of scan, in the scanner's coordinate
		 * system.  Units of meters. */
		Eigen::Vector3d P;

		/* Uncertainty matrix.   This is the square-root
		 * of the covariance matrix of this point's random
		 * variable. By multiplying independent coordinate
		 * estimates by this matrix, we convert from i.i.d.
		 * gaussian to the distribution of this point. */
		Eigen::Matrix3d C;

	/* functions */
	public:

		/* constructors */

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Initializes a default scan point
		 */
		noisy_scan_t();

		/**
		 * Initializes scan point based on input geometry
		 *
		 * @param x   The x-coordinate of mean point position
		 * @param y   The y-coordinate of mean point position
		 * @param z   The z-coordinate of mean point position
		 * @param sr  The std. dev. along ray of point
		 * @param sl  The std. dev. orthogonal to ray of point.
		 */
		noisy_scan_t(double x, double y, double z, 
		             double sr, double sl);

		/* accessors */

		/**
		 * Sets the mean position for this point's distribution,
		 * with units of meters in the coordinate system of
		 * the point's originating scanner.
		 *
		 * Given the std. dev. of the position uncertainty along
		 * the ray of the point and lateral to the ray of the
		 * point, will update the representation of the point's
		 * distribution.  All input must be in units of meters.
		 *
		 * @param p   The point location of the distribution mean
		 * @param c   The square-root of the covariance matrix
		 */
		void set(const Eigen::Vector3d& p,
		         const Eigen::Matrix3d& c);

		/**
		 * Sets the mean position for this point's distribution,
		 * with units of meters in the coordinate system of
		 * the point's originating scanner.
		 *
		 * Given the std. dev. of the position uncertainty along
		 * the ray of the point and lateral to the ray of the
		 * point, will update the representation of the point's
		 * distribution.  All input must be in units of meters.
		 *
		 * @param x   The x-coordinate of mean in sensor coords
		 * @param y   The y-coordinate of mean in sensor coords
		 * @param z   The z-coordinate of mean in sensor coords
		 * @param sr  The std. dev. along ray of point
		 * @param sl  The std. dev. orthogonal to ray of point.
		 */
		void set(double x,double y,double z,double sr,double sl);

		/* probability */

		/**
		 * Generates a sample point from this distribution
		 *
		 * Will sample the Guassian distribution and return
		 * a 3D point in the same coordinate frame as this
		 * point.  The returned point will have units of meters.
		 *
		 * @return   The sampled point location.
		 */
		Eigen::Vector3d generate_sample() const;
};

#endif
