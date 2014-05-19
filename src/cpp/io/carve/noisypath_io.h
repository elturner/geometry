#ifndef NOISYPATH_IO_H
#define NOISYPATH_IO_H

/**
 * @file noisypath_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief These files contain the i/o functionality for noisypath_t
 *
 * @section DESCRIPTION
 *
 * The classes in this file are used to read and write .noisypath files,
 * which house the probability distributions of the localization path
 * positions and rotations.  These files contain a superset of the info
 * stored in .mad files.
 */

#include <Eigen/Dense>
#include <fstream>
#include <string>
#include <vector>
#include <ios>

/**
 * This namespace houses all i/o classes for .noisypath files
 */
namespace noisypath_io
{
	/* the following classes are defined in this namespace */
	class reader_t;
	class writer_t;
	class header_t;
	class zupt_t;
	class pose_t;
	class gauss_dist_t;

	/* the following values are used in this namespace */

	/**
	 * This value indicates the magic number that represents .noisypath
	 * files.
	 */
	static const std::string MAGIC_NUMBER = "noisypath";

	/**
	 * This value indicates the number of bytes used by the magic
	 * number in the binary noisypath file.
	 */
	static const size_t MAGIC_NUMBER_SIZE = (1+MAGIC_NUMBER.size());

	/**
	 * This value indicates the size of the header on disk
	 */
	static const size_t HEADER_SIZE = (MAGIC_NUMBER_SIZE
				+ 2*sizeof(unsigned int));

	/**
	 * The size of one zupt element on dist.  Each zupt contains two
	 * doubles (the start and end times for the zupt).
	 */
	static const size_t ZUPT_ELEMENT_SIZE = (2*sizeof(double));

	/**
	 * The size of one noisy pose in the file
	 *
	 * Each pose consists of:
	 *
	 * 	timestamp                                 (1 double)
	 *	mean position                             (3 doubles)
	 *	upper triangle for covariance of position (6 doubles)
	 *	mean rotation                             (3 doubles)
	 *	upper triangle for covariance of rotation (6 doubles)
	 */
	static const size_t POSE_ELEMENT_SIZE = (19*sizeof(double));

	/**
	 * the zupt_t class represents a zupt interval in the file
	 */
	class zupt_t
	{
		/* parameters */
		public:
		
			/* start and stop times of the zupt are measured
			 * in seconds according to the synchronized
			 * system clock. */
			
			/**
			 * The starting time for this zupt (seconds)
			 */
			double start_time;
			
			/**
			 * The ending time for this zupt (seconds)
			 */
			double end_time;

		/* functions */
		public:

			/**
			 * Will export zupt information to binary stream
			 *
			 * Will write the specified zupt information
			 * to the given binary stream.  This function
			 * is called by the writer_t class.
			 *
			 * @param os   The output stream to write to
			 */
			void serialize(std::ostream& os) const;

			/**
			 * Will parse zupt information from binary stream
			 *
			 * Given an input binary stream, will read a single
			 * zupt interval from that stream, and store the
			 * data in this object.
			 *
			 * @param is    The input binary stream to parse
			 *
			 * @return      Returns zero on success, non-zero
			 *              on failure.
			 */
			int parse(std::istream& is);
	};

	/**
	 * The gauss_dist_t class represents a 3D gaussian distribution
	 * in a .noisypath file
	 */
	class gauss_dist_t
	{
		/* parameters */
		public:

			/* the gaussian distribution is represented
			 * by the mean and covariance matrix. */

			/**
			 * The three-element mean of this distribution
			 */
			Eigen::Vector3d mean;
			
			/**
			 * The covariance matrix of this distribution
			 */
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
	 * The pose_t class is used to represent the data for a single
	 * pose in a .noisypath file
	 */
	class pose_t
	{
		/* parameters */
		public:

			/**
			 * The timestamp of this pose (in seconds)
			 */
			double timestamp;

