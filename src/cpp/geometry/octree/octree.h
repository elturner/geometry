#ifndef OCTREE_H
#define OCTREE_H

/**
 * @file octree.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines a octree structure.
 * The octree represents all of 3D space, and
 * the bounding box grows as more elements are added.
 */

#include <Eigen/Dense>
#include <string>
#include "octnode.h"
#include "shape.h"
#include "octdata.h"

/* defines the octree class */
class octree_t
{
	/*** parameters ***/
	private:

		/* root of the tree and its relative position */
		octnode_t* root;

		/* the tree expands down some max depth.  If this
		 * value is negative, the tree is ill-defined.  If
		 * this value is zero, the tree is empty.  If this
		 * value is positive, then the tree has this many
		 * levels. */
		int max_depth;

	/*** function ***/
	public:

		/* constructors */

		/**
		 * Constructs empty tree with default resolution.
		 */
		octree_t();

		/**
		 * Constructs empty tree with specified resolution.
		 *
		 * The given resolution is in units of length, and the
		 * nodes at the maximum depth of the octree will be at
		 * least this large.  The depth may change as the domain
		 * of space the octree covers grows, but the resolution
		 * of the lowest possible level will remain constant.
		 *
		 * @param r   The specified resolution, units of length
		 */
		octree_t(double r);

		/**
		 * Frees all memory and resources
		 */
		~octree_t();

		/* accessors */

		/**
		 * Sets a new resolution for this tree.
		 *
		 * Sets the resolution to be the argument. Will destroy 
		 * any information in tree.
		 *
		 * @param r  The new resolution, units of length.
		 */
		void set_resolution(double r);

		/**
		 * Retrieves the current resolution for this tree
		 *
		 * The resolution represents the minimum voxel size in the
		 * tree.  The resolution is in units of length.
		 *
		 * @return   Returns the resolution value.
		 */
		double get_resolution() const;

		/**
		 * Frees all memory and resources from this structure.
		 *
		 * Clears all information from tree.  set_resolution()
		 * must be called before adding more data.
		 */
		void clear();

		/**
		 * Clones the given tree into this tree
		 *
		 * Given a reference to another octree, will destroy
		 * information in this octree, and make a deep copy
		 * of the reference.
		 *
		 * @param other The octree to deep copy into this object.
		 */
		void clone_from(const octree_t& other);

		/**
		 * Returns a pointer to the root node of this tree.
		 *
		 * @return   Returns root node pointer.
		 */
		inline octnode_t* get_root() const
		{ return this->root; };

		/* geometry */

		/**
		 * Will increase the domain of octree so point is contained
		 *
		 * Given a point which may be outside of the current
		 * domain of this octree, will potentially increase the
		 * domain of the octree so that the point is covered.  If
		 * the point is already inside the domain of the octree,
		 * then no action will be performed.
		 *
		 * @param p  The point in question
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int include_in_domain(const Eigen::Vector3d& p);

		/**
		 * Will insert the given shape into the tree
		 *
		 * This function will add nodes to the tree, either
		 * to the max depth or until a depth at which
		 * a node already has data stored.  Nodes will be added
		 * iff they intersect the given shape. The domain of the
		 * tree may be extended so that the full shape
		 * is contained within the tree.
		 *
		 * After this call, all leaf nodes in the tree that
		 * are intersected by this line segment will be modified
		 * by the input shape.
		 *
		 * @param s   The shape to use to carve the tree
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		int insert(const shape_t& s);
		
		/* i/o */

		/**
		 * Serializes data structure to binary file.
		 *
		 * Prints a representation of this tree to the
		 * specified binary file.  The tree can be
		 * perfectly reconstructed by using this information.
		 *
		 * @param fn   The path to output file to write
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int serialize(const std::string& fn) const;

		/**
		 * Parses serialization of octree from stream
		 *
		 * Reads from the specified stream and creates the
		 * tree described.  This will destroy any existing data.
		 *
		 * Assumes the content of the stream is formatted in the
		 * same manner as octree_t::serialize().
		 *
		 * @param fn   The path to the input file to parse
		 *
		 * @return     Returns 0 on success, non-zero on failure.
		 */
		int parse(const std::string& fn);
};

#endif
