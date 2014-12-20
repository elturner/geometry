#ifndef QUADTREE_H
#define QUADTREE_H

/**
 * @file   quadtree.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to define a quadtree
 *
 * @section DESCRIPTION
 *
 * This file defines a quadtree structure.
 * The quadtree represents all of 2D space, and
 * the bounding box grows as more elements are added.
 */

#include <geometry/quadtree/quadnode.h>
#include <geometry/quadtree/quaddata.h>
#include <geometry/shapes/linesegment_2d.h>
#include <Eigen/Dense>
#include <iostream>
#include <vector>

/**
 * defines the quadtree class 
 */
class quadtree_t
{
	/* parameters */
	private:

		/**
		 * Root of the tree and its relative position 
		 */
		quadnode_t* root;

		/**
		 * The tree expands down some max depth 
		 */
		int max_depth;

	/* function */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default constructor creates an empty quadtree 
		 */
		quadtree_t();

		/**
		 * Constructor creates an empty quadtree with a given
		 * resolution
		 */
		quadtree_t(double r); /* pass the max depth via grid res */

		/**
		 * Frees all memory and resources for this tree
		 */
		~quadtree_t();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Sets the span of this tree
		 *
		 * Will destroy any existing information, and set
		 * this tree to have the given center, width, and
		 * resolution.
		 *
		 * @param r   The resolution for the tree
		 * @param c   The center point for the tree
		 * @param hw  The halfwidth for the tree's root
		 */
		void set(double r, const Eigen::Vector2d& c, double hw);

		/**
		 * Sets the resolution to be the argument.
		 *
		 * Will destroy any information in tree.
		 *
		 * @param r   The resolution to use for this tree
		 */
		void set_resolution(double r);

		/**
		 * Returns the resolution of the max depth of the tree.
		 *
		 * @return   Returns the resolution of the tree, in meters.
		 */
		double get_resolution() const;

		/**
		 * Clears all information from tree.  
		 *
		 * set_resolution() must be called before adding more data.
		 */
		void clear();

		/**
		 * Will replace the information in this tree with a deep
		 * clone of the given other tree.
		 *
		 * Given a reference to another quadtree, will destroy
		 * information in this quadtree, and make a deep copy
		 * of the reference.
		 *
		 * @param other The other quadtree of which a deep copy will
		 * 		be made and stored in this quadtree.
		 */
		void clone_from(const quadtree_t& other);

		/**
		 * Inserts a point into the quadtree
		 *
		 * Given a point in space, will update the data
		 * structure in the correct leaf node with this
		 * point.
		 *
		 * @param p 	 The point to add to the structure
		 * @param n      The normal of the point
		 * @param w      The weighting to give this value
		 *
		 * @return       On success, returns pointer to the data p 
		 *               was incorporated into.  On failure, 
		 *               returns NULL.
		 */
		quaddata_t* insert(const Eigen::Vector2d& p,
					const Eigen::Vector2d& n,
					double w=1.0);

		/**
		 * Will insert a point with a given height range
		 *
		 * Given a point and its corresponding range of z-values,
		 * will insert the point and update that quaddata's
		 * z-range.
		 *
		 * @param p      The point to insert
		 * @param n      The normal of the point
		 * @param z_min  The min z-value for this point
		 * @param z_max  The max z-value for this point
		 * @param w      The weight to give this sample
		 *
		 * @return       Returns the appropriate data on success.
		 * 	         Returns NULL on error.
		 */
		quaddata_t* insert(const Eigen::Vector2d& p,
					const Eigen::Vector2d& n,
					double z_min, double z_max,
					double w=1.0);

		/**
		 * Inserts pose indices into the tree.
		 *
		 * Given a point and its pose index, will
		 * insert these indices into the appropriate node and
		 * return the corresponding data.  Note that the cell
		 * at this point should already exist.
		 *
		 * @param p          The point to retrieve
		 * @param pose_ind   The poses to add to pre-existing data
		 *
		 * @return    Returns the appropriate data on success.
		 * 	      Returns NULL on error.
		 */
		quaddata_t* insert(const Eigen::Vector2d& p,
					size_t pose_ind);

		/**
		 * Retrieves the data associated with a point in space
		 *
		 * Given a point, will return the quaddata at
		 * the deepest node that contains that point.  If no data
		 * exists there, NULL is returned.
		 *
		 * @returns 	Returns the appropriate data on success.
		 * 		Returns NULL if p out of range of tree.
		 */
		quaddata_t* retrieve(const Eigen::Vector2d& p) const;

		/**
		 * Returns a pointer to the root node of tree
		 *
		 * @return    Returns root of tree
		 */
		inline quadnode_t* get_root() const
		{ return this->root; };

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Retrieve nearest neighbor to given point
		 *
		 * Returns the quaddata that is
		 * closest to the given point.
		 *
		 * If all nodes are empty, returns null.
		 *
		 * @param p   The point to test
		 *
		 * @return    The data of the nearest neighbor to p
		 */
		quaddata_t* nearest_neighbor(
				const Eigen::Vector2d& p) const;
	
		/**
		 * Retrieves all cells within the given range of a point
		 *
		 * Will find all non-empty nodes within distance r
		 * to the point p.
		 *
		 * The resulting list of neighbors will not be sorted.
		 *
		 * @param p    The query point.
		 *
		 * @param r    The distance from p to check.
		 *             If r < 0, will assume infinite range.
		 *
		 * @param neighs    Will insert any relevent quaddatas into
		 *                  this structure.  It is not necessary
		 *                  for this structure to be empty upon
		 *                  call, new elements will be appended.
		 *
		 * @return      Returns 0 on success, non-zero on failure.
		 */
		int neighbors_in_range(const Eigen::Vector2d& p, double r,
				std::vector<quaddata_t*>& neighs) const;

		/**
		 * Finds all cells that intersect the given line segment
		 *
		 * Will determine which data elements intersect
		 * the specified line segment, and add them to 
		 * the specified list.
		 *
		 * @param xings      Where to store any data that 
		 *                   intersect the line segment.
		 * @param line       The line segment to analyze
		 */
		void raytrace(std::vector<quaddata_t*>& xings,
				const linesegment_2d_t& line) const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports this tree to given output stream
		 *
		 * Prints a representation of this tree to the
		 * specified ascii stream.  The tree can be
		 * perfectly reconstructed by using this information.
		 *
		 * format:
		 *
		 * 	<max_depth>
		 * 	<root_halfwidth>
		 * 	<root_center_x> <root_center_y>
		 * 	<data1>
		 * 	<data2>
		 * 	...
		 *
		 * 	Where <datai> indicates a line that was printed by
		 * 	a quaddata_t struct.
		 *
		 * @param os   The output stream to write to
		 */
		void print(std::ostream& os) const;

		/**
		 * Imports tree data from given stream
		 *
		 * Reads from the specified stream and creates the
		 * tree described.  This will destroy any existing data.
		 *
		 * Assumes the content of the stream is formatted in the
		 * same manner as quadtree_t::print().
		 *
		 * @param is   The input stream to read from
		 *
		 * @return     Returns 0 on success, non-zero on failure.
		 */
		int parse(std::istream& is);
};

#endif
