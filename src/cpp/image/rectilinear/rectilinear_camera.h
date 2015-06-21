#ifndef RECTILINEAR_CAMERA_H
#define RECTILINEAR_CAMERA_H

/**
 * @file rectilinear_camera.h
 * @author Nicholas Corso <ncorso@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the rectilinear_camera_t class, which is used
 * to represent the intrinsic and extrinsic calibration, and the
 * poses, for camera imagery with a rectilinear lens.
 */
#include <image/camera.h>
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/image_cache.h>
#include <image/rectilinear/rectilinear_functions.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <vector>
#include <string>
#include <Eigen/StdVector>

/**
 * The rectilinear_camera_t class is used to represent camera output
 *
 * This class represents the pose of the sensor at each timestamp,
 * the calibration for the camera, and provides useful functions for
 * using these values.
 */
class rectilinear_camera_t : public camera_t
{
	/* parameters */
	private:

		/**
		 * Provides the rectilinear calibration parameters
		 */
		rcam_model calibration;


	/* functions */
	public:
	
		/**
		 * Creates empty structure
		 */
		rectilinear_camera_t();

		/**
		 * Frees all memory and resources
		 */
		~rectilinear_camera_t();

		/**
		 * Initializes this structure based on input files
		 *
		 * Given a calibration, metadata, and position information,
		 * will initialize this camera structure by parsing these
		 * files.
		 *
		 * @param calibfile   Input binary ocam calibration file
		 * @param metafile    Input metadata file (post-timesync)
		 * @param path        The system path structure
		 *
		 * @return  Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& calibfile,
		         const std::string& metafile,
		         const std::string& imgdir,
		 	 const system_path_t& path);

		/**
		 * Initializes this structure based on input files
		 *
		 * Given a MCD (Model Camera Data) file and a path,
		 * will initialize this camera structure.
		 *
		 * @param mcdfile   The MCD file to import
		 * @param path      The system path structure
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int init_mcd(const std::string& mcdfile,
		             const system_path_t& path);

		/**
		 * Clears all information from this structure.
		 *
		 * Clears all information from this structure and frees
		 * all resources being used.  Once called, init() can
		 * be called again with different input.
		 */
		void clear();

		/**
		 * Gets the point color and camera-relative properties
		 *
		 * Retrieves the color and confidence of coloring
		 * for the specified point using the nearest
		 * camera (w.r.t. time).
		 *
		 * @param px  The input world x-coordinate of the point
		 * @param py  The input world y-coordinate of the point
		 * @param pz  The input world z-coordinate of the point
		 * @param t   The input timestamp of this point
		 * @param r   Output red component of coloring
		 * @param g   Output green component of coloring
		 * @param b   Output blue component of coloring
		 * @param q   Output quality, cos(angle) to camera norm
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int color_point(double px, double py, double pz, double t,
		                int& r, int& g, int& b, double& q);
};

#endif
