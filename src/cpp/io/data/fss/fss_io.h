#ifndef FSS_IO_H
#define FSS_IO_H

/**
 * @file fss_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to read and write .fss files,
 * which are used to house synchronized and calibrated scans from
 * a given range, depth, or time-of-flight scanner.
 *
 * Make sure to compile with -std=c++0x to support C++11 standard.
 */

#include <string>
#include <ios>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex> /* this needs to use g++ flag: -std=c++0x */

/**
 * Namespace for fss files.
 *
 * This namespace contains all structures pertaining to file i/o for
 * the .fss filetype, and associated structures for storing such
 * information.
 */
namespace fss
{
	/* the following classes are defined in this file */
	class header_t;
	class reader_t;
	class writer_t;
	class frame_t;
	class point_t;

	/* the following definitions are used for .fss file i/o */
	static const int         EARLIEST_SUPPORTED_VERSION = 1;
	static const int         LATEST_SUPPORTED_VERSION   = 3;
	static const std::string MAGIC_NUMBER               = "fss";
	static const std::string END_HEADER_STRING          = "end_header"; 

	/* the following are valid header tags in the .fss file */
	static const std::string HEADER_TAG_VERSION      = "version";
	static const std::string HEADER_TAG_FORMAT       = "format";
	static const std::string HEADER_TAG_SCANNER_NAME = "scanner_name";
	static const std::string HEADER_TAG_SCANNER_TYPE = "scanner_type";
	static const std::string HEADER_TAG_NUM_SCANS    = "num_scans";
	static const std::string HEADER_TAG_NUM_POINTS_PER_SCAN
	                                 = "num_points_per_scan";
	static const std::string HEADER_TAG_UNITS        = "units";
	static const std::string HEADER_TAG_ANGLE        = "angle";

	/* default values */

	/* the default angular spacing is the angular spacing (in radians)
	 * of a hokuyo scanner, which is expressed as (3*pi/2) / 1080 */
	static const double DEFAULT_ANGULAR_SPACING      = 0.0043633;

	/**
	 * This enum dictates the valid formats for data in .fss files
	 */
	enum FILE_FORMAT
	{
		/* space-separated ascii format */
		FORMAT_ASCII, 
			
		/* little-endian binary format */
		FORMAT_LITTLE_ENDIAN, 
			
		/* big-endian binary format */
		FORMAT_BIG_ENDIAN, 
			
		/* unknown file format */
		FORMAT_UNKNOWN 
	};

	/**
	 * Converts the file-format enum to a string
	 *
	 * @param format  The file-format to convert
	 *
	 * @return        Returns the string representation for this format
	 */
	inline std::string format_to_string(FILE_FORMAT format)
	{
		/* choose string based on format value */
		switch(format)
		{
			case FORMAT_ASCII:
				return "ascii";
			case FORMAT_LITTLE_ENDIAN:
				return "little_endian";
			case FORMAT_BIG_ENDIAN:
				return "big_endian";
			default:
			case FORMAT_UNKNOWN:
				return "unknown";
		}
	};

	/**
	 * Parses the given string as a file format type
	 *
	 * Given a string, will attempt to parse it as a file format
	 * specification, and return the corresponding enum value.  If
	 * the string does not represent a valid file format, then the
	 * value of FORMAT_UNKNOWN is returned.
	 *
	 * @param str    The string to parse as a file format
	 * 
	 * @return       Returns the parsed file format
	 */
	inline FILE_FORMAT string_to_format(const std::string& str)
	{
		/* check for each possible file format */
		if(!str.compare("ascii"))
			return FORMAT_ASCII;
		else if(!str.compare("little_endian"))
			return FORMAT_LITTLE_ENDIAN;
		else if(!str.compare("big_endian"))
			return FORMAT_BIG_ENDIAN;
		else
			return FORMAT_UNKNOWN; /* can't parse this string */
	};

	/**
	 * This enum dictates the valid units that can be used in .fss files
	 */
	enum SPATIAL_UNITS
	{
		/* metric */
		UNITS_MILLIMETERS,
		UNITS_CENTIMETERS,
		UNITS_METERS,
		UNITS_KILOMETERS,

		/* uscs imperial units */
		UNITS_INCHES,
		UNITS_FEET,
		UNITS_YARDS,
		UNITS_FURLONGS, /* because, hey, why not? */
		UNITS_MILES,

		/* others */
		UNITS_UNKNOWN
	};

