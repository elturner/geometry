#ifndef NOISE_MODEL_H
#define NOISE_MODEL_H

/**
 * @file noise_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The noise_model_t class defines the noise of the system (and its
 * corresponding scanners) from all error sources.  It can generate random
 * samples of both a scanner position and corresponding scan point
 * position.
 *
 * This file requires Eigen.
 */

#include <io/data/fss/fss_io.h>
#include <goemetry/system_path.h>
#include <geometry/probability/noisy_timestamp.h>
#include <geometry/probability/noisy_scan.h>
#include <Eigen/Dense>
#include <string>

/* the noisy system class */
class noise_model_t
{
	/* parameters */
	private:

		/* the sensor's random poses are determined by the
		 * following components of the localization: */
		noisy_timestamp_t clock; /* the system clock has noise */
		system_path_t path; /* the path localization has noise */
		transform_t* scanner_calib; /* the scanner's transform
		                             * with respect to system
		                             * common */
		noisy_scan_t scan; /* instrinsic noise from scanners */

	/* functions */
	public:
		
		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Initializes empty object.
		 */
		noise_model_t();

		/**
		 * Frees all memory and resources.
		 */
		~noise_model_t();

		/* init */

		/**
		 * Initializes the system path to use
		 *
		 * Will parse the specified files, and use the
		 * parsed information when modeling scanners.  These
		 * files indicates the scanners poses over time and
		 * the relative transform between scanners.
		 *
		 * @param madfile   The input .mad file to parse
		 * @param conffile  The input hardware config file to parse
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int set_path(const std::string& madfile,
		             const std::string& conffile);

		/**
		 * Prepare this model with a given sensor.
		 *
		 * Calling this function will prepare this object to
		 * generate random samples of the world-position of the
		 * scanner and the scanned point for the specified
		 * sensor name.
		 *
		 * @param sn   The name of the sensor to model.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int set_sensor(const std::string& sn);

		/**
		 * Prepare this model with a given timestamp.
		 *
		 * Calling this function will prepare this object
		 * to generate random samples of scan positions based
		 * on the frame of the selected sensor at this specified
		 * timestamp.
		 *
		 * @param ts   The current timestamp to model
		 * @param n    The uncertainty (std. dev.) of this timestamp
		 */
		void set_timestamp(double ts, double n);

		/**
		 * Prepare this model with a given scan point
		 *
		 * The input scan point is represented in the sensor's
		 * coordinate frame, and should include statistical
		 * information about this scan.
		 *
		 * @param p   Point structure for this scan
		 */
		void set_scan(const fss::point_t& p);

		/* sampling */

		/**
		 * Generates a random sample of the initialzied model
		 *
		 * Given the characteristics set to this model, will
		 * generate a random sample for both the position
		 * of the sensor and the position of the scan point.
		 * This sample will be taken from the constructed
		 * distribution from the configured timestamp, path,
		 * calibration, and sensor noise.
		 *
		 * All output will be in world coordinates.
		 *
		 * @param sensor_pos    The output sampled sensor pos
		 * @param scan_pos      The output sampled scan point pos
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int generate_sample(Eigen::Vector3d& sensor_pos,
		                    Eigen::Vector3d& scan_pos) const;
};

#endif
