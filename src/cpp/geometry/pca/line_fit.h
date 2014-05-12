#ifndef LINE_FIT_H
#define LINE_FIT_H

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

#include <Eigen/Dense>
#include <vector>

/**
 * The line_fit_t class is used to represent the best-fit line
 */
class line_fit_t
{
	/* parameters */
	public:

		/* the unit vector indicating the direction of the line
		 * that was fit to the input data */
		Eigen::Vector3d dir;

		/* A point in 3D space that resides on the best-fit line.
		 * Typically this value is the mean of the input set. */
		Eigen::Vector3d p;

	/* functions */
	public:
		
		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Given a set of points, computes best-fit line using PCA
		 *
		 * The best-fit line to the input points is 
		 * computed, and the line model is stored in this object.
		 *
		 * Each point is represented as a pointer to a vector
		 * object, so that additional copies and instantiation
		 * of Eigen structures are not required.
		 *
		 * @param P  The list of points
		 */
		void fit(const std::vector<const Eigen::Vector3d*>& P);

		/**
		 * Computes the distance of a point from the modeled line
		 *
		 * Given a point represented as a Vector3d, will determine
		 * the distance of the point from the line represented
		 * in this object.  This should only be called if fit()
		 * has first been called to initialize the model parameters.
		 *
		 * @param p   The point to measure
		 *
		 * @return    Returns distance of given point to line
		 */
		double distance(const Eigen::Vector3d& p) const;
};

#endif
