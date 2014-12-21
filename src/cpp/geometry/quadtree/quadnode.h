#ifndef QUADNODE_H
#define QUADNODE_H

/**
 * @file    quadnode.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   A node in the quadtree structure
 *
 * @section DESCRIPTION
 *
 * This file contains the quadnode_t class, which is used to
 * represent a single node in a quadtree.  A quadnode also contains
 * pointers to all its subnodes.
 */

#include <geometry/quadtree/quaddata.h>
#include <geometry/shapes/linesegment_2d.h>
#include <Eigen/Dense>
#include <vector>
#include <iostream>

/**
 * defines the individual nodes of a quadtree
 */ 
class quadnode_t
{
	/* parameters */
	public:

		/**
		 * The following value indicates the number
		 * of children per node in this tree.  Since
		 * this is a quadtree, the value is 4 */
		static const size_t CHILDREN_PER_QUADNODE = 4;

		/**
		 * each node has pointers to its children.
		 * These pointers being null implies this
		 * node is a leaf. 
		 *
		 *
		 *              |
		 *       1      |      0
		 *              |
		 * -------------+--------------
		 *              |
		 *       2      |      3
		 *              |
		 */
		quadnode_t* children[CHILDREN_PER_QUADNODE];

		/**
		 * quadnodes have geometry, such as center position
		 * and size.  The position is relative to the origin
		 * of the tree. */
		Eigen::Vector2d center;
		double halfwidth; /* distance from center to edge */

		/**
		 * each node also stored data elements 
		 */
		quaddata_t* data; /* only non-null for leaves */

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default constructor of empty node
		 */
		quadnode_t();
		
		/**
		 * Constructs a node with a given position and size
		 *
		 * @param c   The center of the node
		 * @param hw  The halfwidth of the node
		 */
		quadnode_t(const Eigen::Vector2d& c, double hw);

		/**
		 * Frees all memory and resources
		 */
		~quadnode_t();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Checks if this node is a leaf
		 *
		 * A node is a leaf iff all of its children are null.
		 *
		 * Only leafs contain data.
		 *
		 * @return    Returns true iff this node is a leaf.
		 */
		bool isleaf() const;

		/**
		 * Checks if this node is empty
		 *
		 * A node is empty if it has no data, or the data have no
		 * sample points.
		 *
		 * @return   Returns true iff this node is empty.
		 */
		bool isempty() const;

		/**
		 * Initializes the i'th child of this node
		 *
		 * After this call, the i'th child will exist as a valid 
		 * node.
		 *
		 * Will initialize the i'th child of this node,
		 * assuming it is not already initialized.
		 *
		 * @param i    The child index to initialize
		 */
		void init_child(size_t i);

		/**
		 * Makes a deep copy of this node
		 *
		 * Allocates new memory that is a deep copy of
		 * this node and its subnodes.
		 *
		 * @return   A deep copy of this node
		 */
		quadnode_t* clone() const;

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Checks if this node contains the given point
		 *
		 * If this point is contained in this node, then
		 * the index of the chld node that contains this point
		 * is returned.  If the point is not contained in this node
		 * then a negative value is returned.
		 *
		 * @param p  The point to analyze
		 *
		 * @return   Returns child index that contains p, or -1
		 *           if p is not in node.
		 */
		int contains(const Eigen::Vector2d& p) const;

		/**
		 * Checks if the given line segment intersects this node
		 *
		 * Returns true iff this node is intersected
		 * by the given 2D line segment.
		 *
		 * @param line    The line segment to test
		 *
		 * @return    Returns true iff intersection occurs
		 */
		bool intersects(const linesegment_2d_t& line) const;

		/**
		 * Get the center position of the i'th child, if it
		 * existed.
		 *
		 * Will return the center position of the i'th child.
		 * This call does not depend on the existance of this
		 * child.
		 *
		 * @param i  The index of the child to check
		 *
		 * @return   Returns center position for this child.
		 */
		Eigen::Vector2d child_center(size_t i) const;

		/*-----------------*/
		/* recursive calls */
		/*-----------------*/

		/**
		 * Subdivides the tree so that nodes exist
		 * in the given bounds.
		 *
		 * No data will be stored at these nodes, only the nodes
		 * themselves will be created.
		 *
		 * Will ignore any part of the input domain that is
		 * out of bounds of this node.
		 *
		 * @param xs    The {xmin, xmax} bounds of box
		 * @param ys    The {ymin, ymax} bounds of box
		 * @param input_hw   The resolution at which to 
		 *                   stop subdividing
		 */
		void subdivide(double xs[2], double ys[2], double input_hw);

