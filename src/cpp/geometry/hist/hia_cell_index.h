#ifndef HIA_CELL_INDEX_H
#define HIA_CELL_INDEX_H

/**
 * @file   hia_cell_index.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The hia_cell_index_t defines a cell's spatial index
 *
 * @section DESCRIPTION
 *
 * This file contain the hia_cell_index_t class, which is used to
 * store the index position of a given 2D cell.
 */

#include <Eigen/Dense>
#include <set>

/**
 * The $name class stores the index position of a given 2D cell
 */
class hia_cell_index_t
{
	/* parameters */
	public:

		/**
		 * The index of the x-coordinate
		 */
		int x_ind;

		/**
		 * The index of the y-coordinate
		 */
		int y_ind;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default index
		 */
		hia_cell_index_t() : x_ind(0), y_ind(0)
		{};

		/**
		 * Copy constructor
		 *
		 * @param other   The index to copy
		 */
		hia_cell_index_t(const hia_cell_index_t& other)
			: x_ind(other.x_ind), y_ind(other.y_ind)
		{};

		/**
		 * Constructs an index at the given coordinates
		 *
		 * @param x   The x-coordinate of the index
		 * @param y   The y-coordinate of the index
		 */
		hia_cell_index_t(int x, int y) : x_ind(x), y_ind(y)
		{};

		/**
		 * Constructs index from a continuous position and
		 * a resolution
		 *
		 * @param res   The resolution of the cells
		 * @param p     The point to analyze
		 */
		hia_cell_index_t(double res, const Eigen::Vector2d& p)
		{ this->set(res, p); };

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Sets the contents of this index based on the given values
		 *
		 * @param x   The x-coordinate of the index
		 * @param y   The y-coordinate of the index
		 */
		inline void set(int x, int y)
		{
			this->x_ind = x;
			this->y_ind = y;
		};

		/**
		 * Sets the contents of this index based on a continuous
		 * position and a resolution.
		 *
		 * @param res   The resolution for the cell
		 * @param p     The continuous position
		 */
		inline void set(double res, const Eigen::Vector2d& p)
		{
			this->x_ind = (int) (p(0) / res);
			this->y_ind = (int) (p(1) / res);
		};

		/*----------*/
		/* analysis */
		/*----------*/

		/**
		 * Retrieves the possible neighbor positions around
		 * this index position.
		 *
		 * Note that the potential neighbors may not actually
		 * exist, so you will need to check the output of
		 * this function against the list of valid indices.
		 *
		 * Note that the neighbors will be added to the neighs
		 * set.  Any elements already in that set will still
		 * be present after this call.
		 *
		 * @param neighs   Where to store the neighbors.
		 */
		inline void get_neighs(
				std::set<hia_cell_index_t>& neighs) const
		{
			neighs.insert(hia_cell_index_t(this->x_ind-1,
							this->y_ind));
			neighs.insert(hia_cell_index_t(this->x_ind+1,
							this->y_ind));
			neighs.insert(hia_cell_index_t(this->x_ind,
							this->y_ind-1));
			neighs.insert(hia_cell_index_t(this->x_ind,
							this->y_ind+1));
		};

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Assigns the values of the argument index to this struct
		 *
		 * @param other   The index struct to copy
		 *
		 * @return        Returns the modified index struct
		 */
		inline hia_cell_index_t& operator = (
					const hia_cell_index_t& other)
		{
			this->x_ind = other.x_ind;
			this->y_ind = other.y_ind;

			return *this;
		};

		/**
		 * Checks equality between this and another index
		 *
		 * @param other   The other index to compare against
		 *
		 * @return        Returns true iff equal
		 */
		inline bool operator== (const hia_cell_index_t& other) const
		{
			return (this->x_ind == other.x_ind)
					&& (this->y_ind == other.y_ind);
		};

		/**
		 * Checks inequality between indices
		 *
		 * @param other    The other index to compare against
		 *
		 * @return         Returns true iff not equal
		 */
		inline bool operator!= (const hia_cell_index_t& other) const
		{
			return (this->x_ind != other.x_ind)
					|| (this->y_ind != other.y_ind);
		};

		/**
		 * Performs comparsion for sorting purposes
		 *
		 * @param other   The other index to compare against
		 *
		 * @return        Returns true iff this less than other
		 */
		inline bool operator < (const hia_cell_index_t& other) const
		{
			if(this->y_ind < other.y_ind)
				return true;
			if(this->y_ind > other.y_ind)
				return false;
			return (this->x_ind < other.x_ind);
		}
};

#endif
