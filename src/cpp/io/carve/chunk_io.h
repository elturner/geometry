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
	class chunk_header_t;
	class chunk_reader_t;
	class chunk_writer_t;
	class point_index_t;

	/* the following definitions are used for .chunklist file i/o */
	static const std::string CHUNKLIST_MAGIC_NUMBER      = "chunklist";
	static const std::string CHUNKFILE_MAGIC_NUMBER      = "chunkfile";
	static const size_t      CHUNKFILE_MAGIC_NUMBER_SIZE =
	                        (CHUNKFILE_MAGIC_NUMBER.size()+1);
	static const std::string END_HEADER_STRING           = "end_header";

	/* the following are valid header tags in the .chunklist file */
	static const std::string HEADER_TAG_CENTER        = "center";
	static const std::string HEADER_TAG_HALFWIDTH     = "halfwidth";
	static const std::string HEADER_TAG_NUM_CHUNKS    = "num_chunks";
	static const std::string HEADER_TAG_CHUNK_DIR     = "chunk_dir";

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

			/* directory that contains input file */
			std::string directory; 

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
			 * Returns the X-coordinate of center position
			 *
			 * @return    Returns center position x-coord
			 */
			inline double center_x() const
			{ return this->header.center_x; };

			/**
			 * Returns the Y-coordinate of center position
			 *
			 * @return    Returns center position y-coord
			 */
			inline double center_y() const
			{ return this->header.center_y; };

			/**
			 * Returns the Z-coordinate of center position
			 *
			 * @return    Returns center position z-coord
			 */
			inline double center_z() const
			{ return this->header.center_z; };
			
			/**
			 * Returns the halfwidth recovered from file
			 *
			 * @return   Returns the specified halfwidth
			 */
			inline double halfwidth() const
			{ return this->header.halfwidth; };

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

	/**
	 * The header information for .chunk files
	 */
	class chunk_header_t
	{
		/* security */
		friend class chunk_reader_t;
		friend class chunk_writer_t;

		/* parameters */
		private:

			/* the following fields are part of the
			 * header for .chunk files */
			unsigned long long uuid; /* universally unique id */
			double center_x; /* x-position of chunk center */
			double center_y; /* y-position of chunk center */
			double center_z; /* z-position of chunk center */
			double halfwidth; /* halfwidth of chunk volume */
			unsigned int num_points; /* number of scanpoints */

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initializes default (invalid) parameters 
			 */
			chunk_header_t();

			/**
			 * Initializes values of header.
			 *
			 * Will initialze this header structure to
			 * the given values
			 *
			 * @param u    The uuid of this chunk
			 * @param cx   The x-pos of chunk center
			 * @param cy   The y-pos of chunk center
			 * @param cz   The z-pos of chunk center
			 * @param hw   The halfwidth of chunk
			 * @parma np   The number of intersecting scanpoints
			 */
			void init(unsigned long long u,
			          double cx, double cy, double cz,
			          double hw);

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Parses the given stream as a chunk header
			 *
			 * Will parse binary header information from
			 * the specified input stream.
			 *
			 * @param is   The stream to parse
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Prints this header info to the given stream
			 *
			 * Will print the info specified in this header
			 * object to the given output binary stream.
			 *
			 * @param os   The stream to write to
			 */
			void print(std::ostream& os) const;

			/**
			 * Prints the number of written scanpoints to header
			 *
			 * Given a stream object to an open binary .chunk
			 * file for writing, will seek to the
			 * head of the stream and write the specified
			 * number of intersected scanpoints to the file.
			 *
			 * After this call, the stream will be positioned
			 * at the end of the header.
			 *
			 * @param os   The binary stream to write to
			 * @param np   The number of points to write
			 */
			void write_num_points(std::ostream& os,
			                      unsigned int np);
	};

	/**
	 * This parses binary .chunk files from disk.
	 */
	class chunk_reader_t
	{
		/* parameters */
		private:
		
			/* the file input stream */
			std::ifstream infile;

			/* header information from the file */
			chunk_header_t header;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initializes empty reader
			 */
			chunk_reader_t();

			/**
			 * Frees all memory and resources
			 */
			~chunk_reader_t();
		
			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Returns the number of point indices in file
			 *
			 * @return   The number of elements in the file
			 */
			inline unsigned int num_points() const
			{ return this->header.num_points; };
			
			/**
			 * Returns the X-coordinate of center position
			 *
			 * @return    Returns center position x-coord
			 */
			inline double center_x() const
			{ return this->header.center_x; };

			/**
			 * Returns the Y-coordinate of center position
			 *
			 * @return    Returns center position y-coord
			 */
			inline double center_y() const
			{ return this->header.center_y; };

			/**
			 * Returns the Z-coordinate of center position
			 *
			 * @return    Returns center position z-coord
			 */
			inline double center_z() const
			{ return this->header.center_z; };
			
			/**
			 * Returns the halfwidth recovered from file
			 *
			 * @return   Returns the specified halfwidth
			 */
			inline double halfwidth() const
			{ return this->header.halfwidth; };

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Opens .chunk file for reading
			 *
			 * Will attempt to open and parse the given
			 * .chunk file for reading.  On success, the
			 * header will be parsed from the file.
			 *
			 * @param filename   The path to the file to open
			 *
			 * @return    Returns zero on success, non-zero
			 *            on error.
			 */
			int open(const std::string& filename);

			/**
			 * Retrieves the next index set from the file
			 *
			 * Will retrieve the next element from the
			 * body of the file.  Should only be called
			 * after a successful call to open().
			 *
			 * @param i   Where to store index info
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int next(point_index_t& i);
			
			/**
			 * Closes the reader, freeing resources.
			 *
			 * Will close this reader if a file is open.
			 */
			void close();
	};

	/**
	 * This writes binary .chunk files to disk.
	 */
	class chunk_writer_t
	{
		/* parameters */
		private:

			/* the output stream to write to */
			std::ofstream outfile;

			/* the header information to use */
			chunk_header_t header;

			/* the number of points written so far */
			unsigned int num_written_so_far;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initialzes empty writer
			 */
			chunk_writer_t();

			/**
			 * Frees all memory and resources
			 */
			~chunk_writer_t();

			/**
			 * Initializes writer with specified values
			 *
			 * @param uuid   The uuid for this chunk
			 * @param cx     The x-coordinate of chunk center
			 * @param cy     The y-coordinate of chunk center
			 * @param cz     The z-coordinate of chunk center
			 * @param hw     The halfwidth of chunk volume
			 */
			void init(unsigned long long uuid,
			          double cx, double cy, double cz,
			          double hw);

			/*-----*/ 
			/* i/o */
			/*-----*/

			/**
			 * Opens a .chunk file for writing
			 *
			 * On success of this call, the file will be
			 * opened, and a header will be written.  This
			 * header will specify zero points, but will
			 * be overwritten when the file is closed to
			 * reflect the accurate value.
			 *
			 * This function should be called after init().
			 *
			 * @param filename   The file to write to
			 *
			 * @return      Returns zero on success, non-zero
			 *              on failure.
			 */
			int open(const std::string& filename);

			/**
			 * Writes this point's info to the file body
			 *
			 * This function can only be called on an open
			 * file.  It will write the given point's info
			 * to disk.
			 *
			 * @param i    The point index to write
			 */
			void write(const point_index_t& i);

			/**
			 * Closes this file and frees resources
			 *
			 * Will close an open file stream.  Before
			 * closing, this function will update the file
			 * to how many point indices were written.
			 */
			void close();
	};
	
	/**
	 * This class represents the global indices of a single scan point
	 */
	class point_index_t
	{
		/* parameters */
		public:

			/**
			 * Denotes the global index of a scan wedge
			 *
			 * This index references a wedge's position
			 * in a .wedge file, which should contain
			 * all carve wedges that are used for geometry
			 * processing, making this index globally unique
			 */
			size_t wedge_index;
		
		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs default index (0,0,0)
			 */
			point_index_t();

			/**
			 * Constructs index based on input value
			 *
			 * @param wi   The wedge index
			 */
			point_index_t(size_t wi);
	
			/**
			 * Sets this object to the specified value
			 *
			 * @param wi   The wedge index
			 */
			inline void set(size_t wi)
			{
				/* store the value */
				this->wedge_index = wi;
			};

			/*-----*/
			/* i/o */
			/*-----*/
	
			/**
			 * Parses a single point index from .chunk file
			 *
			 * Will parse the open stream as a binary .chunk
			 * file, and read in a single point, storing
			 * the result in this structure.
			 *
			 * @param is   The input file stream to parse
			 */
			void parse(std::istream& is);

			/**
			 * Prints a single point index to a .chunk stream
			 *
			 * Will print the values contained within this
			 * point_index_t object to the specified output
			 * binary stream, formatted as a .chunk file.
			 *
			 * @param os   The output file to write to
			 */
			void print(std::ostream& os) const;
	
			/*------------*/
			/* operataors */
			/*------------*/
	
			/**
			 * Check if two point indices are equal
			 */
			inline bool operator == (
					const point_index_t& other) const
			{
				return (this->wedge_index 
						== other.wedge_index);
			};

			/**
			 * Sets value of point
			 */
			inline point_index_t& operator = (
					const point_index_t& other)
			{
				/* set each parameter */
				this->wedge_index = other.wedge_index;

				/* return the result */
				return (*this);
			};

			/**
			 * Used for ordering objects in maps or sets
			 */
			inline bool operator < (
					const point_index_t& other) const
			{
				/* check each parameter in order */
				return (this->wedge_index 
						< other.wedge_index);
			};
	};
}

#endif
