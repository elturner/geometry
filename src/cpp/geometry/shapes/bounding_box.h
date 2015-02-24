#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

/**
 * @file   bounding_box.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Computes the bounding box of an octree
 *
 * @section DESCRIPTION
 *
 * The bounding_box_t class extends shape_t, and iterates through
 * the nodes of a given octree to compute the bounding box of the
 * data stored in the octree.
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <util/error_codes.h>
#include <Eigen/Dense>

/**
 * Represents the bounding box of populated data in an octree
 *
 * The bounding_box_t class extends shape_t, and iterates through
 * the nodes of a given octree to compute the bounding box of the
 * data stored in the octree.
 */
class bounding_box_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The minimum corner of the octree so far
		 *
		 * If this corner has any components greater
		 * than the max corner, then the bounding box
		 * is invalid.
		 */
		Eigen::Vector3d min_corner;

		/**
		 * The maximum corner of the octree so far
		 *
		 * If this corner has any components less than
		 * the min corner, then the bounding box is invalid.
		 */
		Eigen::Vector3d max_corner;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Clears all info in this bounding box
		 *
		 * After this call, the bounding box will
		 * be reset to be invalid.
		 */
		inline void clear()
		{
			this->min_corner << 1,1,1;
			this->max_corner << 0,0,0;
		};

		/**
		 * Populates the bounding box.
		 *
		 * Initializes this bounding box based on the given
		 * octree.  This will populate the fields of the
		 * bounding box based on non-empty leaf nodes
		 * of the octree, but will not modify the tree in
		 * any way.
		 *
		 * Will clear any existing info.
		 *
		 * @param tree    The tree to analyze.
		 */
		void init(octree_t& tree);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Checks if the box is currently valid.
		 *
		 * @return  Returns true iff bounding box is valid.
		 */
		inline bool is_valid() const
		{
			if(this->min_corner(0) > this->max_corner(0))
				return false;
			if(this->min_corner(1) > this->max_corner(1))
				return false;
			if(this->min_corner(2) > this->max_corner(2))
				return false;
			return true;
		};

		/**
		 * Retrieves minimum bound for the i'th dimension.
		 *
		 * @param i   The coordiante to check: x=0, y=1, z=2
		 *
		 * @return    Reference to the min corner at coordinate i
		 */
		inline double get_min(size_t i) const
		{ return this->min_corner(i); };

		/**
		 * Retrieves maximum bound for the i'th dimension.
		 *
		 * @param i   The coordiante to check: x=0, y=1, z=2
		 *
		 * @return    The max bound in the i'th coordinate.
		 */
		inline double get_max(size_t i) const
		{ return this->max_corner(i); };
		
		/*----------------------------*/
		/* overloaded shape functions */
		/*----------------------------*/

		/**
		 * Overloaded from shape_t class.
		 *
		 * Always returns zero
		 *
		 * @return   Returns zero.
		 */
		inline unsigned int num_verts() const
		{ return 0; };
		
		/**
		 * Overloaded from shape_t class.
		 *
		 * Returns default vector
		 *
		 * @param i   Unused.
		 *
		 * @return    Returns default vector.
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			Eigen::Vector3d v;
			MARK_USED(i);
			return v;
		};
		
		/**
		 * Overloaded from shape_t class
		 *
		 * This intersects function will always return true,
		 * since we want to analyze all of a given octree.
		 *
		 * @param c   The center of an octnode
		 * @param hw  The halfwidth of an octnode
		 *
		 * @return     True, always.
		 */
		inline bool intersects(const Eigen::Vector3d& c,
		                       double hw) const
		{ 
			MARK_USED(c);
			MARK_USED(hw);
			return true; 
		};
		
		/**
		 * Will analyze the specified data and update bounding box
		 *
		 * Given an octdata element at a specified location,
		 * will update bounding box if the data has a count
		 * greater than zero.
		 *
		 * @param c   The center of the data location
		 * @param hw  The halfwidth of the data location
		 * @param d   The data to analyze, may be null.
		 *
		 * @return   Returns d, unmodified.
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);
};

#endif
