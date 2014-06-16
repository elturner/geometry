#ifndef CHUNK_DICT_H
#define CHUNK_DICT_H

/**
 * @file chunk_dict.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The chunk dictionary class identifies chunk files via locations
 *
 * @section DESCRIPTION
 *
 * This file contains the chunk_dict_t class, which is used to identify
 * which chunks contain which points in an efficient manner.  Given a
 * point in 3D space, it is able to return the file path to the chunk
 * file that intersects that point.
 */

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <Eigen/Dense>

/* the following classes are defined in this file */
class chunk_dict_t;
class chunk_key_t;

/**
 * The chunk_key_t class is used to has a chunk's position
 */
class chunk_key_t
{
	/* security */
	friend class chunk_dict_t;

	/* parameters */
	private:

		/* the 3D index of this chunk */
		long x_ind;
		long y_ind;
		long z_ind;
	
	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default constructor
		 */
		chunk_key_t()
		{ this->x_ind = this->y_ind = this->z_ind = 0; };

		/**
		 * Contructs key based on indices
		 *
		 * @param x  The index in the x direction
		 * @param y  The index in the y direction
		 * @param z  The index in the z direction
		 */
		chunk_key_t(long x, long y, long z)
		{
			this->x_ind = x;
			this->y_ind = y;
			this->z_ind = z;
		};

		/**
		 * Contructs key based on continuous position
		 *
		 * The input position should be shifted and scaled so
		 * that snapping to the nearest integer will be the key.
		 *
		 * @param p    The shifted/scaled position to key
		 */
		chunk_key_t(const Eigen::Vector3d& p);

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Sets this key's data to data from given key
		 *
		 * @param other   The other key to copy from
		 *
		 * @return        The modified key
		 */
		inline chunk_key_t& operator = (const chunk_key_t& other)
		{
			/* copy values */
			this->x_ind = other.x_ind;
			this->y_ind = other.y_ind;
			this->z_ind = other.z_ind;

			/* return the modified key */
			return (*this);
		};

		/**
		 * Checks equality between keys
		 *
		 * @param     The other key to check
		 *
		 * @return    Returns true iff this == other
		 */
		inline bool operator == (const chunk_key_t& other) const
		{
			return ( (this->x_ind == other.x_ind)
				&& (this->y_ind == other.y_ind)
				&& (this->z_ind == other.z_ind) );
		};

		/**
		 * Checks inequality between keys
		 *
		 * @param      The other key to check
		 *
		 * @return     Returns true iff this != other
		 */
		inline bool operator != (const chunk_key_t& other) const
		{
			return ( (this->x_ind != other.x_ind)
				|| (this->y_ind != other.y_ind)
				|| (this->z_ind != other.z_ind) );
		};

		/**
		 * Checks the ordering of this key
		 *
		 * Will return true iff this key is less than the
		 * given key.  This function allows for a unique
		 * sorting of keys.
		 *
		 * @param other    The other key to check
		 *
		 * @return    Returns true iff this < other
		 */
		inline bool operator < (const chunk_key_t& other) const
		{
			/* check each coordinate */
			if(this->x_ind < other.x_ind)
				return true;
			if(this->x_ind > other.x_ind)
				return false;
			if(this->y_ind < other.y_ind)
				return true;
			if(this->y_ind > other.y_ind)
				return false;
			return (this->z_ind < other.z_ind);
		};

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Prints index information to stream
		 */
		void print(std::ostream& os) const;
};

/**
 * The chunk_dict_t class maps 3D points to chunk files
 */
class chunk_dict_t
{
	/* parameters */
	private:

		/**
		 * This represents the mapping from positions to chunkfiles
		 */
		std::multimap<chunk_key_t, std::string> dict;

		/**
		 * This value represents the center position of the tree
		 *
		 * This position will be subtracted from all test positions
		 * to ensure the coordinate frame used to aligned.
		 */
		Eigen::Vector3d center;

		/**
		 * This value represents the width of the chunks in space.
		 *
		 * Each test position is divided by this value, to normalize
		 * the coordinates for discretization into indices.
		 */
		double width;

	/* functions */
	public:

		/**
		 * Initializes this dictionary based on the chunklist file
		 *
		 * Will parse the chunklist file, and corresponding chunk
		 * files, to populate the dictionary with these chunks.
		 *
		 * @param filename   The path to the .chunklist file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& filename);

		/**
		 * Retrieves the chunk file that intersects the given point
		 *
		 * Given a point in 3D space, will determine which chunk
		 * file(s) corresponds to the point.
		 *
		 * @param p   The point to look up
		 * @param ss  The set of strings to add retrieved element to
		 */
		void retrieve(const Eigen::Vector3d& p,
			std::set<std::string>& ss) const; 

	/* helper functions */
	private:

		/**
		 * Generates a key object from a position
		 */
		inline chunk_key_t genkey(const Eigen::Vector3d& p) const
		{
			return chunk_key_t((p - this->center)/this->width);
		};
};

#endif