	/**
	 * Will return the conversion between given set of units and meters
	 *
	 * Given a set of units, will return the conversion factor from
	 * meters to that set of units. So if:
	 *
	 * 	c = convert_units_from_meters(units)
	 *
	 * Then it will be the case that:
	 *
	 * 	x meters = (c*x) units
	 *
	 * @param units  The units to convert from
	 * 
	 * @return    Returns the conversion factor from meters.
	 */
	inline double convert_units_from_meters(SPATIAL_UNITS units)
	{
		/* return value based on input */
		switch(units)
		{
			/* metric (aka easy-mode) */
			case UNITS_MILLIMETERS:
				return 1000.0;
			case UNITS_CENTIMETERS:
				return 100.0;
			case UNITS_METERS:
				return 1.0;
			case UNITS_KILOMETERS:
				return 0.001;

			/* uscs */
			case UNITS_INCHES:
				return 39.3701;
			case UNITS_FEET:
				return 3.28084;
			case UNITS_YARDS:
				return 1.09361;
			case UNITS_FURLONGS:
				return 0.00497096954;
			case UNITS_MILES:
				return 0.000621371;
			
			/* unknown values */
			default:
				return -1.0; /* unknown conversion */
		}
	};

	/**
	 * Converts enum to string for units
	 *
	 * Given a SPATIAL_UNITS enum, will return the associated string
	 *
	 * @param units   The units to convert to string
	 *
	 * @return   The string representation of these units
	 */
	inline std::string units_to_string(SPATIAL_UNITS units)
	{
		/* return value based on input */
		switch(units)
		{
			case UNITS_MILLIMETERS:
				return "millimeters";
			case UNITS_CENTIMETERS:
				return "centimeters";
			case UNITS_METERS:
				return "meters";
			case UNITS_KILOMETERS:
				return "kilometers";
			case UNITS_INCHES:
				return "inches";
			case UNITS_FEET:
				return "feet";
			case UNITS_YARDS:
				return "yards";
			case UNITS_FURLONGS:
				return "furlongs";
			case UNITS_MILES:
				return "miles";
			
			/* unknown values */
			default:
				return "unknown";
		}
	};

	/**
	 * Parses a string as a spatial unit system
	 *
	 * Will attempt to parse the given string as a SPATIAL_UNITS
	 * enum.  If unable to parse UNITS_UNKNOWN will be returned.
	 *
	 * @param str   The string to parse
	 *
	 * @return      The unit enum denoted by the given string
	 */
	inline SPATIAL_UNITS string_to_units(const std::string& str)
	{
		/* check all possible units */
		if(!str.compare("millimeters"))
			return UNITS_MILLIMETERS;
		else if(!str.compare("centimeters"))
			return UNITS_CENTIMETERS;
		else if(!str.compare("meters"))
			return UNITS_METERS;
		else if(!str.compare("kilometers"))
			return UNITS_KILOMETERS;
		else if(!str.compare("inches"))
			return UNITS_INCHES;
		else if(!str.compare("feet"))
			return UNITS_FEET;
		else if(!str.compare("yards"))
			return UNITS_YARDS;
		else if(!str.compare("furlongs"))
			return UNITS_FURLONGS;
		else if(!str.compare("miles"))
			return UNITS_MILES;
		else
			return UNITS_UNKNOWN;
	};

	/**
	 * This represents the data stored in the header of a .fss file
	 */
	class header_t
	{
		/* friend classes */
		friend class reader_t;
		friend class writer_t;
		friend class frame_t;
		friend class point_t;

		/* parameters */
		private:

			/* the following are fields stored in the header */
			int version; /* version number of file */
			FILE_FORMAT format; /* how data stored in file */
			std::string scanner_name; /* source scanner name */
			std::string scanner_type; /* type of scanner */
			SPATIAL_UNITS units; /* units of point positions */
			size_t num_scans; /* number of frames */
			int num_points_per_scan; /* negative --> variable */
			double angle; /* angular spacing between points */

		/* functions */
		public:

			/**
			 * Initializes default header information
			 */
			header_t();

