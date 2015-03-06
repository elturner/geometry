#ifndef HIA_ANALYZER_H
#define HIA_ANALYZER_H

/**
 * @file   hia_analyzer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports data from hia file, performs room analysis
 *
 * @section DESCRIPTION
 *
 * The hia_analyzer_t class will perform geometric analysis on the
 * contents of a hia file in order to facilitate floorplan generation.
 */

#include <geometry/shapes/bounding_box.h>
#include <geometry/hist/hia_cell_index.h>
#include <geometry/hist/hia_cell_info.h>
#include <geometry/hist/hia_room_info.h>
#include <iostream>
#include <string>
#include <map>
#include <set>

/**
 * The hia_analyzer_t class will perform geometric analysis on the
 * contents of a hia file in order to faciilate floorplan generation.
 */
class hia_analyzer_t
{
	/* the following type-definitions are used for convenience
	 * purposes in this class */
	typedef std::map<hia_cell_index_t, hia_cell_info_t>   cellmap_t;
	typedef std::map<hia_cell_index_t, hia_room_info_t>   roommap_t;

	/* paramters */
	private:

		/*------------------------*/
		/* level-wide information */
		/*------------------------*/

		/**
		 * The level index this object represents
		 */
		int level;

		/**
		 * The bounding box for the cells in this
		 * histogram.
		 *
		 * Note that the x,y bounds will contian
		 * the cells, but the z-bounds may not.  If
		 * they don't, then it is because they
		 * represent the estimated floor/ceiling
		 * heights, whereas an individual cell may
		 * exceed the floor or ceiling.
		 *
		 * units: meters
		 */
		bounding_box_t bounds;

		/**
		 * The resolution of each cell in this object
		 *
		 * units:  meters
		 */
		double resolution;

		/*------------------*/
		/* cell information */
		/*------------------*/

		/**
		 * The mapping between a unique index for each
		 * cell to the information about that cell.
		 */
		cellmap_t cells;

		/*------------------*/
		/* room information */
		/*------------------*/

		/**
		 * The set of rooms in this map
		 *
		 * Each room is made up of a subset of cells
		 */
		roommap_t rooms;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Creates an empty analyzer with invalid fields.
		 */
		hia_analyzer_t() :
			level(-1),bounds(),resolution(-1),cells(),rooms()
		{};

		/**
		 * Initializes this structure by importing .hia file
		 *
		 * Any information stored in this structure before this
		 * call will be discarded.  The file will be parsed and
		 * the structure will be populated/initialized based on
		 * the file contents.
		 *
		 * @param filename   The input file to parse
		 *
		 * @return           Returns zero on success,
		 * 			non-zero on failure.
		 */
		int readhia(const std::string& filename);

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Iterates over the cells, computing sum of neighborhood
		 * values of open_heights.
		 *
		 * Will populate the neighborhood_sum field of each cell
		 * with the sum of its neighborhood's values, given
		 * a radius for that neighborhood.
		 *
		 * @param dist   The radius to search for each cell
		 *
		 * @return       Returns zero on success, 
		 * 			non-zero on failure.
		 */
		int populate_neighborhood_sums(double dist);

		/**
		 * Labels local maxima within the cell map
		 *
		 * Must be called after populate_neighborhood_sums().  A
		 * local max is any cell that has the largest neighborhood
		 * sum of any cell in its neighborhood.
		 *
		 * If a cell is a local max, it will be given a unique,
		 * non-zero room id.  If a cell is not a local max, it
		 * will be given a room id of -1.
		 *
		 * @param dist   The neighborhood radius distance.
		 *
		 * @return       Returns zero on success, 
		 * 			non-zero on failure.
		 */
		int label_local_maxima(double dist);

		/**
		 * Assigns a room index to every cell
		 *
		 * This must be called after label_local_maxima() has
		 * been run successfully.
		 *
		 * Will propegate room indices from the labeled seed cells
		 * and label each non-seed cell with the room index that
		 * is topologically closest to it.
		 *
		 * Note that open_height counts as inverse distance, so
		 * going through several cells with low open_height will
		 * be considered farther away than the same number of cells
		 * with greater open_height.
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int propegate_room_labels();

		// TODO

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes the neighborhood sum values for each stored
		 * cell.
		 *
		 * Will export neighborhood sum info to the specified
		 * stream.  Each line will contain:
		 *
		 * 	<index_x> <index_y> <neighborhood_sum>
		 *
		 * @param os  The output stream to write to
		 */
		void write_neighborhood_sums(std::ostream& os) const;

		/**
		 * Writes the locations of the local maxima
		 *
		 * Will export info to the specified stream.  Each
		 * line will contain:
		 *
		 * 	<center_x> <center_y> <room_index>
		 *
		 * For each cell that has non-negative room_index
		 *
		 * @param os   The output stream to write to
		 */
		void write_localmax(std::ostream& os) const;

	/* helper functions */
	private:

		/**
		 * Generates the index for the given position
		 *
		 * Given a continuous 2D position, will return
		 * the index that occupies that position.
		 *
		 * @param p   The position to analyze
		 *
		 * @return    Returns the index of p
		 */
		inline hia_cell_index_t get_index_of(
				const Eigen::Vector2d& p) const
		{
			hia_cell_index_t ind;

			/* check if we have a valid resolution */
			if(this->resolution <= 0)
				return ind;

			/* populate the index */
			ind.set(this->resolution, p);

			/* return the index */
			return ind;
		};

		/**
		 * For a given cell, will find all neighboring cells within
		 * the specified distance.
		 *
		 * Will find any cells that can be reached from the seed
		 * cell by not going more than the given distance away from
		 * the seed cell.
		 *
		 * The seed cell is considered a neighbor of itself.
		 *
		 * neighs will not be cleared before processing. Any values
		 * in this set before this call will remain in this set.
		 *
		 * @param seed    The index of the seed cell
		 * @param dist    The maximum distance to travel
		 * @param neighs  Where to store all neighboring cells.
		 *
		 * @return        Returns zero on success, 
		 * 			non-zero on failure.
		 */
		int neighbors_within(const hia_cell_index_t& seed,
					double dist, 
				std::set<hia_cell_index_t>& neighs) const;
};

#endif
