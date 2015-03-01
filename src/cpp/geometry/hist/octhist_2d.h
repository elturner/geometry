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

#include <io/hia/hia_io.h>
#include <io/levels/building_levels_io.h>
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
	typedef std::pair<int, int>                index_t;
	typedef std::map< index_t, hia::cell_t >   histmap_t;

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

		/**
		 * The building level to operate on
		 *
		 * By default, the entire elevation is considered
		 * and defined as level "0".  However, a specific
		 * level can be specified, which limits the aspect of
		 * the building referenced.
		 */
		building_levels::level_t level;

		/**
		 * The following represent the indices of the current
		 * cell being analyzed.
		 */
		index_t current_index;

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
		 * @param lev       The building level to use (OPTIONAL)
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int init(octree_t& octree, double res, 
				const building_levels::level_t& lev);
		int init(octree_t& octree,
				const building_levels::level_t& lev);
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
		 * @param x    The x-position to lookup
		 * @param y    The y-position to lookup
		 * @param minz The minimum z-coordinate of this sample
		 * @param maxz The maximum z-coordinate of this sample
		 * @param w    The weight to add
		 */
		void insert(double x, double y, double minz, double maxz,
					double w);

		/**
		 * Adds the specified weight to the cell covering
		 * the given index.
		 *
		 * @param ind  The cell index referenced
		 * @param minz The minimum z-coordinate of this sample
		 * @param maxz The maximum z-coordinate of this sample
		 * @param w    The weight to add
		 */
		void insert(const index_t& ind, double minz, double maxz,
					double w);
		
		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Computes the min and max height bounds based on
		 * the cell content.
		 *
		 * Will compute the bounds of the heights described in
		 * the cells.  These values may be different than the
		 * level floor and ceiling heights.
		 *
		 * @param minz  Where to store the min height
		 * @param maxz  Where to store the max height
		 */
		void compute_height_bounds(double& minz, 
				double& maxz) const;

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

		/**
		 * Exports this histogram to a .hia file
		 *
		 * Histrogrammed Interior Area (HIA) files
		 * represent the interior volume of a building
		 * as a 2D top-down histogram.
		 *
		 * @param filename    Where to write the .hia file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int writehia(const std::string& filename) const;

		/*----------------------------*/
		/* overloaded shape functions */
		/*----------------------------*/

		/**
		 * Overloaded from shape_t class.
		 *
		 * Always returns one
		 *
		 * @return   Returns one
		 */
		inline unsigned int num_verts() const
		{ return 1; };
		
		/**
		 * Overloaded from shape_t class.
		 *
		 * Returns bin center of current index.
		 *
		 * @param i   Unused.
		 *
		 * @return    Returns current index bin center.
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			MARK_USED(i);
			return this->bin_center(this->current_index);
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
		bool intersects(const Eigen::Vector3d& c, double hw) const;
		
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

		/**
		 * Gets the center position of the specified index
		 *
		 * @param ind   The index to analyze.
		 *
		 * @return  Returns the zero-height center position
		 * of the given bin index.
		 */
		inline Eigen::Vector3d bin_center(const index_t& ind) const
		{
			Eigen::Vector3d c;

			/* get center position */
			c(0) = (ind.first + 0.5) * this->resolution; 
			c(1) = (ind.second + 0.5) * this->resolution; 
			c(2) = 0.0;
			
			/* return it */
			return c;
		};
};

#endif
