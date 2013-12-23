#ifndef FISHEYE_CAMERA_H
#define FISHEYE_CAMERA_H

/**
 * @file fisheye_camera.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the fisheye_camera_t class, which is used
 * to represent the intrinsic and extrinsic calibration, and the
 * poses, for camera imagery with a fisheye lens.
 */

#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/fisheye/ocam_functions.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <vector>
#include <string>

/**
 * The fisheye_camera_t class is used to represent camera output
 *
 * This class represents the pose of the sensor at each timestamp,
 * the calibration for the camera, and provides useful functions for
 * using these values.
 */
class fisheye_camera_t
{
	/* parameters */
	private:

		/**
		 * Provides the fisheye calibration parameters
		 */
		struct ocam_model calibration;

		/**
		 * Metadata information for each image frame
		 *
		 * Each element corresponds with an image on disk
		 */
		std::vector<color_image_frame_t> metadata;

		/**
		 * This list represents the camera poses for each frame
		 *
		 * It is dynamically allocated, since it contains Eigen
		 * structures. It is the same size as the metadata list.
		 */
		transform_t* poses;

	/* functions */
	public:
	
		/**
		 * Creates empty structure
		 */
		fisheye_camera_t();

		/**
		 * Frees all memory and resources
		 */
		~fisheye_camera_t();

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
		         const system_path_t& path);

		/**
		 * Clears all information from this structure.
		 *
		 * Clears all information from this structure and frees
		 * all resources being used.  Once called, init() can
		 * be called again with different input.
		 */
		void clear();

	// TODO left off here
};

#endif
