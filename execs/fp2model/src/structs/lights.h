#ifndef LIGHTS_H
#define LIGHTS_H

/**
 * @file   lights.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports .lights files as part of a BIM object
 *
 * @section DESCRIPTION
 *
 * This file contains the lights_t class, which is used to import
 * files of type *.lights and to store their values.  These files
 * contain wattages for the ceiling lights for each room in a model.
 */

#include <vector>
#include <string>

/**
 * The lights_t class contains a list of wattages for each room
 * of a model that represent ceiling lights.
 */
class lights_t
{
	/* parameters */
	private:

		/**
		 * This vector represents the ceiling light wattages
		 * for each room.
		 *
		 * The length of this vector is the same as the number
		 * of rooms in the model.
		 */
		std::vector<double> wattages;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs default empty lights struct.
		 */
		lights_t() : wattages() {};

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Clears all information from this object
		 */
		inline void clear()
		{ this->wattages.clear(); };

		/**
		 * Retrieves wattage for the given room index
		 *
		 * @param i   The index of the room to analyze
		 *
		 * @return    Returns the power usage of the ceiling
		 *            lights in the i'th room, in Watts.
		 */
		inline double get_room(size_t i) const
		{ return this->wattages[i]; };

		/**
		 * Retrieves the number of rooms stored in this structure
		 *
		 * @return    Number of rooms stored here.
		 */
		inline size_t size() const
		{ return this->wattages.size(); };

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Parses the specified .lights file
		 *
		 * Given the path to the file that contains light
		 * wattages, will import it and store the values
		 * in this structure.
		 *
		 * @param filename    The path to the .lights file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int import(const std::string& filename);
};

#endif