			/**
			 * Initializes header info given necessary fields.
			 *
			 * Will initialize the header to a specific sensor.
			 * The fields not referenced in this function will
			 * remain unchanged.
			 *
			 * @param name      The name of the sensor
			 * @param type      The type of the sensor
			 * @param num_s     Number of scan frames in file
			 * @param num_p     Number of points per scan frame
			 * @param u         The units of the points
			 * @param ang       The specified angular spacing
			 */
			void init(const std::string& name, 
			          const std::string& type, 
			          size_t num_s,
			          int num_p,
				  SPATIAL_UNITS u,
			          double ang=DEFAULT_ANGULAR_SPACING);

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
			 * as a fss-formatted header to the specified stream
			 * object.  This does not check the fail-bits of
			 * this stream.
			 *
			 * @param outfile   The stream to write to
			 */
			void print(std::ostream& outfile) const;
	};


	/**
	 * This represents a reader object for .fss files
	 */
	class reader_t
	{
		/* parameters */
		private:
			
			/* the input file stream being read from */
			std::ifstream infile;

			/* the header information from this file */
			header_t header;

			/* a list of stream positions that represent
			 * the start of each frame in the file.  This
			 * is populated when the file is first read,
			 * and is useful for random-access of frames. */
			std::vector<std::streampos> frame_positions;

			/* The list of timestamps for each frame is also
			 * stored, so that frames can be retrieved by
			 * temporal location as well. */
			std::vector<double> frame_timestamps;

			/* the following parameters indicate how to
			 * parse the data points */
			bool auto_correct_for_bias;/* will subtract bias */
			bool auto_convert_to_meters;/* converts to meters */

			/* this mutex is locked when the functions get()
			 * or get_nearest() are called, which allows these
			 * functions to be threadsafe. */
			std::mutex mtx;

		/* functions */
		public:

			/**
			 * Initializes empty reader object for .fss files
			 */
			reader_t();

			/**
			 * Frees all memory and resources
			 */
			~reader_t();

			/* settings */

			/**
			 * Will set if bias is auto-correct for
			 *
			 * When reading points, by default, each point
			 * has a bias value, which is not subtracted
			 * from the original point.  If this is called
			 * with a value of 'true', then this subtraction
			 * is performed before any points are given
			 * to the caller from get() functions.
			 *
			 * @param cfb  Flag to correct for bias or not
			 */
			inline void set_correct_for_bias(bool cfb)
			{ this->auto_correct_for_bias = cfb; };

			/**
			 * Will set if the points are returned in meters
			 *
			 * By default, points are automatically converted
			 * to units of meters, regardless of what units
			 * the are stored in the file.  If this parameter
			 * is set to false, then the points will be returned
			 * in their native units.
			 *
			 * @param ctm   Flag to convert to meters or not
			 */
			inline void set_convert_to_meters(bool ctm)
			{ this->auto_convert_to_meters = ctm; };
	
			/* i/o */

			/**
			 * Closes the file, frees all memory and resources
			 */
			void close();

			/**
			 * Opens and parses a .fss file
			 *
			 * Given the path to a .fss file, will attempt
			 * to open and read this file.  On success, the
			 * metainformation will be read in, and frames
			 * will be able to be retrieved in random-access
			 * order.
			 *
			 * @param filename  The path to the file to import
			 *
			 * @return   Returns zero on success, 
			 *           non-zero on failure.
			 */
			int open(const std::string& filename);

			/* accessors */

			/**
			 * Returns the number of frames in the opened file
			 */
			size_t num_frames() const;

			/**
			 * Returns scanner name of source of these scans
			 */
			std::string scanner_name() const;

			/**
			 * Returns the units of the retrieved points
			 */
			SPATIAL_UNITS units() const;

			/**
			 * Returns the angular spacing of retrieved points
			 */
			double angle() const;

			/**
			 * Retrieves the i'th frame
			 *
			 * Will populate the given structure with the data
			 * from the i'th frame from the opened file.  This
			 * call will fail if i is out of bounds or if no
			 * file has been opened.
			 *
			 * This function is thread-safe!
			 *
			 * @param frame   The frame object to populate
			 * @param i       The frame index to read from
			 *
			 * @return        Returns zero on success,
			 *                non-zero on failure.
			 */
			int get(frame_t& frame, unsigned int i);
			
			/**
			 * Retrieves the closest frame to given timestamp
			 *
			 * Given a timestamp value, will populate the
			 * given structure with the frame information that
			 * is closest in time.
			 *
			 * This function is thread-safe!
			 *
			 * @param frame  The frame object to populate
			 * @param ts     The timestamp to compare to
			 *
			 * @return       Returns zero on success, non-zero
			 *               on failure.
			 */
			int get_nearest(frame_t& frame, double ts);
	};
	
	/**
	 * This represents a writer object for .fss files
	 */
	class writer_t
	{
		/* paramters */
		private:

			/* the output file stream being written to */
			std::ofstream outfile;

			/* the header information for this file */
			header_t header;

			/* progress in writing */
			size_t points_written_so_far;

		/* functions */
		public:

			/* constructors */

			/**
			 * Initializes empty writer
			 */
			writer_t();

			/**
			 * Frees all memory and resources
			 */
			~writer_t();

			/* i/o */

			/**
			 * Initializes the header information for file
			 *
			 * Stores the information for how to format
			 * the file.  This function should be called
			 * BEFORE the file is opened.
			 *
			 * @param name   Name of scanner being stored
			 * @param type   Type of scanner being stored
			 * @param num_s  Number of scan frames to store
			 * @param num_p  Number of points per scan frame.
			 *               If set to negative, then each scan
			 *               frame will have a variable number
			 *               of points and will be defined at
			 *               start of frame.
			 * @param u      The units of the points
			 * @param ang    The specified angular spacing
			 */
			void init(const std::string& name,
			          const std::string& type,
			          size_t num_s, int num_p,
			          SPATIAL_UNITS u=UNITS_METERS,
			          double ang=DEFAULT_ANGULAR_SPACING);

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
			 * Writes the given frame to file
			 *
			 * Will write this scan frame to the file.  This
			 * frame must be consistent with the number of
			 * points per scan frame given when init() was
			 * called.  If this function is called more
			 * times than the num_s value specified, then
			 * an error will be thrown.
			 *
			 * @param frame   The scan frame to write to file.
			 *
			 * @return        Returns zero on success,
			 *                non-zero on fialure.
			 */
			int write(const frame_t& frame);

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
	 * Represents data stored in one frame of the body of a .fss file
	 */
	class frame_t
	{
		/* parameters */
		public:

			/* the following parameters denote one scan frame */
			double timestamp; /* in synchronized system clock */
			std::vector<point_t> points; /* points in scan */

		/* functions */
		public:
		
			/**
			 * Initializes empty frame
			 */
			frame_t();

			/**
			 * Reads frame from input stream
			 *
			 * Given an input stream and header information
			 * about the stream, will read the next available
			 * frame.
			 *
			 * @param infile   The input stream to read from
			 * @param header   Header information about stream
			 *
			 * @return    Returns zero on success,
			 *            non-zero on failure.
			 */
			int parse(std::istream& infile,
			          const header_t& header);

			/**
			 * Writes frame to output stream
			 *
			 * Will write this frame's information to the
			 * given output stream, given the header information
			 * about that stream.
			 *
			 * @param outfile  The output stream to write to
			 * @param header   Header information about stream
			 *
			 * @return   Returns zero on success, non-zero on
			 *           failure.
			 */
			int print(std::ostream& outfile,
			          const header_t& header) const;
	};

	/**
	 * Represents a single point object from the body of a .fss file
	 */
	class point_t
	{
		/* parameters */
		public:
			
			/* these are the values as seen in the file */
			double x;      /* spatial x-coordinate */
			double y;      /* spatial y-coordinate */
			double z;      /* spatial z-coordinate */
			int intensity; /* greyscale color */
			double bias;   /* bias of range measurement */
			double stddev; /* noise of range measurement */
			double width;  /* noise of lateral measurement */

		/* functions */
		public:

			/**
			 * Creates default point
			 */
			point_t();

			/**
			 * Will read this point from input stream
			 *
			 * Given header information about the input stream,
			 * will read in the next point.
			 *
			 * @param infile   The stream to read from
			 * @param header   Header information about source
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.
			 */
			int parse(std::istream& infile,
			          const header_t& header);

			/**
			 * Will write the point information to given stream
			 *
			 * Given a stream and the header information about
			 * how the stream is formatted, will write the
			 * information stored in this point structure to
			 * the stream.
			 *
			 * @param outfile  Where to write the point to
			 * @param header   The header information about dest
			 *
			 * @return    Returns zero on success, non-zero on 
			 *            failure.
			 */
			int print(std::ostream& outfile, 
			          const header_t& header) const;
	
			/* modifiers */

			/**
			 * Corrects the point's spatial position by bias
			 *
			 * Will subtract the bias of the range measurement
			 * from the point's spatial position.
			 */
			void correct_for_bias();

			/**
			 * Scales the position of the point by given value
			 *
			 * @param s   The value to scale the position by
			 */
			void scale(double s);
	};	
}

#endif
