#ifndef OCTHIST_2D_H
#define OCTHIST_2D_H

/**
 * @file     octhist_2d.h
 * @author   Eric Turner <elturner@eecs.berkeley.edu>
 * @brief    Performs a top-down 2D histogram of octree occupancy
 *
 * @section DESCRIPTION
 *
 * Will analyze an octree to determine the 2D histogram of occupancy
 * information in each node, projected onto the xy-axis.
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <iostream>
#include <map>

/**
 * The $name class is used to generate a histogram of octrees
 *
 * The generated histogram will represent a top-down projection
 * of the probabilities in each node onto the xy-axis.
 */
class octhist_2d_t : public shape_t
{
	/* the following type-definitions are used for convenience
	 * purposes in this class */
	typedef std::pair<int, int>           index_t;
	typedef std::map< index_t, double >   histmap_t;

	/* parameters */
	private:

		/**
		 * The histogram map.
		 *
		 * The histogram bins are arranged in a 2D grid.
		 * Each bin stores the aggregate values of all
		 * nodes that intersect it vertically.
		 */
		histmap_t cells;

		/**
		 * The resolution dicates the side-length of
		 * each 2D cell in the histogram.
		 *
		 * units: meters
		 */
		double resolution;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Constructs empty histogram
		 */
		octhist_2d_t()
		{ this->clear(); };

		/**
		 * Initialize the hisogram with an octree
		 *
		 * This will clear any information from this
		 * histogram and will repopulate it based on
		 * the values in the specified octree.
		 *
		 * @param octree    The input octree to analyze
		 * @param res       The resolution to use for
		 *                  the histogram.  If not specified,
		 *                  then will use the resolution of
		 *                  the octree.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int init(octree_t& octree, double res);
		int init(octree_t& octree);

		/**
		 * Clears all information in this histogram.
		 */
		void clear();

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Adds the specified weight to the cell covering
		 * the given position.
		 *
		 * @param x   The x-position to lookup
		 * @param y   The y-position to lookup
		 * @param w   The weight to add
		 */
		void insert(double x, double y, double w);

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports this histogram to a text-file stream
		 *
		 * Each line of the output stream will contain:
		 *
		 * 	<x_index> <y_index> <weight>
		 *
		 * Where the first two values represent the bin index
		 * and the third value is a double indicating the weight
		 * given to that cell.
		 *
		 * @param os  The output stream to write to
		 */
		void writetxt(std::ostream& os) const;

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
		 * Will analyze the specified data and store in this
		 * histogram.
		 *
		 * Given an octdata element at a specified location,
		 * will store in this histogram.
		 *
		 * @param c   The center of the data location
		 * @param hw  The halfwidth of the data location
		 * @param d   The data to analyze, may be null.
		 *
		 * @return   Returns d, unmodified.
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);

	/* helper functions */
	private:

		/**
		 * Gets the discretized bin index of a continuous value
		 * in this histogram.
		 *
		 * @param x   The x-index to lookup
		 * @param y   The y-index to lookup
		 *
		 * @return    Returns the index of the bin
		 */
		inline index_t get_index(double x, double y) const
		{
			index_t ind;

			/* populate index based on the resolution
			 * of this histogram */
			ind.first  = (int) (x / this->resolution);
			ind.second = (int) (y / this->resolution);

			/* return the index */
			return ind;
		};
};

#endif
