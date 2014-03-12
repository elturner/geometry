#ifndef LINESEGMENT_H
#define LINESEGMENT_H

/**
 * @file linesegment.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the linesegment_t class, which is used to store
 * line segments for efficient ray-tracing through octrees.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <Eigen/Dense>

/**
 * the linesegment_t class represents a line segment in 3D space
 */
class linesegment_t : public shape_t
{
	/* parameters */
	private:

		/* The original end-points of line segment in 3D */
		Eigen::Vector3d orig;
		Eigen::Vector3d end;

		/* the 'direction' of the ray is (end-orig) */
		Eigen::Vector3d invdir; /* element-wise inverse of dir */

		/* the sign of invdir.  Zero means invdir[i] is positive,
		 * while One means invdir[i] is negative. */
		int s[3];

	/* functions */
	public:

		/* constructors */

		/**
		 * Constructs line segment from end points.
		 */
		linesegment_t(const Eigen::Vector3d& a,
		              const Eigen::Vector3d& b)
		{
			/* store values */
			orig = a;
			end = b;
			invdir(0) = 1/(end(0)-orig(0));
			invdir(1) = 1/(end(1)-orig(1));
			invdir(2) = 1/(end(2)-orig(2));
			s[0] = (invdir[0] < 0);
			s[1] = (invdir[1] < 0);
			s[2] = (invdir[2] < 0);
		};

		/**
		 * Frees all memory and resources
		 */
		~linesegment_t() { /* do nothing */ };

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
			/* determine which endpoint to return */
			if(i == 0)
				return orig;
			return end;
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
			double tmin, tmax, tymin, tymax, tzmin, tzmax; 
			Eigen::Matrix<double,3,2> bounds;

			/* populate bounds */
			bounds << c(0) - hw, c(0) + hw,
			          c(1) - hw, c(1) + hw,
			          c(2) - hw, c(2) + hw;

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

			/* get values for z-coordinates */
			tzmin = (bounds(2,this->s[2]) - this->orig(2))
					* this->invdir(2); 
			tzmax = (bounds(2,1-this->s[2]) - this->orig(2))
					* this->invdir(2); 
			
			/* check 3D cube intersection */
			if ((tmin > tzmax) || (tzmin > tmax))
				return false; /* no intersection */
		
			/* at this point, we know the line intersects the
			 * box.  What remains to be seen is if the line
			 * SEGMENT intersects the box, or if the range
			 * of the segment stops short. */

			/* get distances of intersection points along ray */
			if (tzmin > tmin)
				tmin = tzmin; /* flip so ray is positive */
			if (tzmax < tmax)
				tmax = tzmax; /* flip so ray is positive */
			
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
		                                octdata_t* d) const
		{
			/* do nothing */
			c = c;
			hw = hw;
			return d;
		};
};

#endif