			/**
			 * The mean and covariance of the position
			 *
			 * This distribution represents the position
			 * component of the pose.  This represents
			 * the location of the system origin in world
			 * coordinates.
			 */
			gauss_dist_t position;

			/**
			 * The mean and covariance of the rotation
			 *
			 * This distribution represents the rotation
			 * component of the pose, specifically roll,
			 * pitch, and yaw.
			 */
			gauss_dist_t rotation;

		/* functions */
		public:
		
			/* Since this class contains Eigen structures, we
			 * need to properly align memory */
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			/**
			 * Parses the pose from binary stream
			 *
			 * Will parse a single pose's worth of data from
			 * the given binary stream and will store it in
			 * this structure.
			 *
			 * @param is  The input stream should be aligned to
			 *            the start of the pose.
			 *
			 * @return    Returns zero on success,
			 *            non-zero on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Will serialize this pose to stream
			 *
			 * Assuming this pose is populated with the
			 * appropriate statistical information, this call
			 * will write that data to the stream.
			 *
			 * @param os   The output stream to use
			 */
			void serialize(std::ostream& os) const;
	};

	/**
	 * The header_t class is used to represent the header of a 
	 * noisypath file.
	 */
	class header_t
	{
		/* security */
		friend class reader_t;
		friend class writer_t;

		/* parameters */
		private:

			/**
			 * The number of poses in this file
			 */
			unsigned int num_poses;
			
			/**
			 * The list of zupts for this file
			 */
			std::vector<zupt_t> zupts;

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
			void serialize(std::ostream& os) const;
	};
	
	/**
	 * The reader_t class can parse a .noisypath file and give random-
	 * access to its contents
	 */
	class reader_t
	{
		/* parameters */
		private:

			/* the file stream to the noisypath file */
			std::ifstream infile;

			/* the header information of this file */
			header_t header;

			/* the list of pose timestamps from this file */
			std::vector<double> timestamps;

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
			 * Opens noisypath file for reading.
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
			inline size_t num_poses() const
			{ return this->header.num_poses; };

			/**
			 * Retrieves the zupts in this file
			 *
			 * Will copy the zupt information into the given
			 * vector.
			 *
			 * @param zupts   Where to store the zupt info 
			 */
			inline void get_zupts(
				std::vector<zupt_t>& zupts) const
			{
				zupts.insert(zupts.begin(),
					this->header.zupts.begin(), 
					this->header.zupts.end());
			};

			/**
			 * Parses the specified pose and stores results
			 *
			 * Will retrieve the specified pose and store
			 * its information in the given structure.
			 *
			 * @param p    Where to store the pose information
			 * @param i    The index of pose to retrieve
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int read(pose_t& p, unsigned int i);
	
			/**
			 * Get nearest pose to the given timestamp
			 *
			 * Will retrieve the pose from the file
			 * that has a timestamp that is closest
			 * to the given time.
			 *
			 * @param p    Where to store the pose info
			 * @param t    The timestamp to analyze
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int read_nearest(pose_t& p, double t);
	};

	/**
	 * The writer_t class can generate .noisypath files
	 */
	class writer_t
	{
		/* parameters */
		private:

			/* the output file stream */
			std::ofstream outfile;

			/* the header information for this file */
			header_t header;

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
			 * @param zupts        The zupt info to export
			 *
			 * @return   Returns zero on success, non-zero
			 *           on failure.
			 */
			int open(const std::string& filename,
			         const std::vector<zupt_t>& zupts);
	
			/**
			 * Closes any open streams
			 *
			 * If this object has open streams, they will
			 * be closed.  If no open streams exist, then
			 * this call is a no-op.
			 */
			void close();
	
			/**
			 * Writes a pose to file
			 *
			 * Will write the specified pose information to
			 * the exported file.
			 *
			 * @param p   The pose information to write to file
			 *
			 * @return    Returns zero on success, non-zero
			 *            on failure.
			 */
			int write(const pose_t& p);
	};
}

#endif
