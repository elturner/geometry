#ifndef OCTNODE_H
#define OCTNODE_H

/**
 * @file octnode.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the octnode_t class, which is used as a node
 * in an octree.  This class specializes in ray-tracing functions
 * through octrees.
 */

#include <Eigen/Dense>
#include <iostream>
#include "shape.h"
#include "octdata.h"

/* number of children each node in the octree has */
#define CHILDREN_PER_NODE 8

/**
 * defines the individual nodes of a octree
 */ 
class octnode_t
{
	/* parameters */
	public:

		/* each node has pointers to its children.
		 * These pointers being null implies this
		 * node is a leaf. */
		octnode_t* children[CHILDREN_PER_NODE];

		/* octnodes have geometry, such as center position
		 * and size.  The position is relative to the origin
		 * of the tree. */
		Eigen::Vector3d center;
		double halfwidth; /* distance from center to edge */

		/* each node also stored data elements */
		octdata_t* data; /* only non-null for leaves */

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/
		
		/* eigen constructions must be properly aligned */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		/**
		 * Constructs empty leaf node
		 */
		octnode_t();

		/**
		 * Constructs leaf node with given position and width
		 */
		octnode_t(const Eigen::Vector3d& c, double hw);
		
		/**
		 * Frees all memory and resources
		 */
		~octnode_t();

		/**
		 * Frees all dynamically allocated memory and info
		 *
		 * After this call, the node will still have valid geometry,
		 * but will be an empty leaf node with no data.
		 */
		void clear();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Returns true iff this node is a leaf.
		 *
		 * @return   Returns true iff this node is a leaf
		 */
		bool isleaf() const;

		/**
		 * After call, i'th child will be initialized
		 *
		 * Will initialize the i'th child of this node,
		 * assuming it is not already initialized.  If i'th node 
		 * already exists, it will not be modified.  Otherwise, 
		 * it will be a valid empty leaf node after this call.
		 *
		 * @param i  The index of the child node to initialize
		 */
		void init_child(int i);

		/**
		 * Clones this node as a deep copy
		 *
		 * Allocates new memory that is a deep copy of
		 * this node and its subnodes.
		 *
		 * @return  Returns a deep copy of this node
		 */
		octnode_t* clone() const;

		/*----------*/
		/* geometry */
		/*----------*/
	
		/**
		 * Checks if a given point is within the volume of this node
		 *
		 * If the given point is outside the bounds of this node,
		 * then a negative value is returned.  If it is inside 
		 * the bounds of this node, then a value in [0,8) will
		 * be returned, which specifies the child node index that
		 * contains this point.
		 *
		 * @param p  The point to test
		 *
		 * @return  Returns negative iff p not contained in node
		 */
		int contains(const Eigen::Vector3d& p) const;

		/**
		 * Will attempt to simplify this node
		 *
		 * A simplification can occur when every child exists
		 * and is a leaf.  If the data for all these children
		 * fits simplification thresholds, then the children's
		 * data will be merged, and this node will become a
		 * leaf with the merged data.
		 *
		 * Note that this function call is NOT recursive.  It
		 * will only check if this node can be simplified, not
		 * its subnodes.
		 *
		 * @return   Returns true iff the node was simplified
		 */
		bool simplify();
	
		/*-----------------*/
		/* recursive calls */
		/*-----------------*/

		/**
		 * Will recursively simplify this node and its subnodes
		 *
		 * Will attempt to simplify the subnodes of this node
		 * recursively, and if successful, will attempt to
		 * simplify this node as well.
		 *
		 * @return  Returns true iff the node was fully simplified
		 */
		bool simplify_recur();

		/**
		 * Gets the leaf node that contains this point.
		 *
		 * Will recurse through the children of this node, and
		 * find the deepest subnode that contains this point.  If
		 * this point is not contained by this node, null is
		 * returned.
		 *
		 * @param p  The point to test
		 *
		 * @return   Returns pointer to leaf node containing p.
		 */
		octnode_t* retrieve(const Eigen::Vector3d& p) const;

