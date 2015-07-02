#ifndef TANGO_IO_H
#define TANGO_IO_H

/**
 * @file   tango_io.h
 * @author Eric Turner <elturner@indoorreality.com>
 * @brief  Parses tango .dat files
 *
 * @section DESCRIPTION
 *
 * The tango_io namespace contains classes used to parse and
 * represent the data products stored in the .dat files generated
 * from the Google Tango data collection application.
 */

#include <fstream>
#include <istream>
#include <string>
#include <vector>

/**
 * The tango_io namespace contains classes used to represent tango data
 */
namespace tango_io
{
	/* the following classes are in this namespace */
	class tango_reader_t;
	class tango_frame_t;
	class tango_point_t;

	/**
	 * The tango_reader_t class used to parse .dat files from the tango
	 */
	class tango_reader_t
	{
		/* parameters */
		private:

			/**
			 * The input file stream that represents the data
			 */
			std::ifstream infile;

			/**
			 * The current frame index
			 *
			 * This value will be incremented after a frame
			 * is read.
			 */
			size_t current_index;

			/**
			 * The starting position of each frame in the
			 * input stream.
			 *
			 * This is useful for random access of scans.
			 */
			std::vector<std::streampos> frame_locs;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Default constructor represents unopened file
			 */
			tango_reader_t();

			/**
			 * Constructs reader by attempting to open file
			 *
			 * @param filename   The tango .dat file to open
			 */
			tango_reader_t(const std::string& filename);

			/**
			 * Frees all memory and resources
			 */
			~tango_reader_t();

			/*----------------*/
			/* I/O Operations */
			/*----------------*/

			/**
			 * Attempt to open the specified file
			 *
			 * @param filename   The .dat file to open
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int open(const std::string& filename);

			/**
			 * Will return true iff a file is open
			 *
			 * @return   Returns whether file currently open
			 */
			inline bool is_open() const
			{ return this->infile.is_open(); };

			/**
			 * Retrieves the frame at the specified index
			 *
			 * Given an index, will populate the specified
			 * frame structure with the information of that
			 * frame from the file.
			 *
			 * @param i   The index to read
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int get(size_t i, tango_frame_t& frame);

			/**
			 * Returns the total number of frames found in
			 * the current file.
			 *
			 * For an open file, will return the total number
			 * of frames.  If no file is open, then returns
			 * zero.
			 *
			 * @return    Returns total number of frames.
			 */
			inline size_t num_frames() const
			{ return this->frame_locs.size(); };

			/**
			 * Retrieves the next frame of tango data.
			 *
			 * The parsed data are stored in the specified
			 * structure.
			 *
			 * @param frame   Where to store the next frame of
			 *                data from the tango file
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.  Will return non-zero if end-
			 *            of-file reached, or if no file open.
			 */
			int next(tango_frame_t& frame);

			/**
			 * Returns true if the end of file is reached
			 *
			 * Will also return true if no file is open.
			 *
			 * @return   Returns true if reached end of file
			 */
			inline bool eof() const
			{ 
				return (!(this->is_open()) 
						|| this->infile.eof());
			};

			/**
			 * Closes file if open
			 *
			 * If no file is open, this function call is
			 * a no-op.
			 */
			void close();
	
		/* helper functions */
		private:
		
			/**
			 * Reads a single char from input stream
			 *
			 * This function is necessary because the
			 * input file is formatted using Java's
			 * DataInput specification, which expands
			 * some characters into 1, 2, or 3 bytes.
			 *
			 * @return   Returns the next character
			 */
			char read_char();

			/**
			 * Reads a single integer from input stream
			 *
			 * This function is necessary because the
			 * input file is formatted using Java's
			 * DataInput specification.  An integer
			 * is represented by four input characters
			 * in big-endian ordering.
			 *
			 * @return   Returns the next integer.
			 */
			int read_int();

			/**
			 * Reads a single double from input stream
			 *
			 * This function is necessary because the
			 * input file is formatted using Java's
			 * DataInput specification.  An integer
			 * is represented by eight input characters
			 * in big-endian ordering.
			 *
			 * @return   Returns the next double
			 */
			double read_double();
	};

	/**
	 * The tango_frame_t class represents a single frame of tango data
	 */
	class tango_frame_t
	{
		/* parameters */
		public:

			/**
			 * The index of this frame
			 */
			size_t index;

			/**
			 * The timestamp of this frame
			 */
			double timestamp;

			/**
			 * The position of the tango sensor at the
			 * time of this frame.
			 *
			 * Expressed as a length-3 array:  x,y,z
			 */
			double position[3];

			/**
			 * The orientation of the tango sensor at the time
			 * of this frame, represented as a quaternion.
			 *
			 * Expressed as a length-4 array: qx,qy,qz,qw
			 */
			double quaternion[4];

			/**
			 * The set of points captured during this frame
			 *
			 * These are in the depth-sensor reference frame.
			 */
			std::vector<tango_point_t> points;
	};

	/**
	 * This class represents a single point in a tango depth frame
	 */
	class tango_point_t
	{
		/* parameters */
		public:

			/**
			 * The position of this point.
			 */
			float x;
			float y;
			float z;
	};
}

#endif
