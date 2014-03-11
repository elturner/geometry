#ifndef SCAN_MODEL_H
#define SCAN_MODEL_H

/**
 * @file scan_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the scan_model_t class, which is used to
 * model the probability distribution of a sensor's pose position
 * and a corresponding scan point position in world coordinates.
 *
 * This distribution is affected by: timestamp inaccuracies, system
 * velocity (linear and rotational), measured system pose and corresponding
 * covariance estimates of that pose, extrinsic transform from system common
 * to sensor position, and scanner intrinsic noise.
 *
 * This class requires the Eigen framework.
 */

#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <string>
#include <iostream>
#include <Eigen/Dense>

/**
 * The scan_model_t class models noise in a sensor's world-coords position.
 *
 * This class is used to estimate the probability distribution associated
 * with a sensor's position in world coordinates.  This model is only
 * valid for a given frame of the sensor, and must be updated for each
 * subsequent frame.
 */
class scan_model_t
{
	/* parameters */
	private:

		/*------------------------*/
		/* sensor-specific values */
		/*------------------------*/

		/* standard deviation of timestamp error for the current
		 * sensor, as modeled by linear fit error */
		double ts_std; /* units: seconds */
		
		/* The sensor -> system transform for the current sensor */
		transform_t sensor_calib;

		/*-----------------------*/
		/* frame-specific values */
		/*-----------------------*/

		/* The deterministic pose of a sensor */
		pose_t pose; /* this is the maximum-likelihood pose */

		/*---------------*/
		/* cached values */
		/*---------------*/

		/**
		 * The following values are used to cache expensive
		 * matrix computations that are required for each scan
		 * point.
		 */
		Eigen::Matrix3d R_s2w; /* rotation from system to world */
		Eigen::Matrix3d R_s2w_t; /* transpose of R_s2w */
		Eigen::Matrix3d R_l2w; /* rotation matrix from sensor
		                        * coordinates to world coords */
		Eigen::Matrix3d R_l2w_t; /* transpose of R_l2w */
		Eigen::Matrix3d twwt; /* outer product of angular velocity
		                       * axis of rotation, scaled by 
		                       * uncertainty in angle (radians^2) */

		/*------------------------------*/
		/* computed distribution values */
		/*------------------------------*/
		
		/**
		 * The output mean of the sensor's world position,
		 * as determined by all input parameters.
		 */
		Eigen::Vector3d output_sensor_mean;

		/**
		 * The output covariance matrix of the sensor's world
		 * position, as determined by all input parameters.
		 */
		Eigen::Matrix3d output_sensor_cov;

		/**
		 * The output mean of the scanpoint's world position,
		 * as determined by all input parameters
		 */
		Eigen::Vector3d output_scanpoint_mean;

		/**
		 * The output covariance matrix of the scanpoint's world
		 * position, as determined by all input parameters.
		 */
		Eigen::Matrix3d output_scanpoint_cov;

	/* functions */
	public:
	
		/*--------------*/
		/* constructors */
		/*--------------*/

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Constructs empty pose model
		 */
		scan_model_t();

		/*--------------------------*/
		/* initialization functions */
		/*--------------------------*/

		/**
		 * Resets the sensor-specific parameters
		 *
		 * Will initialize all paramters of this object that
		 * depend on the sensor hardware configuration but not
		 * specific frame information.
		 *
		 * @param sensor_name     The name of the sensor
		 * @param timesync_err    The time-synchronization error
		 *                        for this sensor (std. dev.) in
		 *                        units of seconds.
		 * @param path            The system path object.
		 *
		 * @return Returns zero on success, non-zero on failure.
		 */
		int set_sensor(const std::string& sensor_name,
		               double timesync_err,
		               const system_path_t& path);

		/**
		 * Resets the frame-specific parameters
		 *
		 * Will initialize all parameters of this object that
		 * depend on a specific sensor frame.  This function
		 * should be called only after the set_sensor() function
		 * is called.
		 *
		 * @param time    The timestamp of the desired frame
		 * @param path    The system path object
		 *
		 * @return  Returns zero on success, non-zero on failure.
		 */
		int set_frame(double time, const system_path_t& path);

		/**
		 * Resets the scan-point-specific parameters
		 *
		 * Will initialize all internal parameters that are
		 * dependent on the specific scan-point observed
		 * by the sensor in the current frame.  This function
		 * should only be called after set_sensor() and set_frame()
		 * have been called at least once each.
		 *
		 * @param p    The scan point and its corresponding noise
		 */
		void set_point(const noisy_scanpoint_t& p);

		/**
		 * Initializes the given carve map structure with results
		 *
		 * Will initialize the input carve map structure with the
		 * computed results from this object.  This function should
		 * be called after the call to set_point().  This call
		 * will copy over the global gaussian distributions of
		 * the sensor and scan-point positions.
		 *
		 * @param cm   The carve map to initialize
		 */
		void populate(carve_map_t& cm) const;

		/*---------------------*/
		/* debugging functions */
		/*---------------------*/

		/**
		 * Will export gaussian models to file
		 *
		 * Will export the parameters for the gaussian models
		 * of the sensor position and the scan point position
		 * in world coordinates to the specified file stream.
		 *
		 * The output will be formatted in ascii, written in two
		 * lines, in the following notation:
		 *
		 * <sx> <sy> <sz> <sc00> <sc01> <sc02> ... <sc21> <sc22>
		 * <px> <py> <pz> <pc00> <pc01> <pc02> ... <pc21> <pc22>
		 *
		 * Where [<sx>,<sy>,<sz>] is the mean of the sensor's 
		 * position, [<px>,<py>,<pz>] is the mean of the scanpoint's
		 * position, <scij> represents element i,j of the sensor's
		 * covariance matrix, and <pcij> is element i,j of the
		 * scanpoint's covariance matrix.
		 *
		 * @param out   The output stream to write this info
		 */
		void serialize(std::ostream& out) const;

		/**
		 * Will export meshes of gaussian models to OBJ file
		 *
		 * Will export a mesh representing the basic shape
		 * of the probability distribution of this point/sensor
		 * pair to a wavefront OBJ file stream.
		 *
		 * @param out   The output stream to write to
		 */
		void writeobj(std::ostream& out) const;
};

#endif
