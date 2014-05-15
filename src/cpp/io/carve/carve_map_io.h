#ifndef CARVE_MAP_IO_H
#define CARVE_MAP_IO_H

/**
 * @file carve_map_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief These files contain the i/o functionality for carve_map_t
 *
 * @section DESCRIPTION
 *
 * The classes in this file are used to read and write .carvemap files,
 * which house the probability distributions of the input scan points
 * and the sensor positions, which are modeled as gaussians in global 3D
 * coordinates.
 */

#include <geometry/carve/gaussian/carve_map.h>
#include <Eigen/Dense>
#include <fstream>
#include <string>
#include <vector>
#include <ios>

/**
 * This namespace houses all i/o classes for .carvemap files
 */
namespace cm_io
{
	/* the following classes are defined in this namespace */
	class reader_t;
	class writer_t;
	class header_t;
	class frame_t;
	class gauss_dist_t;

	/* the following values are used in this namespace */

	/**
	 * This value indicates the magic number that represents .carvemap
	 * files.
	 */
	static const std::string MAGIC_NUMBER = "carvmap";

	/**
	 * This value indicates the number of bytes used by the magic
	 * number in the binary carvemap file.
	 */
	static const size_t MAGIC_NUMBER_SIZE = (1+MAGIC_NUMBER.size());

	/**
	 * This value indicates the size of the header in the file
	 */
	static const size_t HEADER_SIZE=(MAGIC_NUMBER_SIZE+sizeof(size_t));
	
	/**
	 * The size of a vector stored in the file
	 *
	 * A vector is three 8-byte doubles, for a total of 24 bytes.
	 */
	static const size_t VECTOR_SIZE = (3*sizeof(double));

	/**
	 * The size of a covariance matrix in file
	 *
	 * A covariance matrix is represented as a 3x3 matrix, but since
	 * this matrix is symmetric, only 6 values need to be explicitly
	 * written.  Values are written in row-major order, and is used to
	 * represent the uncertainty component of a probability 
	 * distribution.
	 *
	 * Here is the ordering of the values stored in file for a 
	 * covariance matrix:
	 *
	 *	[0]	[1]	[2]
	 *
	 *	 -	[3]	[4]
	 *
	 *	 -	 -	[5]
	 */
	static const size_t COV_MAT_SIZE = (6*sizeof(double));
	
	/**
	 * The size of a multi-variate gaussian distribution in file
	 *
	 * A gaussian distribution is represented by a mean location
	 * (a vector) and a covariance matrix.
	 */
	static const size_t GAUSS_DIST_SIZE = (VECTOR_SIZE + COV_MAT_SIZE);

	/**
	 * The size of a frame-header on disk
	 */
	static const size_t FRAME_HEADER_SIZE = (sizeof(size_t) 
						+ GAUSS_DIST_SIZE);

	/**
	 * The size of one scan point in the file
	 *
	 * Each point consists of the distribution of the point in space,
	 * the planarity estimate of that point, and the corner estimate
	 * of that point.
	 */
	static const size_t POINT_INFO_SIZE = (sizeof(double)
				+ sizeof(double) + GAUSS_DIST_SIZE);

	/**
	 * The gauss_dist_t class represents a 3D gaussian distribution
	 * in a .carvemap file
	 */
	class gauss_dist_t
	{
		/* parameters */
		public:

			/* the gaussian distribution is represented
			 * by the mean and covariance matrix. */
			Eigen::Vector3d mean;
			Eigen::Matrix3d cov;

		/* functions */
		public:
		
			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			/**
			 * Serialize the stored info to file
			 *
			 * This will write the mean and the upper-triangle
			 * of the covariance matrix to file, in row-major
			 * order.
			 *
			 * @param os   The output stream to use
			 */
			void serialize(std::ostream& os) const;

			/**
			 * Will parse a gauss_dist_t from given stream
			 *
			 * Will parse the information from the given
			 * file stream, and store the resulting info
			 * in this structure.  This will read a vector
			 * and the upper half of the covariance matrix
			 * from file, populating the mean and covariance
			 * matrix in this structure, under the assumption
			 * that the covariance matrix is symmetric.
			 *
			 * @param is   The input stream to use
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int parse(std::istream& is);
	};

	/**
	 * The frame_t class is used to represent the data for a single
	 * scan frame in a .carvemap file
	 */
	class frame_t
	{
		/* security */
		friend class reader_t;
		friend class writer_t;

		/* parameters */
		private:

			/* the position of the start of frame in file */
			std::streampos fileloc;

