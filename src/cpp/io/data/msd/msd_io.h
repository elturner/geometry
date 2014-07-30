#ifndef MSD_WRITER_H
#define MSD_WRITER_H

/**
 * @file   msd_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to export .msd files
 *
 * @section DESCRIPTION
 *
 * The .msd file format is a deprecated format used by the gen-1
 * backpack processing code.  In order to interface the data products
 * of later generation backpacks with the gen-1 pipeline, this
 * writer class can be used to export information into the MSD format.
 *
 * This class requires the Eigen framework.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <Eigen/Dense>

/**
 * this namespace contains all msd in/out classes 
 */
namespace msd
{
	/* the following classes are defined in this namespace */
	class header_t;
	class frame_t;
	class writer_t;

	/**
	 * The msd::header_t class represents the header of a msd file
	 */
	class header_t
	{
		/* security */
		friend class writer_t;

		/* parameters */
		private:

			/* The serial number is represented as an
			 * integer */
			int serial_num;

			/* the transform from the specified laser's
			 * coordinate frame to the common coordinate
			 * frame of the backpack */
			Eigen::Vector3d T; /* translation (mm) */
			Eigen::Matrix3d R; /* rotation */

			/* number of frames in this file */
			int num_scans;

		/* functions */
		public:

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Serializes the header info to stream
			 *
			 * Given a binary stream, will export the
			 * information stored in this header object.
			 *
			 * @param os   The output stream to use
			 */
			void serialize(std::ostream& os) const;
	};

	/**
	 * The msd::frame_t class represents a single frame of a msd file
	 */
	class frame_t
	{
		/* parameters */
		public:

			/**
			 * number of points in this scan
			 */
			int num_points;

			/**
			 * timestamp of this frame (units: seconds)
			 */
			double timestamp;

			/**
			 * The scan points of this frame are stored
			 * in a matrix, where each column is a point,
			 * given dimensions 2xN
			 *
			 * units: millimeters
			 */
			Eigen::MatrixXd points;

		/* functions */
		public:

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Serializes the frame info to stream
			 *
			 * Given a binary stream, will export this
			 * frame information.
			 *
			 * @param os  The output stream to use
			 */
			void serialize(std::ostream& os) const;
	};
	
	/**
	 * The msd::writer_t class is used to export scans to msd files
	 */
	class writer_t
	{
		/* parameters */
		private:

			/* the header of this file */
			header_t header;
			
			/* the output stream for this writer */
			std::ofstream outfile;

		/* functions */
		public:

			/**
			 * Initializes info about this file and scanner
			 *
			 * Will initialize the header information about
			 * the scanner that will be stored in this file.
			 *
			 * @param serial   The serial number of the scanner
			 * @param R        Rotation matrix to common
			 * @param T        Translation vector (mm) to common
			 * @param num      Number of scan lines
			 */
			void init(int serial, const Eigen::Matrix3d& R,
				const Eigen::Vector3d& T, int num);

			/**
			 * Opens this writer to a given file
			 *
			 * Will open the specified file for writing.
			 * This will also write the header information,
			 * so the header should already be initialized
			 * before this call is made.
			 *
			 * @param filename   The file to open
			 *
			 * @return  Returns zero on success, non-zero
			 *          on failure.
			 */
			int open(const std::string& filename);

			/**
			 * Writes the given scan frame to file
			 *
			 * Given a scan frame, will write it to the
			 * (assumed to already be) opened file.
			 *
			 * @param frame   The frame to export
			 */
			void write(const frame_t& frame);

			/**
			 * Closes this file
			 *
			 * Clears all memory and resources of this file
			 */
			void close();
	};

}

#endif
