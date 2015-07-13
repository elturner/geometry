#ifndef POINT_2D_H
#define POINT_2D_H

/**
 * @file    point_2d.h
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   Defines a point in 2D space (also allows for height ranges)
 *
 * @section DESCRIPTION
 *
 * This file defines the point_2d_t class, whihc is used to store
 * the shape information for a point in 2D, with optionally information
 * about a range of heights at that point.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/Dense>

/**
 * The point_2d_t class represents a point in 2d (or 2.5d) space
 */
class point_2d_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The point in question 
		 */
		Eigen::Vector2d p;

		/**
		 * A range of heights.
		 *
		 * If max < min, then the full range of heights
		 * will be considered as intersecting this point.
		 * If max <= min, then only the range specified will
		 * be considered as eligable to intersect this point.
		 */
		double z_min, z_max;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs default point
		 */
		point_2d_t() : p(0,0), z_min(1), z_max(0)
		{};

		/**
		 * Constructs point from given value
		 *
		 * @param q    The point to test
		 */
		point_2d_t(const Eigen::Vector2d& q) 
			: p(q), z_min(1), z_max(0)
		{};

		/**
		 * Constructs range of heights at given point
		 *
		 * @param q      The point to test
		 * @param min_z  The minimum elevation
		 * @param max_z  The maximum elevation
		 */
		point_2d_t(const Eigen::Vector2d& q, 
			double min_z, double max_z)
				: p(q), z_min(min_z), z_max(max_z)
		{};

		/**
		 * Frees all memory and resources
		 */
		~point_2d_t() { /* do nothing */ };

		/*---------------*/
		/* initialiation */
		/*---------------*/

		/**
		 * Initializes based on given geometry
		 *
		 * @param q      The 2D point
		 */
		inline void init(const Eigen::Vector2d& q)
		{
			this->p = q;
			this->z_min = 1; /* min > max means full height */
			this->z_max = 0;
		};

		/**
		 * Initializes based on given geometry
		 *
		 * @param q      The 2D point
		 * @param min_z  The minimum elevation
		 * @param max_z  The maximum elevation
		 */
		inline void init(const Eigen::Vector2d& q,
				double min_z, double max_z)
		{
			this->p = q;
			this->z_min = min_z;
			this->z_max = max_z;
		};

		/*----------------------*/
		/* overloaded functions */
		/*----------------------*/

		/**
		 * Retrieves the number of vertices (just one point)
		 *
		 * @return   Number of vertices
		 */
		inline unsigned int num_verts() const
		{ return 1; };

		/**
		 * Retrieves the i'th vertex
		 *
		 * @param i   The vertex to retrieve
		 *
		 * @return    The i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			Eigen::Vector3d v;
			MARK_USED(i);

			/* copy point location */
			v << p(0), p(1), 0;

			return v;
		};

		/**
		 * Tests intersection of this point with cube
		 *
		 * Given the center and halfwidth of a cube, will
		 * return true iff this point / vertical line-segment
		 * intersects the cube.
		 *
		 * @return c   The center of the cube
		 * @param hw   The halfwidth of the cube
		 *
		 * @return     Returns true iff intersection occurs
		 */
		inline bool intersects(const Eigen::Vector3d& c,
				double hw) const
		{
			Eigen::Vector2d diff;
			double d;

			/* check if intersection is possible given
			 * the height ranges of this point */
			if(this->z_min <= this->z_max)
			{
				/* we have a valid height range,
				 * so the cube better fall within it */
				if(c(2) - hw > this->z_max)
					return false; /* cube too high */
				if(c(2) + hw < this->z_min)
					return false; /* cube too low */
			}

			/* check for 2D intersection */
			diff(0) = this->p(0) - c(0);
			diff(1) = this->p(1) - c(1);
			d = diff.cwiseAbs().maxCoeff();
			if(d > hw)
				return false; /* out of bounds */

			/* an intersection occurs if we got here */
			return true;
		};

		/**
		 * Will be called on leaf nodes this shape intersects
		 *
		 * Currently a no-op
		 *
		 * @param c   The center of the leaf node
		 * @param hw  The halfwidth of the leaf node
		 * @param d   The original data
		 *
		 * @return    Returns the modified data
		 */
		inline octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
						double hw,
						octdata_t* d)
		{
			/* do nothing */
			MARK_USED(c);
			MARK_USED(hw);
			return d;
		};
};

#endif