			/* Number of scan points in this frame */
			size_t num_points;

			/* the distribution of the sensor position for
			 * this frame */
			gauss_dist_t sensor_pos;

		/* functions */
		public:
		
			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			/**
			 * Initializes empty frame
			 */
			frame_t();

			/**
			 * Parses the frame's header info
			 *
			 * This function will parse the number of points
			 * and the sensor pos from a frame that is contained
			 * in the input stream.
			 *
			 * @param is  The input stream should be aligned to
			 *            the start of the frame.
			 *
			 * @return    Returns zero on success,
			 *            non-zero on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Will serialize this frame's header info to stream
			 *
			 * Assuming this frame is populated with frame-
			 * specific header information needed, will write
			 * the frame-header to the given stream.
			 *
			 * @param os   The output stream to use
			 */
			void serialize(std::ostream& os) const;
	};

	/**
	 * The header_t class is used to represent the header of a 
	 * carvemap file.
	 */
	class header_t
	{
		/* security */
		friend class reader_t;
		friend class writer_t;

		/* parameters */
		private:

			/* the number of farmes defined in this file */
			size_t num_frames;

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
	 * The reader_t class can parse a .carvemap file and give random-
	 * access to its contents
	 */
	class reader_t
	{
		/* parameters */
		private:

			/* the file stream to the carvemap file */
			std::ifstream infile;

			/* the header information of this file */
			header_t header;

			/* the list of frames in the file.  This is
			 * populated in the call to open().  Note this
			 * frame list is dynamically allocated, and must
			 * be appropriately freed on deconstruction */
			frame_t* frames;

		/* functions */
		public:

			/**
			 * initializes empty reader
			 */
			reader_t();

			/**
			 * Frees all memory and resources
			 */
			~reader_t();

			/**
			 * Opens carvemap file for reading.
			 *
			 * On this call, will attempt to open, read,
			 * and parse the given file.  If successfully,
			 * this call will populate the header and frame
			 * structures in this object.
			 *
			 * @param filename   The location of file to read
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int open(const std::string& filename);

			/**
			 * Closes any open streams
			 *
			 * This call will close the file if it is open.  If
			 * no file is open, then this call is a no-op. This
			 * call will also free any used dynamic memory.
			 */
			void close();

			/**
			 * Retrieves the number of frames in this file
			 *
			 * @return   Returns number of frames in file
			 */
			inline size_t num_frames() const
			{ return this->header.num_frames; };

			/**
			 * Retrieves the number of points in the given frame
			 *
			 * @param f  The frame index to check
			 *
			 * @return   Returns number of points in frame #f
			 */
			inline size_t num_points_in_frame(size_t f) const
			{
				/* check if this reader is populated */
				if(this->frames == NULL
					|| f >= this->header.num_frames)
					return 0;

				/* check given frame */
				return this->frames[f].num_points;
			};

			/**
			 * Parses the specified carvemap and stores results
			 *
			 * Given the frame and point indices of a desired
			 * carve map that is contained in the input file,
			 * will parse the info of this carve map and store
			 * the results in the given structure.
			 *
			 * @param cm   Where to store the parsed data
			 * @param f    The frame index of this carve map
			 * @param i    The point index within frame
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int read(carve_map_t& cm, size_t f, size_t i);
	};

	/**
	 * The writer_t class can generate .carvemap files
	 */
	class writer_t
	{
		/* parameters */
		private:

			/* the output file stream */
			std::ofstream outfile;

		/* functions */
		public:

			/**
			 * Frees all memory and resources
			 */
			~writer_t();

			/**
			 * Opens the specified file for writing
			 *
			 * Will attempt to access the specified file
			 * for writing.  This call will export the header
			 * information to the file.
			 *
			 * @param filename     The file path to write to
			 * @param num_frames   The number of frames in file
			 *
			 * @return   Returns zero on success, non-zero
			 *           on failure.
			 */
			int open(const std::string& filename,
			         size_t num_frames);
	
			/**
			 * Closes any open streams
			 *
			 * If this object has open streams, they will
			 * be closed.  If no open streams exist, then
			 * this call is a no-op.
			 */
			void close();
	
			/**
			 * Writes a frame based on the list of scan points
			 *
			 * Given an array of scan points, will write a new
			 * frame to the file.
			 * 
			 * @param cm_arr   Pointer to an array of carve maps
			 * @param num      Length of cm_arr
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int write_frame(const carve_map_t* cm_arr,
			                size_t num);
	};
}

#endif
