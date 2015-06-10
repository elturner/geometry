#ifndef TRANSFORM_H
#define TRANSFORM_H

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

#include <vector>
#include <string>
#include <Eigen/Dense>
#include <Eigen/Geometry>

/* the transform_t class represents rigid-body transformations */
class transform_t
{
	/* parameters */
	public:

		/* Translation vector of this transformation */
		Eigen::Vector3d T; /* meters */
		
		/* The following specify the orientation of the scanner
		 * at this pose as a rotation from system coordinates
		 * to world coordinates */
		Eigen::Matrix3d R; /* rotation matrix */

	/* functions */
	public:

		/* the pose list is an array that contains eigen 
		 * constructions, so it must be properly aligned */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Initializes identity transformation
		 */
		transform_t();

		/**
		 * Frees all memory and resources.
		 */
		~transform_t();

		/* geometry */

		/**
		 * Sets the transform based on translation and rotations
		 *
		 * Given a translation as a vector of three values (x,y,z),
		 * and a rotation as a vector of three values (roll, pitch,
		 * yaw), will populate this transform with the appropriate
		 * values to represent these inputs.
		 *
		 * These values should be in meters and radians, 
		 * respectively.
		 *
		 * @param tToCommon   The translation values (x, y, z)
		 * @param rToCommon   The rotation angles (roll, pitch, yaw)
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int set(const std::vector<double>& tToCommon,
		        const std::vector<double>& rToCommon);

		/**
		 * Concatenates two transforms
		 *
		 * Will concatenate the specified transform to this one,
		 * and store the result in-place.  Applying the resulting
		 * transform is equivalent to applying the original, and
		 * then the argument transform.
		 *
		 * Consider the following example:
		 *
		 * A2B.cat(B2C)
		 * A2C = A2B
		 *
		 * @param t    The transform to post-apply to this one
		 */
		void cat(const transform_t& t);

		/**
		 * Applies this transform to the given list of points
		 *
		 * Given a list of points, represented as columns in
		 * the input matrix, this transform will be applied
		 * in-place to these points.
		 *
		 * @param pts   The points to transform in-place
		 */
		void apply(Eigen::MatrixXd& pts) const;

		/**
		 * Applies this transform to the given 3D point
		 *
		 * Given a point, this transform will be applied in-place.
		 *
		 * @param p   The point to transform.
		 */
		void apply(Eigen::Vector3d& p) const;

		/**
		 * Applies the inverse of this transform to given list
		 *
		 * Given a list of points, represented as columns in
		 * the input matrix, the inverse of this transform will
		 * be applied in-place to these points.
		 *
		 * @param pts    The points to trnasform in-place
		 */
		void apply_inverse(Eigen::MatrixXd& pts) const;

		/**
		 * Applies the inverse of this transform to the given point
		 *
		 * Will apply the inverse of this transform in-place on the
		 * specified point p.
		 *
		 * @param p   The point to modify.
		 */
		void apply_inverse(Eigen::Vector3d& p) const;

		/**
		 * Get the squared distance from other transform
		 *
		 * Will get the displacement of the translations
		 * between these two transforms, and return the
		 * squared distance.
		 *
		 * @param other   The other transform to compare to
		 *
		 * @return    Returns the squared distance to other.T
		 */
		inline double dist_sq(const transform_t& other) const
		{
			double x, y, z;
	
			/* compute distance */
			x = this->T(0) - other.T(0);
			y = this->T(1) - other.T(1);
			z = this->T(2) - other.T(2);
			return x*x + y*y + z*z;
		};

		/* operators */

		/**
		 * Sets value of this transform to argument
		 *
		 * Sets the value of this object to rhs
		 *
		 * @param  rhs   The transform to copy
		 *
		 * @return  Returns this tranform after modifications
		 */
		inline transform_t operator = (const transform_t& rhs)
		{
			/* copy params */
			this->T = rhs.T;
			this->R = rhs.R;

			/* return the value of this point */
			return (*this);
		};
};

#endif
