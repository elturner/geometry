#ifndef PEOPLE_H
#define PEOPLE_H

/**
 * @file   people.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports .people files as part of a BIM object
 *
 * @section DESCRIPTION
 *
 * This file contains the people_t class, which is used to import
 * files of type *.people and to store their values.  These files
 * contain number of people for each room in a model.
 */

#include <vector>
#include <string>

/**
 * The people_t class contains a list of counts for each room
 * of a model that represent people.
 */
class people_t
{
	/* parameters */
	private:

		/**
		 * This vector represents the people counts
		 * for each room.
		 *
		 * The length of this vector is the same as the number
		 * of rooms in the model.
		 */
		std::vector<int> counts;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs default empty people struct.
		 */
		people_t() : counts() {};

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Clears all information from this object
		 */
		inline void clear()
		{ this->counts.clear(); };

		/**
		 * Retrieves wattage for the given room index
		 *
		 * @param i   The index of the room to analyze
		 *
		 * @return    Returns the power usage of the ceiling
		 *            people in the i'th room, in Watts.
		 */
		inline size_t get_room(size_t i) const
		{ return this->counts[i]; };

		/**
		 * Retrieves the number of rooms stored in this structure
		 *
		 * @return    Number of rooms stored here.
		 */
		inline size_t size() const
		{ return this->counts.size(); };

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Parses the specified .people file
		 *
		 * Given the path to the file that contains light
		 * counts, will import it and store the values
		 * in this structure.
		 *
		 * @param filename    The path to the .people file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int import(const std::string& filename);
};

#endif
