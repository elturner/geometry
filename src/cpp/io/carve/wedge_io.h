#ifndef WEDGE_IO_H
#define WEDGE_IO_H

/**
 * @file wedge_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Provides i/o classes for carve_wedge_t lists
 *
 * @section DESCRIPTION
 *
 * This file contains classes that are used to import and export
 * carve_wedge_t objects from file.
 *
 * Make sure to compile with -std=c++0x to support C++11 standard.
 */

#include <geometry/shapes/carve_wedge.h>
#include <fstream>
#include <string>
#include <iostream>
#include <mutex> /* this needs to use g++ flag: -std=c++0x */

/**
 * The wedge namespace contains classes and members used for i/o operations
 * for carve_wedge_t objects.
 */
namespace wedge
{
	/* the following classes are defined in this namespace */
	class header_t;
	class reader_t;
	class writer_t;

	/* the following values are used in this namespace */

	/**
	 * This value defines the magic number at the beginning of
	 * a binary wedge list file.
	 */
	static const std::string MAGIC_NUMBER = "wedge";

	/**
	 * This value indicates the number of bytes used by the magic
	 * number in the binary wedge list file.
	 */
	static const size_t MAGIC_NUMBER_SIZE = (1+MAGIC_NUMBER.size());

	/**
	 * The total size of the header of a wedge file
	 * 
	 * The header consists of the magic number and an 8-byte
	 * integer used to represent the number of wedges in file.
	 */
	static const size_t HEADER_SIZE = (MAGIC_NUMBER_SIZE + 8);

	/**
	 * The size of a vertex stored in the file
	 *
	 * A vertex is three 8-byte doubles, for a total of 24 bytes.
	 */
	static const size_t VERTEX_SIZE = (3*sizeof(double));
	
	/**
	 * The size of a covariance matrix in file
	 *
	 * A covariance matrix is represented as a 3x3 matrix in row-major
	 * order, and is used to represent the uncertainty component
	 * of a probability distribution.
	 */
	static const size_t COV_MAT_SIZE = (9*sizeof(double));

	/**
	 * The size of a multi-variate gaussian distribution in file
	 *
	 * A gaussian distribution is represented by a mean location
	 * (a vertex) and a covariance matrix.
	 */
	static const size_t GAUSS_DIST_SIZE = (VERTEX_SIZE + COV_MAT_SIZE);

	/**
	 * The size of a carve-map stored in file
	 *
	 * A carve map is represented by two multivariate gaussian 
	 * distributions
	 */
	static const size_t CARVE_MAP_SIZE = (2*GAUSS_DIST_SIZE);

	/**
	 * The size of a wedge as stored in the file
	 *
	 * A wedge is represented by 6 vertices and 4 carve maps
	 */
	static const size_t WEDGE_SIZE = (6*VERTEX_SIZE + 4*CARVE_MAP_SIZE);

	/**
	 * The header_t class is used to represent the header of a 
	 * wedge list file.
	 */
	class header_t
	{
		/* security */
		friend class reader_t;
		friend class writer_t;

		/* parameters */
		private:

			/* the number of wedges defined in this file */
			size_t num_wedges;

		/* functions */
		public:

			/**
			 * Initializes empty header
			 */
			header_t();

			/**
			 * Parses the header info from given file stream
			 *
			 * @param is   The input file stream to parse
			 *
			 * @return     Returns zero on success,
			 *             non-zero on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Prints the header info to the given stream
			 *
			 * @param os   The output stream to print to
			 */
			void print(std::ostream& os) const;
	};

	/**
	 * The reader_t class will parse a given wedge list file
	 */
	class reader_t
	{
		/* parameters */
		private:
		
			/* the header info for this file */
			header_t header;

			/* the stream for this file */
			std::ifstream infile;
			
			/* This mutex will lock each time the get() function
			 * is called, to allow this reader to be threadsafe
			 */
			std::mutex mtx;

		/* functions */
		public:

			/**
			 * Frees all memory and resources 
			 */
			~reader_t();

			/**
			 * Opens the given file for reading 
			 *
			 * Will attempt to open the file, parse the
			 * header information.
			 *
			 * @param filename     The path to the file to open
			 *
			 * @return     Returns zero on success, non-zero on
			 *             failure.
			 */
			int open(const std::string& filename);

			/**
			 * Retrieves the i'th wedge from this file
			 *
			 * Will read the information for the i'th wedge
			 * specified in this file.
			 *
			 * @param w   Where to store the wedge information
			 * @param i   The index of the wedge to retrive
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.
			 */
			int get(carve_wedge_t& w, unsigned int i);
	
			/**
			 * Retrieves the number of wedges in this file
			 *
			 * @return  Returns the number of wedges in file
			 */
			inline size_t num_wedges() const
			{ return header.num_wedges; };

			/**
			 * Closes this file if it is open
			 *
			 * Closes all file handles.
			 */
			void close();
	};

	/**
	 * The writer_t class is used to export wedge files to disk
	 */
	class writer_t
	{
		/* parameters */
		private:

			/* the header of this file */
			header_t header;

			/* the file stream to write to */
			std::ofstream outfile;
		
		/* functions */
		public:

			/**
			 * Frees all memory and resources
			 */
			~writer_t();

			/**
			 * Will open the file for writing
			 *
			 * @param filename   Where to write the file to disk
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.
			 */
			int open(const std::string& filename);

			/**
			 * Writes a wedge to the file
			 *
			 * Will export this wedge's information to the
			 * opened file stream.
			 *
			 * @param w   The wedge to export
			 */
			void write(const carve_wedge_t& w);

			/**
			 * Closes this file, if it is open
			 */
			void close();
	};
}

#endif
