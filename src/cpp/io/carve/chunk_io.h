#ifndef CHUNK_IO_H
#define CHUNK_IO_H

/**
 * @file chunk_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to read and write .chunk files,
 * which are used to define which scan points intersect which subsets
 * of the scan volume.
 */

#include <string>
#include <ios>
#include <iostream>
#include <fstream>
#include <vector>

/**
 * Namespace for chunk files.
 *
 * This namespace contains all structures pertaining to file i/o for
 * the .chunk filetype, and associated structures for storing such
 * information.
 */
namespace chunk
{
	/* the following classes are defined in this file */
	class chunklist_header_t;
	class chunklist_reader_t;
	class chunklist_writer_t;
	class chunk_reader_t;
	class chunk_writer_t;

	/* the following definitions are used for .chunklist file i/o */
	static const std::string CHUNKLIST_MAGIC_NUMBER     = "chunklist";
	static const std::string CHUNKFILE_MAGIC_NUMBER     = "chunkfile";
	static const std::string END_HEADER_STRING          = "end_header"; 

	/* the following are valid header tags in the .chunklist file */
	static const std::string HEADER_TAG_CENTER        = "center";
	static const std::string HEADER_TAG_HALFWIDTH     = "halfwidth";
	static const std::string HEADER_TAG_NUM_CHUNKS    = "num_chunks";
	static const std::string HEADER_TAG_CHUNK_DIR     = "chunk_dir";
	static const std::string HEADER_TAG_SENSOR        = "sensor";

	/* the following are file extensions used by these classes */
	static const std::string CHUNKFILE_EXTENSION      = ".chunk";
	static const char FILE_SEPERATOR                  = '/';

	/**
	 * This represents the data in the header of a .chunklist file
	 */
	class chunklist_header_t
	{
		/* friend classes */
		friend class chunklist_reader_t;
		friend class chunklist_writer_t;

		/* parameters */
		private:

			/* the volume geometry (units: meters) */
			double center_x,center_y,center_z; /* center pos */
			double halfwidth; /* the halfwidth of root */

			/* chunk info */
			size_t num_chunks; /* number of chunks */
			std::string chunk_dir; /* location of chunk files,
			                        * includes '/' at end */

			/* sensor information */
			std::vector<std::string> sensor_names;

		/* functions */
		public:

			/**
			 * Initializes default header information
			 */
			chunklist_header_t();

			/**
			 * Initializes header info given necessary fields.
			 *
			 * Will initialize the header.  The sensor names
			 * will be cleared after this call.
			 *
			 * @param cx   The x-coordinate of root center
			 * @param cy   The y-coordinate of root center
			 * @param cz   The z-coordinate of root center
			 * @param hw   The halfwidth of root
			 * @param cd   The chunk directory
			 * @param nc   The number of chunks to write
			 */
			void init(double cx, double cy, double cz, 
			          double hw, const std::string& cd, 
			          size_t nc);

			/**
			 * Adds a sensor name to this header.
			 *
			 * Names will be stored in the order they are
			 * added.
			 *
			 * @param name   The name to add
			 */
			inline void add_sensor(const std::string& name)
			{ sensor_names.push_back(name); };

			/**
			 * Parses the header from the given file stream
			 *
			 * Will attempt to parse the header information
			 * from the specified stream.  Will only return
			 * success if header successfully parsed and the
			 * version of the file is supported.  The parsed
			 * information will be stored in this object.
			 *
			 * The given stream should already be open.
			 *
			 * @param infile   The stream to parse
			 *
			 * @return  Returns zero on success, 
			 *          non-zero on failure.
			 */
			int parse(std::istream& infile);
	
			/**
			 * Writes this header information to given stream
			 *
			 * Will export the information stored in this object
			 * as a chunk-formatted header to the specified
			 * stream object.  This does not check the 
			 * fail-bits of this stream.
			 *
			 * @param outfile   The stream to write to
			 */
			void print(std::ostream& outfile) const;
	};


	/**
	 * This represents a reader object for .chunklist files
	 */
	class chunklist_reader_t
	{
		/* parameters */
		private:
			
			/* the input file stream being read from */
			std::ifstream infile;

			/* the header information from this file */
			chunklist_header_t header;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initializes empty reader object for .chunk files
			 */
			chunklist_reader_t();

			/**
			 * Frees all memory and resources
			 */
			~chunklist_reader_t();

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Closes the file, frees all memory and resources
			 */
			void close();

			/**
			 * Opens and parses a .chunklist file
			 *
			 * Given the path to a .chunklist file, will attempt
			 * to open and read this file.  On success, the
			 * header will be read in, and list elements
			 * will be able to be retrieved by calling next().
			 *
			 * @param filename  The path to the file to import
			 *
			 * @return   Returns zero on success, 
			 *           non-zero on failure.
			 */
			int open(const std::string& filename);

			/* accessors */

			/**
			 * Returns the number of chunks in the opened file
			 */
			size_t num_chunks() const;

			/**
			 * Retrieves the next chunk file
			 *
			 * Will parse the file for the next chunk uuid,
			 * and store the corresponding .chunk file in
			 * the specified string.  This file path will
			 * be relative to the currently parsed .chunklist
			 * file.
			 *
			 * @param file    The next parsed chunk file path
			 *
			 * @return        Returns zero on success,
			 *                non-zero on failure.
			 */
			int next(std::string& file);
	};
	
	/**
	 * This represents a writer object for .chunklist files
	 */
	class chunklist_writer_t
	{
		/* paramters */
		private:

			/* the output file stream being written to */
			std::ofstream outfile;

			/* the header information for this file */
			chunklist_header_t header;

			/* progress in writing */
			size_t chunks_written_so_far;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initializes empty writer
			 */
			chunklist_writer_t();

			/**
			 * Frees all memory and resources
			 */
			~chunklist_writer_t();

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Initializes the header information for file
			 *
			 * Stores the information for how to format
			 * the file.  This function should be called
			 * BEFORE the file is opened.
			 *
			 * @param cx   The x-coordinate of root center
			 * @param cy   The y-coordinate of root center
			 * @param cz   The z-coordinate of root center
			 * @param hw   The halfwidth of root
			 * @param cd   The chunk directory
			 * @param nc   The number of chunks to write
			 */
			void init(double cx, double cy, double cz, 
			          double hw, const std::string& cd, 
			          size_t nc);

			/**
			 * Adds a sensor to write out to file
			 *
			 * This function should be called before the file
			 * is opened.  It will add a sensor name to be
			 * written to the file header.  Sensors are indexed
			 * in the order in which they are added.
			 *
			 * @param name   The name of the sensor to add
			 */
			inline void add_sensor(const std::string& name)
			{ this->header.add_sensor(name); };

			/**
			 * Opens file for writing
			 *
			 * Will attempt to open the file.  If file is
			 * successfully opened, then will write header
			 * to file.  This call will fail if header
			 * has not been initialized.
			 *
			 * @param filename    The path to file to write to
			 *
			 * @return   Returns zero on success,
			 *           non-zero on failure.
			 */
			int open(const std::string& filename);

			/**
			 * Writes the given chunk uuid to file
			 *
			 * @param frame   The uuid to write to file
			 */
			void write(const std::string& uuid);

			/**
			 * Closes the writer
			 *
			 * Will close the writer if it has been opened.
			 * If the writer has not been opened, this is
			 * a no-op.  This function is called automatically
			 * on deconstruction.
			 */
			void close();
	};

	// TODO left off here
}

#endif
