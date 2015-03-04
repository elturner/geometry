#ifndef HIA_IO_H
#define HIA_IO_H

/**
 * @file   hia_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Reader/writer for the .hia file format.
 *
 * @section DESCRIPTION
 *
 * The .hia (Histogrammed Interior Area) file format stores
 * a top-down 2D histogram of a building model's interior volume.
 *
 * This file stores the local floor and ceiling heights
 * for each 2D cell, as well as the amount of interior height
 * occurs within that cell.
 *
 * It is intended to be used for the generation of building floorplans
 *
 * Created February 25, 2015
 */

#include <string>
#include <fstream>
#include <iostream>

/**
 * The hia_io namespace stores classes used to read/write .hia files
 *
 * The .hia (Histogrammed Interior Area) file format stores
 * a top-down 2D histogram of a building model's interior volume.
 *
 * This file stores the local floor and ceiling heights
 * for each 2D cell, as well as the amount of interior height
 * occurs within that cell.
 *
 * It is intended to be used for the generation of building floorplans
 */
namespace hia
{
	/* the following classes are defined in this file */
	class header_t;
	class cell_t;
	class reader_t;
	class writer_t;

	/*-----------------------------------------------------*/
	/* the following constants are used in the file format */
	/*-----------------------------------------------------*/

	/**
	 * The magic number appears at the top of every valid .hia file
	 */
	static const std::string MAGIC_NUMBER = "hiafile";
	static const size_t MAGIC_NUMBER_SIZE = 8; /* includes \0 */

	/**
	 * The major version of the file.
	 *
	 * Files with major version numbers different than this
	 * are not supported.
	 */
	static const int VERSION_MAJOR = 1;

	/**
	 * The minor version of the file.
	 *
	 * Files with minor version numbers different than this
	 * will still be supported, as long as the major version
	 * is the same.
	 */
	static const int VERSION_MINOR = 0;

	/*--------------------*/
	/* the header_t class */
	/*--------------------*/

	/**
	 * The header_t class reads and writes header information
	 * from and to a .hia file.
	 *
	 * The header contains metadata about the file, such as
	 * version number or level index.
	 */
	class header_t
	{
		/* security */
		friend class reader_t;
		friend class writer_t;

		/* parameters */
		private:
			
			/**
			 * The format version of the corresponding file
			 */
			int version_major, version_minor;

			/**
			 * The level index
			 *
			 * This value indicates which level of a building
			 * model is represented by the corresponding file.
			 *
			 * This value is zero-indexed.
			 *
			 * If a building is only single-storied, then
			 * the value should be 0.
			 */
			int level_index;
			
			/**
			 * The number of cells in the file.
			 *
			 * This value indicates how many cells are
			 * specified in the associated file.
			 */
			unsigned int num_cells;

			/**
			 * The bounding box of the stored data
			 *
			 * This bounding box includes x, y, and z
			 * dimensions.  The horizontal dimensions
			 * (x and y) indicate the bounds of the
			 * cells within this file, and the z-bounds
			 * indicate any level bounds.  
			 *
			 * The z-bounds may be unlimited, in which case
			 * this was the only level in the model.
			 *
			 * units:  meters.
			 */
			double x_min, y_min, z_min, x_max, y_max, z_max;

			/**
			 * The histogram resolution
			 *
			 * This value indicates the width of one cell
			 * in the grid represented in this file.  Each
			 * cell contains information about the vertical
			 * extent of its interior volume.  Each cell
			 * is a square.
			 *
			 * units: meters
			 */
			double resolution;

		/* functions */
		public:

			/*----------------*/
			/* Initialization */
			/*----------------*/

			/**
			 * Constructs default (invalid) header
			 */
			header_t() : 
				version_major(-1), version_minor(-1),
				level_index(-1), 
				num_cells(0),
				x_min(1), y_min(1), z_min(1),
				x_max(0), y_max(0), z_max(0),
				resolution(-1)
			{};

			/**
			 * Initializes valid header to specified values
			 *
			 * @param levind   The level index
			 * @param num      Number of cells
			 * @param xmin     The min x-coordinate
			 * @param ymin     The min y-coordinate
			 * @param zmin     The min z-coordinate
			 * @param xmax     The max x-coordinate
			 * @param ymax     The max y-coordinate
			 * @param zmax     The max z-coordinate
			 * @param res      The resolution
			 */
			void init(int levind, unsigned int num, 
				double xmin, double ymin, double zmin,
				double xmax, double ymax, double zmax,
				double res);

			/**
			 * Verifies that the information stored in this
			 * header is valid.
			 *
			 * @return   Returns true iff valid header info
			 */
			bool is_valid() const;

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Parses header information from the given
			 * input stream.
			 *
			 * Will treat the specified istream as the
			 * beginning of a .hia file and read the
			 * header information.
			 *
			 * @param is   The input stream to parse
			 *
			 * @return     Returns zero on success,
			 * 		non-zero on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Exports the contents of this header
			 * struture to the specified output stream.
			 *
			 * Will write the header to the output stream
			 * at its current position.  It is up to the
			 * caller to ensure the stream is at the beginning
			 * of the file.
			 *
			 * @param os   The output stream to write to
			 *
			 * @return     Returns zero on success, 
			 * 		non-zero on failure.
			 */
			int serialize(std::ostream& os) const;
	};

	/*------------------*/
	/* the cell_t class */
	/*------------------*/

