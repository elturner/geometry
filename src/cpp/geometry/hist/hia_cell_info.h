#ifndef HIA_CELL_INFO_H
#define HIA_CELL_INFO_H

/**
 * @file   hia_cell_info.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The hia_cell_info_t class holds info about a hia cell
 *
 * @section DESCRIPTION
 *
 * This file contains the hia_cell_info_t class.  This class is
 * used to store information about a single cell from a hia
 * (Histogrammed Interior Area) file, which contains a top-down
 * 2D histogram of the interior area for a level of a building
 * model.
 */

#include <io/hia/hia_io.h>
#include <Eigen/Dense>

/**
 * The hia_cell_info_t class stored information about a single
 * cell from a hia structure.
 */
class hia_cell_info_t
{
	/* parameters */
	public:

		/*---------------------*/
		/* original parameters */
		/*---------------------*/

		/**
		 * The center position of this cell
		 *
		 * units:  meters
		 */
		Eigen::Vector2d center;

		/**
		 * The min and max heights for the total extent
		 * of this cell.
		 *
		 * units: meters
		 */
		double min_z, max_z;

		/**
		 * The open height of this cell
		 *
		 * This value indicates how much of (max_z - min_z)
		 * is actually open, interior area.
		 *
		 * units:  meters
		 */
		double open_height;

		/*-------------------------*/
		/* intermediate parameters */
		/*-------------------------*/

		/**
		 * The sum of the 'open_height' fields for
		 * a neighborhood surrounding this cell.
		 *
		 * This value indicates the total sum of the
		 * values around this cell, including this cell's
		 * value itself.
		 *
		 * This sum is useful for the purposes of providing
		 * a low-pass filtered version of the building model,
		 * for the purposes of detecting room centers.
		 */
		double neighborhood_sum;

		/**
		 * The room index of this cell
		 *
		 * This value indicates what room the cell belongs
		 * to.  If this value is negative, then no room has
		 * been assigned yet.
		 */
		int room_index;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Makes empty (invalid) cell
		 */
		hia_cell_info_t()
			: 
				center(0,0), min_z(1), max_z(0),
				open_height(-1), neighborhood_sum(-1), 
			  	room_index(-1)
		{};

		/**
		 * Constructs cell from given cell i/o structure
		 *
		 * Given a hia::cell_t structure, will populate this
		 * object.
		 *
		 * @param c   The input cell structure
		 */
		hia_cell_info_t(const hia::cell_t& c)
			:
				center(c.center_x, c.center_y),
				min_z(c.min_z), max_z(c.max_z),
				open_height(c.open_height),
				neighborhood_sum(-1), room_index(-1)
		{};

		/**
		 * Sets the value of this object based on the input struct
		 *
		 * Given a hia::cell_t structure, will populate this
		 * object.  All other information in this object will
		 * be reset.
		 *
		 * @param c   The input cell structure.
		 */
		inline void init(const hia::cell_t& c)
		{
			this->center            << c.center_x, c.center_y;
			this->min_z             = c.min_z;
			this->max_z             = c.max_z;
			this->open_height       = c.open_height;
			this->neighborhood_sum  = -1;
			this->room_index        = -1;
		};

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Resets the neighborhood sum to zero
		 */
		inline void reset_sum()
		{ this->neighborhood_sum = 0; };

		/**
		 * Adds the given cell's open height to the neighborhood
		 * sum of this cell.
		 *
		 * @param neigh   The neighboring cell to add
		 */
		inline void add_neigh(const hia_cell_info_t& neigh)
		{ this->neighborhood_sum += neigh.open_height; };

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies the values of the argument structure into
		 * this structure.
		 *
		 * @param other   The other cell info to copy
		 *
		 * @return        Returns the modified cell.
		 */
		inline hia_cell_info_t& operator = (
					const hia_cell_info_t& other)
		{
			/* copy parameters */
			this->center           = other.center;
			this->min_z            = other.min_z;
			this->max_z            = other.max_z;
			this->open_height      = other.open_height;
			this->neighborhood_sum = other.neighborhood_sum;
			this->room_index       = other.room_index;

			/* return the modified struct */
			return *this;
		};
};

#endif
