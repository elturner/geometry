#ifndef LOG_READER_H
#define LOG_READER_H

/**
 * @file   log_reader.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Reader class for OpenNI log files
 *
 * @section DESCRIPTION
 *
 * This file contains the log_reader_t class, which is used to 
 * read .log files.  These files contain the trajectory of a
 * PrimeSense sensor and correspond to the data in a .oni file.
 *
 * This class requires the Eigen framework.
 */

#include <string>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>

/**
 * The log_reader_t class is used to parse ascii .log files
 */
class log_reader_t
{
	/* parameters */
	private:

		/**
		 * The contents of the file
		 *
		 * The list of poses for each frame.  Each pose is stored
		 * as a 4x4 transformation matrix.
		 */
		std::vector<Eigen::Matrix4d,
			Eigen::aligned_allocator<Eigen::Matrix4d> > poses;

	/* functions */
	public:

		/**
		 * Opens a file for reading
		 *
		 * Will attempt to open the file at the given
		 * location.  This should be a .log file that
		 * contains PrimeSense trajectory information.
		 *
		 * @param filename  The file to open
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int parse(const std::string& filename);

		/**
		 * Uses the imported poses to convert a pixel to a 3D point
		 *
		 * Given the (u,v) coordinates of a pixel for frame f, as
		 * well as the depth value d at that pixel, this function
		 * will compute the world coordinates of the 3D point
		 * represented by that pixel.
		 *
		 * @param f    The index of the frame (indices start at 0)
		 * @param u    The horizontal pixel index (column)
		 * @param v    The vertical pixel index (row)
		 * @param d    The depth value of the pixel (millimeters)
		 * @param p    Where to store the 3D point in world coords
		 *             (the point will have units of meters)
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int compute_point(size_t f, size_t u, size_t v, double d,
				Eigen::Vector3d& p) const;
};

#endif
