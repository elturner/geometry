#ifndef BUILDING_LEVELS_IO_H
#define BUILDING_LEVELS_IO_H

/**
 * @file    building_levels_io.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Reader/writer for building levels files
 *
 * @section DESCRIPTION
 *
 * This file contains the building_levels namespace, which has
 * classes used to read from and write to building levels files.
 * 
 * These files represent the horizontal partitioning of scanned buildings
 * into levels, or stories, based on floor and ceiling heights for each
 * level.
 */

#include <iostream>
#include <vector>
#include <string>

/**
 * The building_levels namespace contains reader and writer classes
 * for *.levels files.
 */
namespace building_levels
{
	/* the following classes are in this namespace */
	class header_t;
	class level_t;
	class file_t;

	/**
	 * This value represents the latest supported version
	 * of the .levels file format.
	 */
	const size_t MAJOR_VERSION = 1;
	const size_t MINOR_VERSION = 0;

	/**
	 * The header class can read and write headers from .levels
	 * files.
	 */
	class header_t
	{
		/* security */
		friend class file_t;

		/* parameters */
		private:

			/**
			 * The major version number of this header
			 */
			size_t major_version;

			/**
			 * The minor version number of this header
			 */
			size_t minor_version;

			/**
			 * The number of levels in file
			 *
			 * Valid files have at least one level.
			 */
			size_t num_levels;

		/* functions */
		public:

			/**
			 * Initializes default header
			 */
			header_t() : major_version(MAJOR_VERSION),
				minor_version(MINOR_VERSION),
				num_levels(0)
			{};
	};

	/**
	 * The level_t class represents a single building level
	 */
	class level_t
	{
		/* parameters */
		public:

			/**
			 * The index of this level 
			 *
			 * Levels are index from the bottom up, starting
			 * with index 0.
			 */
			size_t index;

			/**
			 * The floor height of this level.
			 *
			 * Must be less than the ceiling height.
			 *
			 * units: meters
			 */
			double floor_height;

			/**
			 * The ceiling height of this level
			 *
			 * Must be greater than the floor height.
			 *
			 * units: meters
			 */
			double ceiling_height;

		/* functions */
		public:

			/**
			 * Constructs an invalid level
			 */
			level_t() : index(0), 
			            floor_height(1.0),
				    ceiling_height(0.0)
			{};

			/**
			 * Construts a level from given values
			 *
			 * @param i   Index of level
			 * @param f   Floor height (units: meters)
			 * @param c   Ceiling height (units: meters)
			 */
			level_t(size_t i, double f, double c) :
				index(i), floor_height(f), ceiling_height(c)
			{};

			/**
			 * Returns true iff the level is valid
			 */
			inline bool is_valid() const
			{ 
				return (this->floor_height 
						< this->ceiling_height);
			};

			/**
			 * Copies level into this one
			 */
			inline level_t& operator = (const level_t& other)
			{
				this->index = other.index;
				this->floor_height = other.floor_height;
				this->ceiling_height = other.ceiling_height;
				return *(this);
			};
	};

	/**
	 * The file_t class is used to import and export .levels files
	 */
	class file_t
	{
		/* parameters */
		private:

			/**
			 * The header of the file
			 */
			header_t header;
			
			/**
			 * The list of the levels in the file
			 */
			std::vector<level_t> levels;

		/* functions */
		private:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Creates a default, empty file
			 */
			file_t()
			{};

			/**
			 * Clears all info from this structure
			 */
			void clear();

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Retrieves the number of levels
			 *
			 * If value is zero, then file is ill-defined.
			 */
			inline size_t num_levels() const
			{ return this->header.num_levels; };

			/**
			 * Retrieves the i'th level
			 *
			 * This call assumes that i is a valid level index
			 *
			 * @param i  The level index to retrieve
			 *
			 * @return   Const reference to the i'th level
			 */
			const level_t& get_level(size_t i) const
			{ return this->levels[i]; };

			/**
			 * Inserts a level into this structure
			 *
			 * If a level already exists at the specified
			 * index of lev, then it will be replaced.
			 *
			 * If the index of lev is >= num_levels,
			 * then the level list will be resized.
			 *
			 * If level is invalid, then will return failure.
			 *
			 * @param lev   The level to insert
			 *
			 * @return   Returns zero on success, non-zero on
			 *           failure.
			 */
			int insert(const level_t& lev);

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Parses a specified .levels file
			 *
			 * @param filename  The file to read
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int parse(const std::string& filename);

			/**
			 * Writes to specified .levels file
			 *
			 * @param filename   The file to write to
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int write(const std::string& filename) const;
	};
}

#endif
