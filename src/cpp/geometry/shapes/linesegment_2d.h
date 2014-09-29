#ifndef LINESEGMENT_2D_H
#define LINESEGMENT_2D_H

/**
 * @file linesegment_2d.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the linesegment_2d_t class, which is used to store
 * the 2D project of a line segment.  If used to intersect 3D space, it
 * will intersect all shapes that are projected onto the xy plane and
 * intersect this 2D line segment.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/Dense>

/**
 * the linesegment_2d_t class represents a line segment in 2D space
 */
class linesegment_2d_t : public shape_t
{
	/* parameters */
	private:

		/* The original end-points of line segment in 2D */
		Eigen::Vector2d orig;
		Eigen::Vector2d end;

		/* the 'direction' of the ray is (end-orig) */
		Eigen::Vector2d invdir; /* element-wise inverse of dir */

		/* the sign of invdir.  Zero means invdir[i] is positive,
		 * while One means invdir[i] is negative. */
		int s[2];

	/* functions */
	public:

		/* constructors */

		/**
		 * Constructs default line segment
		 */
		linesegment_2d_t()
		{
			Eigen::Vector2d zeros(0,0);
			this->init(zeros, zeros);
		};

		/**
		 * Contructs line segment from end points in 2D
		 */
		linesegment_2d_t(const Eigen::Vector2d& a,
				 const Eigen::Vector2d& b)
		{
			this->init(a,b);
		};

		/**
		 * Constructs line segment from end points in 3D.
		 *
		 * The 3D points will be projected onto 2D space.
		 */
		linesegment_2d_t(const Eigen::Vector3d& a,
		              const Eigen::Vector3d& b)
		{
			this->init(a, b);
		};

		/**
		 * Frees all memory and resources
		 */
		~linesegment_2d_t() { /* do nothing */ };

		/**
		 * Initializes this line segment to the given points
		 *
		 * @param a  The starting point
		 * @param b  The ending point
		 */
		inline void init(const Eigen::Vector2d& a,
		                 const Eigen::Vector2d& b)
		{
			/* store values */
			orig = a;
			end = b;
			invdir(0) = 1/(end(0)-orig(0));
			invdir(1) = 1/(end(1)-orig(1));
			s[0] = (invdir[0] < 0);
			s[1] = (invdir[1] < 0);
		};

		/**
		 * Initializes the line segment from the given 3D points
		 *
		 * @param a  The starting point
		 * @param b  The ending point
		 */
		inline void init(const Eigen::Vector3d& a,
		                 const Eigen::Vector3d& b)
		{
			Eigen::Vector2d aa, bb;

			/* project to 2D */
			aa << a(0), a(1);
			bb << b(0), b(1);
			this->init(aa,bb);
		};

		/**
		 * Retrieves the number of vertices that compose this line
		 *
		 * @return   The number of vertices in line
		 */
		inline unsigned int num_verts() const
		{ return 2; /* it's a line, duh */ };
		
		/**
		 * Retrieves the i'th vertex of shape in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			Eigen::Vector3d v;

			/* determine which endpoint to return */
			if(i == 0)
				v << orig(0), orig(1), 0;
			else
				v << end(0), end(1), 0;

			/* return the projected vector in 3d */
			return v;
		};

		/**
		 * Tests intersection of this line segment with cube
		 *
		 * Given the min and max corners of an axis-aligned cube,
		 * will return true iff this line segment intersects the
		 * cube.
		 *
		 * bounds should be a 3x2 matrix, where the first column
		 * is the min corner of the cube, and the second column
		 * is the max corner of the cube.
		 *
		 * This algorithm was taken from:
		 *
		 * An Efficient and Robust Ray–Box Intersection Algorithm,
		 * Amy Williams et al. 2004.
		 *
		 * http://www.scratchapixel.com/lessons/3d-basic-lessons/
		 * lesson-7-intersecting-simple-shapes/ray-box-intersection/
		 *
		 * @param c   Center of the cube
		 * @param hw  Half-width of the cube
		 *
		 * @return    Returns true iff line intersects cube
		 */
		inline bool intersects(const Eigen::Vector3d& c,
		                       double hw) const
		{
			double tmin, tmax, tymin, tymax; 
			Eigen::Matrix<double,2,2> bounds;

			/* populate bounds */
			bounds << c(0) - hw, c(0) + hw,
			          c(1) - hw, c(1) + hw;

			/* compute intersections in x-coordinates */
			tmin = (bounds(0,this->s[0]) - this->orig(0))
			 		* this->invdir(0); 
			tmax = (bounds(0,1-this->s[0]) - this->orig(0))
			 		* this->invdir(0); 
			 
			/* get equivalent values for y-coordinates */
			tymin = (bounds(1,this->s[1]) - this->orig(1))
			 		* this->invdir(1); 
			tymax = (bounds(1,1-this->s[1]) - this->orig(1))
			 		* this->invdir(1);
			 
			/* check that line intersects the xy square of
			 * the projection of this cube */
			if ((tmin > tymax) || (tymin > tmax))
				return false; /* no intersection */
			if (tymin > tmin)
				tmin = tymin; /* flip so ray is positive */
			if (tymax < tmax)
				tmax = tymax; /* flip so ray is positive */

			/* at this point, we know the line intersects the
			 * box.  What remains to be seen is if the line
			 * SEGMENT intersects the box, or if the range
			 * of the segment stops short. */

			/* check if line is too short to intersect box */
			if(tmin > tmax || tmin > 1 || tmax < 0)
				return false;

			/* line intersects box */
			return true;
		};
		
		/**
		 * Will be called on leaf nodes this shape intersects
		 *
		 * This function will allow the shape to modify the
		 * data stored at leaf nodes that it intersects.  It
		 * will be given the current data element, and should
		 * return the modified data element.  If the input
		 * is null, this function is expected to allocate a
		 * new octdata_t object to use.
		 *
		 * Typically, the return value should be the same as
		 * the input.
		 *
		 * @param c    The center position of leaf node
		 * @param hw   The half-width of leaf node
		 * @param d    The original data, can be null
		 *
		 * @return     Returns pointer to the modified data
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
