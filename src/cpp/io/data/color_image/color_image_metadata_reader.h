#ifndef COLOR_IMAGE_METADATA_READER_H
#define COLOR_IMAGE_METADATA_READER_H

/**
 * @file color_image_metadata_reader.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains definitions for classes used to read
 * and parse the output image metadata files generated after
 * demosaicing and time synchronization has occurred.
 *
 * These files include the file names of the jpeg images,
 * as well as the meta-information for each image,
 * such as timestamp and camera settings.
 */

#include <fstream>
#include <string>
#include <bayer/ocam_functions.h>

/* the following classes are defined in this file */
class color_image_frame_t;
class color_image_reader_t;

/**
 * This class defines one image's metadata
 */
class color_image_frame_t
{
	/* parameters */
	public:

		/* File name of image on disk */
		std::string image_file;

		/* index of this image for its camera */
		int index;     /* indexed from 0 */

		/* the timestamp of the image, in units of seconds */
		double timestamp;

		/* camera settings when image was taken */
		int exposure; /* exposure time, microseconds */
		int gain; /* digital gain, range [1-4] */

	/* functions */
	public:

		/* constructors */

		/**
		 * Initialize empty frame
		 */
		color_image_frame_t();

		/**
		 * Frees all memory and resources for this frame
		 */
		~color_image_frame_t();

		/**
		 * Will parse the next image metadata line from file
		 *
		 * The stream is assumed to be a color_image metadata
		 * file, positioned in the body of the file (not in
		 * the header), at the start of a line.  The next
		 * line will be parsed and stored in this struct.
		 *
		 * @param is    The ascii stream to read and parse
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int parse(std::istream& is);

		/**
		 * Operator '=' will copy the specified value to this struct
		 *
		 * Will copy the given argument to this object.
		 *
		 * @param rhs  The right-hand side argument to copy
		 *
		 * @return     Returns this object
		 */
		inline color_image_frame_t operator = (
		                           const color_image_frame_t& rhs)
		{
			/* copy values */
			this->image_file = rhs.image_file;
			this->index = rhs.index;
			this->timestamp = rhs.timestamp;
			this->exposure = rhs.exposure;
			this->gain = rhs.gain;
		
			/* return result */
			return (*this);
		};
};

/**
 * This class will parse the color_image metadata file and 
 * yield metadata frames.
 */
class color_image_reader_t
{
	/* parameters */
	private:

		/* the ascii file to parse */
		std::ifstream infile;

		/* The camera name and information */
		std::string camera_name; /* name of camera */
		int num_images; /* number of images in this file */
		int jpeg_quality; /* quality of compression [1-100] */
		std::string output_dir; /* where images were written */

		/* camera fisheye calibration data */
		struct ocam_model calibration;

	/* functions */
	public:

		/* constructors */

		/**
		 * Initializes unopened, empty reader
		 */
		color_image_reader_t();

		/**
		 * Frees all memory and resources
		 */
		~color_image_reader_t();

		/**
		 * Open file for parsing
		 *
		 * Given the path to a file, will attempt to open
		 * the file and parse the header.  After this call,
		 * this object will be ready to return metadata values
		 * with the next() function.
		 *
		 * @param filename The file to open
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int open(const std::string& filename);

		/**
		 * Returns the name of the camera for the open file
		 *
		 * Returns the camera name that was stored in the
		 * file that is currently open.
		 *
		 * @return   Returns the camera name of current file
		 */
		inline std::string get_camera_name() const
		{
			return this->camera_name;
		};

		/**
		 * Parses the next frame from the file
		 *
		 * Will read the next line in the file, and populate
		 * the specified frame structure with its information.
		 *
		 * @param frame The struct to populate with the next frame
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int next(color_image_frame_t& frame);

		/**
		 * Returns true iff end of file reached
		 *
		 * Will pass the eof flag from the file stream as
		 * the return value of this function
		 *
		 * #returns     Returns true iff end-of-file reached
		 */
		bool eof() const;

		/**
		 * Closes the stream and frees resources
		 *
		 * Will close any open file streams and frees allocated
		 * resources of this class.
		 */
		void close();
};

#endif