	/**
	 * The cell_t class records the information about a single
	 * grid cell in the associated file.
	 */
	class cell_t
	{
		/* parameters */
		public:

			/**
			 * The center position of this cell
			 *
			 * units:  meters
			 */
			double center_x, center_y;

			/**
			 * The total range of elevations seen in
			 * this cell
			 *
			 * units:  meters
			 */
			double min_z, max_z;

			/**
			 * The amount of the elevation of this cell
			 * that is marked as "open" or "interior".
			 *
			 * units:  meters
			 */
			double open_height;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Generates a default (empty) cell
			 */
			cell_t() : 
				center_x(0), center_y(0), 
				min_z(1), max_z(0), open_height(-1)
			{};

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Parses the specified file stream and stores
			 * next cell in this structure.
			 *
			 * Assumes that the given input stream is positioned
			 * at the start of a cell_t in the file.
			 *
			 * Any information stored in this structure will
			 * be destroyed by this call.
			 *
			 * @param is  The input stream to parse
			 *
			 * @return    Returns zero on success, non-zero
			 * 		on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Writes this cell_t to the specified output stream
			 *
			 * @param os   The output stream to write to
			 *
			 * @return     Returns zero on success, non-zero
			 * 		on failure.
			 */
			int serialize(std::ostream& os) const;
	};

	/*--------------------*/
	/* the reader_t class */
	/*--------------------*/

	/**
	 * The reader_t class is used to parse .hia files
	 */
	class reader_t
	{
		/* parameters */
		private:

			/**
			 * The header parsed from the file
			 */
			header_t header;

			/**
			 * The input file stream to parse from
			 */
			std::ifstream infile;

		/* functions */
		public:

			/**
			 * Frees all memory and resources
			 */
			~reader_t()
			{ this->close(); };

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Opens a file with this reader
			 *
			 * If an error occurs, will print
			 * descriptive text to cerr.
			 *
			 * @param filename   The file to open
			 *
			 * @return    Returns zero on success,
			 * 		non-zero on failure.
			 */
			int open(const std::string& filename);
			
			/**
			 * Reads the next cell from the file
			 *
			 * Will parse the next cell from the file
			 * and store the resulting info in the referenced
			 * structure.
			 *
			 * @param cell   Where to store the cell info
			 *
			 * @return       Returns zero on success, non-zero
			 * 			on failure.
			 */
			int next(cell_t& cell);

			/**
			 * Closes any open file streams, frees all
			 * memory and resources.
			 */
			void close();
	
			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Returns the level index stored in this file
			 *
			 * @return   Open file's level index
			 */
			inline int level_index() const
			{ return this->header.level_index; };

			/**
			 * Returns the total number of cells in file.
			 *
			 * @return   Total number of cells
			 */
			inline unsigned int num_cells() const
			{ return this->header.num_cells; };

			/**
			 * Returns the min bound in the x-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the min-x bound
			 */
			inline double x_min() const
			{ return this->header.x_min; };

			/**
			 * Returns the min bound in the y-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the min-y bound
			 */
			inline double y_min() const
			{ return this->header.y_min; };

			/**
			 * Returns the min bound in the z-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the min-z bound
			 */
			inline double z_min() const
			{ return this->header.z_min; };

			/**
			 * Returns the max bound in the x-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the max-x bound
			 */
			inline double x_max() const
			{ return this->header.x_max; };

			/**
			 * Returns the max bound in the y-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the max-y bound
			 */
			inline double y_max() const
			{ return this->header.y_max; };

			/**
			 * Returns the max bound in the z-coordinate
			 *
			 * Units:  meters
			 *
			 * @return   Returns the max-z bound
			 */
			inline double z_max() const
			{ return this->header.z_max; };

			/**
			 * Returns the resolution of the cells stored
			 * in the open file.
			 *
			 * units: meters
			 *
			 * @return   Resolution stored in file.
			 */
			inline double resolution() const
			{ return this->header.resolution; };
	};

	/*--------------------*/
	/* the writer_t class */
	/*--------------------*/

	/**
	 * The writer_t class is used to generate a .hia file
	 */
	class writer_t
	{
		/* parameters */
		private:
			
			/**
			 * The header to write to file
			 */
			header_t header;

			/**
			 * The output file stream
			 *
			 * This is a binary file stream.
			 */
			std::ofstream outfile;

		/* functions */
		public:

			/**
			 * Frees all memory and resources
			 */
			~writer_t()
			{ this->close(); };

			/**
			 * Opens a new file to write to
			 *
			 * Will write to the specified .hia file
			 *
			 * @param filename   Where to write the file
			 * @param res    The resolution of this file
			 * @param level  The level index of this file
			 * @param minz   The minimum z-coordinate
			 * @param maxz   The maximum z-coordinate
			 *
			 * @return     Returns zero on success, non-zero
			 * 		on failure.
			 */
			int open(const std::string& filename, 
					double res, int level,
					double minz, double maxz);

			/**
			 * Writes the specified cell to the file
			 *
			 * @param cell   The cell to write to file
			 *
			 * @return       Returns zero on success,
			 * 			non-zero on failure.
			 */
			int write(const cell_t& cell);
		
			/**
			 * Closes the file stream
			 *
			 * This call will also rewrite the header to
			 * the file, in order to update the count of
			 * cells and bounding box.
			 */
			void close();
	};
}

#endif
