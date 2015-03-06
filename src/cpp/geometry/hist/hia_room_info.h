#ifndef HIA_ROOM_INFO_H
#define HIA_ROOM_INFO_H

/**
 * @file    hia_room_info.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Stores information about rooms in a hia map
 *
 * @section DESCRIPTION
 *
 * The hia_room_info_t class stores information about each room
 * in a hia map.  This information includes which cells belong
 * to each room.
 */

#include <geometry/hist/hia_cell_index.h>
#include <set>

/**
 * The hia_room_info_t class stores the set of cells in each room
 */
class hia_room_info_t
{
	/* parameters */
	private:

		/**
		 * The set of cells in this room
		 */
		std::set<hia_cell_index_t> cells;

		// TODO bounding box, dominant axes, etc.?

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Creates empty room
		 */
		hia_room_info_t() : cells()
		{};

		/**
		 * Creates room with a single cell
		 *
		 * @param c   The seed cell for this room
		 */
		hia_room_info_t(const hia_cell_index_t& c)
		{ this->insert(c); };

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Returns the number of cells in this room
		 *
		 * @return    Number of cells in room.
		 */
		inline size_t size() const
		{ return this->cells.size(); };

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Clears the room of all cells
		 */
		inline void clear()
		{ this->cells.clear(); };

		/**
		 * Inserts a cell into this room
		 * 
		 * @param ci   The cell index to insert
		 */
		inline void insert(const hia_cell_index_t& ci)
		{ this->cells.insert(ci); };
};


#endif
