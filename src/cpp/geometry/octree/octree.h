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

/**
 * defines the octree class
 */
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
		 * Constructs an empty tree with the specified initial size
		 *
		 * Will construct an empty tree with the initial center
		 * position and size given, with the specified resolution.
		 *
		 * @param c   The center of the tree, units of length
		 * @param hw  The halfwidth of tree's root, units of length
		 * @param r   The resolution of the tree, units of length
		 */
		octree_t(const Eigen::Vector3d& c, double hw, double r);

		/**
		 * Frees all memory and resources
		 */
		~octree_t();

		/* accessors */

		/**
		 * Sets the size of this tree.
		 *
		 * This call will clear any existing data from the tree.
		 *
		 * @param c  The center of the tree, units of length
		 * @param hw  The halfwidth of tree's root, units of length
		 * @param r   The resolution of the tree, units of length
		 */
		void set(const Eigen::Vector3d& c, double hw, double r);

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
		 * Retrieves the max depth of this tree
		 *
		 * @return   Retuns the maximum allowed depth of the tree
		 */
		inline int get_max_depth() const
		{ return this->max_depth; };

		/**
		 * Will increase the max allowed depth of the tree
		 *
		 * This call will NOT destroy any existing data in the
		 * tree.  It will also not add any new nodes.  After this
		 * call, the allowed resolution of the tree will be 
		 * multiplied by 2^(-n), but the tree structure itself
		 * will be unchanged.
		 *
		 * @param n   Number of levels by which to increase depth
		 */
		void increase_depth(unsigned int n);

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

		/*----------*/
		/* geometry */
		/*----------*/

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
		 * Will increase domain of the octree so box is contained
		 *
		 * Given a point and a halfwidth, will expand the
		 * domain of the octree to include the specified
		 * axis-aligned cube.
		 *
		 * @param p   The center of the box in question
		 * @param hw  The halfwidth of the box in question
		 *
		 * @return  Returns zero on success, non-zero on failure.
		 */
		int include_in_domain(const Eigen::Vector3d& p, double hw);

		/**
		 * Expands the structure of the tree at the given point
		 *
		 * Will grow the structure of the tree at the specified
		 * point so that the node with the specified halfwidth
		 * exists.  This node will be created if it does not
		 * already exist, and will be returned.
		 *
		 * If the tree has not been initialized with a resolution,
		 * then this function will return NULL.
		 *
		 * After this call, the value rd will contain the relative
		 * depth from the output returned node to the max depth
		 * of this tree.  In this sense, the output node can act
		 * as a tree with max depth rd.
		 *
		 * @param p   The point at which to expand the tree
		 * @param hw  The halfwidth of the desired node
		 * @param rd  Where to store the relative depth of output
		 * 
		 * @return    Returns a pointer to the node in question
		 */
		octnode_t* expand(const Eigen::Vector3d& p, double hw,
		                  unsigned int& rd);

		/**
		 * Will find all leaf nodes that overlap this shape
		 *
		 * This function will find all pre-existing leaf nodes
		 * in the tree that intersect with the given shape.  For
		 * each of these nodes found, the apply_to_leaf() function
		 * of the shape will be called on it.
		 *
		 * This function will NOT create any new nodes in the tree.
		 * 
		 * @param s   The shape to test
		 */
		void find(shape_t& s);

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
		 * Nodes that already exist and already have data will
		 * not be subdivided further, but their data may be
		 * changed by this call.
		 *
		 * After this call, all leaf nodes in the tree that
		 * are intersected by this line segment will be modified
		 * by the input shape.
		 *
		 * @param s   The shape to use to carve the tree
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		int insert(shape_t& s);

		/**
		 * Will subdivide the tree to the max depth based on its
		 * intersection with the given shape.
		 *
		 * Even if nodes already exist with data, they will be
		 * subdivided by this shape if an intersection occurs.
		 *
		 * Unlike both find(s) and insert(s), this function will
		 * NOT call s.apply_to_leaf() on any leaves encountered.
		 * In order to get that functionality, just call find(s)
		 * after this call.
		 *
		 * @param s   The shape to use to subdivide the tree
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int subdivide(const shape_t& s);

		/*-----*/
		/* i/o */
		/*-----*/

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

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Verifies that this tree is well-formed
		 *
		 * Will recursively iterate through the nodes of the
		 * tree, making sure that each node is legitimately formed.
		 *
		 * If an error is encountered, will print to stderr.
		 *
		 * @return   Returns zero on success, non-zero on failure
		 */
		int verify() const;
};

#endif