		/**
		 * Simplifies the tree structure.
		 *
		 * Will not simplify any nodes that contain data
		 *
		 * If a node has all children with all of the following
		 * qualities:
		 * 	- non-null
		 * 	- leaf
		 * 	- no data
		 *
		 * Then the children will be removed.  This process
		 * is performed bottom-up recursively.
		 *
		 * @return    Returns true iff node has been simplified.
		 */
		bool simplify();

		/**
		 * Gets the neighboring nodes of this node that occur
		 * under the specified parent node.
		 *
		 * For all nodes under the parent node given, will check
		 * if they neighbor this node.  If so, they will be
		 * added to the output container 'neighs'.
		 *
		 * @param neighs   Where to store neighboring nodes
		 * @param parent   The parent node to recursively check
		 *                 for neighbors.
		 * @param err      The error parameter for testing abutting
		 *                 sides.  This distance should be smaller
		 *                 than any node feature.
		 */
		void get_neighbors_under(std::vector<quadnode_t*>& neighs,
						quadnode_t* parent,
						double err) const;

		/**
		 * Inserts a point into the subtree of this node
		 *
		 * Will insert the given point into this node
		 * or one of its children.  Will force the
		 * insertion to the specified relative depth,
		 * creating new children if necessary.
		 *
		 * @param p     The point to add to the tree.
		 * @param n     The normal vector of this point
		 * @param w     The weight of the point
		 * @param d     The relative depth at which to add
		 *		this point.  Will add to the current
		 *		level if d=0.
		 *
		 * @return   Returns a pointer to the leaf node that p was
		 *           added to.  Returns NULL on error.
		 */
		quaddata_t* insert(const Eigen::Vector2d& p, 
					const Eigen::Vector2d& n,
					double w, int d); 

		/**
		 * Retrieves the data at the given point
		 *
		 * Will return the leaf node that contains the
		 * given point.
		 *
		 * @param p    The query point.
		 *
		 * @return     Returns node that is a leaf and contains p.
		 *             Returns NULL on error.
		 */
		const quadnode_t* retrieve(const Eigen::Vector2d& p) const;

		/**
		 * Gets the nearest neighbor for the given point
		 *
		 * Will compare this node and its children to their
		 * proximity to point p compared to the best quadnode
		 * seen so far.
		 *
		 * If this node or one of its decendents are closer to
		 * p, then the value of best_so_far will be updated.
		 *
		 * @param best_so_far	If null, will insert a non-empty
		 * 			decendent that is close to p.
		 * 			If non-null, will only insert
		 * 			decendents that are closer than the
		 * 			current value is to p.
		 *
		 * @param p 		The query point.
		 *
		 * @return     Returns 0 on success, non-zero on failure.
		 */
		int nearest_neighbor(quaddata_t** best_so_far, 
					const Eigen::Vector2d& p) const;

		/**
		 * Retrieves all the subnodes in the given range
		 *
		 * Searches this node and all its decendants for all
		 * quaddatas that fall within distance r of p.
		 *
		 * Stores qualifying pointers in provided list.
		 *
		 * @param p       The query point
		 * @param r       The distance to query point.  If r < 0,
		 *                will use infinite range.
		 * @param neighs  The list to modify with candidate
		 *                quaddatas
		 *
		 * @return     Returns 0 on success, non-zero on failure.
		 */
		int nodes_in_range(const Eigen::Vector2d& p, double r,
				std::vector<quaddata_t*>& neighs) const;

		/**
		 * Will find all subnodes intersecting given line segment.
		 *
		 * Will ray trace between the points provided.  If
		 * any data under this node intersect this line segment,
		 * they will be inserted into the specified list.
		 *
		 * @param xing   Where to store data that intersects
		 * @param line   The line segment representing this ray
		 */
		void raytrace(std::vector<quaddata_t*>& xings,
				const linesegment_2d_t& line) const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports this node and its children to the given stream
		 *
		 * Prints the data under this node to the
		 * specified ascii stream.
		 *
		 * @param os   The output stream to write to
		 */
		void print(std::ostream& os) const;
};

#endif