		/**
		 * Will expand tree structure so p is covered at depth d
		 *
		 * Will recursively expand the tree structure that
		 * intersects the given point, so that the node d-levels
		 * down exists.  Will return this node.
		 *
		 * If the tree is already expanded to this depth, then
		 * no modification to the structure will occur.
		 *
		 * @param p  The point at which to expand
		 * @param d  The relative depth to expand
		 *
		 * @return   Returns the node at relative depth d covering p
		 */
		octnode_t* expand(const Eigen::Vector3d& p, unsigned int d);

		/**
		 * Will find all leaf nodes that overlap this shape
		 *
		 * This function will find all pre-existing leaf nodes
		 * in the tree that intersect with the given shape.  For
		 * each of these nodes found, the apply_to_leaf() function
		 * of the shape will be called on it.
		 *
		 * @param s   The shape to test
		 */
		void find(shape_t& s);

		/**
		 * Will insert shape into node, updating tree structure
		 *
		 * This function will add subnodes to this node, either
		 * to the specified depth or until a depth at which
		 * a node already has data stored.
		 *
		 * After this call, all leaf nodes under this node that
		 * are intersected by this line segment are given to
		 * the input shape, where s.apply_to_leaf() is called
		 * on each of their data elements.
		 *
		 * This function is recursive, and will perform intersection
		 * checks for all subnodes, but NOT for the top-level
		 * node.  Only call this function after checking top-level
		 * node against the given line.
		 *
		 * @param s      The shape to insert and analyze
		 * @param d      The depth at which to carve
		 */
		void insert(shape_t& s, int d);

		/**
		 * Returns the count of this node and all its subnodes
		 *
		 * Will recursively count the number of subnodes under
		 * this node, and return the value (the count includes
		 * this node).
		 *
		 * @return   Returns the count of allocated nodes.
		 */
		unsigned int get_num_nodes() const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Will serialize this node to specified binary stream
		 *
		 * Will write information of this node and its subnodes
		 * to the specified binary stream, whcih can then be parsed
		 * by the parse() function.
		 *
		 * @param os   The binary output stream to serialize
		 */
		void serialize(std::ostream& os) const;

		/**
		 * Will parse tree information from specified binary stream
		 *
		 * Will read information from the specified stream to
		 * populate this node and its subnodes.  This stream
		 * should be formatted in the manner as produced by
		 * the serialize() function.
		 *
		 * Any information in this node before this call will
		 * be destroyed.
		 *
		 * @param is   The binary input stream to parse
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int parse(std::istream& is);

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Runs various sanity checks to verify that the
		 * invarients of this structure are met by this octnode
		 *
		 * Will recursively verify this node and its subnodes to
		 * make sure that this part of the octree is valid and
		 * well-formed.
		 *
		 * If an error is found, it will print to stderr.
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		int verify() const;
};

/***** Helper Functions *****/

/* The ordering of the children for each leaf is as follows:
 *
 * 		y
 *              ^
 *       1      |      0
 *              |
 * -------------+-------------> x	(top, z+)
 *              |
 *       2      |      3
 *              |
 *
 * 		y
 *              ^
 *       5      |      4
 *              |
 * -------------+-------------> x	(bottom, z-)
 *              |
 *       6      |      7
 *              |
 */
inline Eigen::Vector3d relative_child_pos(int child_index)
{
	Eigen::Vector3d v;

	/* returns the relative position of a child
	 * with respect to its parent's center, with each
	 * dimension of size 1 */
	switch(child_index)
	{
		/* top children */
		case 0:
			v << 1, 1, 1;
			break;
		case 1:
			v << -1, 1, 1;
			break;
		case 2:
			v << -1,-1, 1;
			break;
		case 3:
			v <<  1,-1, 1;
			break;

		/* bottom children */
		case 4:
			v << 1, 1,-1;
			break;
		case 5:
			v << -1, 1,-1;
			break;
		case 6:
			v << -1,-1,-1;
			break;
		case 7:
			v <<  1,-1,-1;
			break;
		
		/* invalid input */
		default:
			v << 0, 0, 0;
			break;
	}

	/* return position */
	return v;
};

#endif
