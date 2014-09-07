#ifndef PLANE_H
#define PLANE_H

/**
 * @file   plane.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines the geometry of a plane in 3D space
 *
 * @section DESCRIPTION
 *
 * This file contains the plane_t class, which represents the shape
 * of a plane in 3D space.  It extends the functionality of the
 * shape_t interface class, for convenience.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>

/**
 * The plane_t class represents the geometry of a plane in 3D space
 */
class plane_t : public shape_t
{
	/* parameters */
	public:

		/**
		 * The directionality of the plane is defined by a normal
		 *
		 * The normal of the plane represents the direction
		 * that is orthogonal to the displacement of any two
		 * points on the plane.
		 *
		 * This value is assumed to be normalized.
		 */
		Eigen::Vector3d normal;

		/**
		 * The offset of the plane is defined by a point
		 *
		 * This point resides in 3D space, and intersects
		 * the plane.
		 */
		Eigen::Vector3d point;

	/* functions */
	public:
	
		/*--------------*/
		/* constructors */
		/*--------------*/

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Constructs default plane orientation and offset
		 */
		plane_t();

		/**
		 * Constructs plane from given other plane
		 */
		plane_t(const plane_t& other)
			:	normal(other.normal),
				point(other.point)
		{};

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Computes the distance of a given point to this plane
		 *
		 * Given a point in 3D space, this function will return
		 * the distance of the point to this plane.
		 *
		 * @param p   A point to analyze
		 *
		 * @return    Returns the distance to p from this plane
		 */
		inline double distance_to(const Eigen::Vector3d& p) const
		{ return this->normal.dot(p - this->point); };

		/**
		 * Performs PCA on the given points, and stores the best-fit
		 * plane in this structure.
		 *
		 * After this call, the parameters of this structure
		 * will be modified to fit the plane given the input
		 * points.
		 *
		 * @param P    The list of points to fit a plane to
		 */
		void fit(const std::vector<Eigen::Vector3d,
			Eigen::aligned_allocator<Eigen::Vector3d> >& P);

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes a representation of this plane to an OBJ file
		 *
		 * Given the stream to a Wavefront OBJ file, will export
		 * a patch of this plane.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;

		/*-----------------------------------*/
		/* overloaded functions from shape_t */
		/*-----------------------------------*/

		/**
		 * The number of vertices used to represent this shape
		 *
		 * Since the shape of a plane is infinite, this
		 * is not really a valid operation to ask.  However,
		 * since the plane is defined by a point in space (as
		 * well as a normal), it seems reasonable to return it
		 * here.
		 *
		 * @return   The number of vertices for this shape
		 */
		inline unsigned int num_verts() const
		{ return 1; };

		/**
		 * Gets the i'th vertex
		 *
		 * As noted above, only one vertex is explicitly
		 * defined for this plane, so that will always be
		 * returned.
		 *
		 * @param i   The index of the vertex to return
		 *
		 * @return    The position of the i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{ 
			MARK_USED(i);
			return this->point;
		};

		/**
		 * Determines if the aabb is intersected by this plane
		 *
		 * Will perform the computation necessary to determine
		 * if this plane intersects the Axis-Aligned Bounding
		 * Box (AABB) defined by a center position and a half-
		 * width.
		 *
		 * NOTE:
		 *
		 * For computational efficiency, this calculation is
		 * only approximate.  It checks the plane-sphere
		 * intersection, not the plane-box intersection.
		 *
		 * @param c   The center of the sphere to check
		 * @param hw  The radius of the sphere to check
		 *
		 * @return    Returns true iff the sphere is intersected
		 */
		inline bool intersects(const Eigen::Vector3d& c,
					double hw) const
		{
			/* get distance of c from plane and
			 * check if inside radius */
			return (this->distance_to(c) < hw);
		};
		
		/**
		 * Performs a no-op on the given data
		 *
		 * This function is overloaded from the shape_t class.
		 * It is used to allow shapes to interact and modify
		 * the data stored in an octree.  Since this plane class
		 * is only concerned with defining the plane, we don't need
		 * to do anything here.
		 *
		 * @param c   The center of the node to modify
		 * @param hw  The halfwidth of the node to modify
		 * @param d   The data of the node to modify
		 *
		 * @return    Returns the unmodified data.
		 */
		inline octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                                double hw,
		                                octdata_t* d)
		{
			MARK_USED(c);
			MARK_USED(hw);
			return d;
		};
};

#endif
