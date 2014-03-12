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
#include <string>
#include <iostream>
#include <Eigen/Dense>

/* the following types are used to represent matrices for this class */

/* dimensionality of various parameters */
#define NUM_SENSOR_INPUT_VARS 7
#define NUM_SCANPOINT_INPUT_VARS 10

/* output position covariance mat */
typedef Eigen::Matrix<double, 3, 3> pos_cov_t;

/* The covariance matrix used to represent uncertainties in the values
 * that are used as input in determining the distribution of the sensor's
 * position. This matrix has the following ordering:
 *
 *	roll
 *	pitch
 *	yaw
 *	x_sensor2system
 *	y_sensor2system
 *	z_sensor2system
 *	ts_std
 */
typedef Eigen::Matrix<double,
        NUM_SENSOR_INPUT_VARS, NUM_SENSOR_INPUT_VARS> sensor_cov_t;

/* The covariance matrix used to represent the uncertainties in the
 * values that are ued as input to determine the distribution of the
 * scan point's position in world coordinates.  The matrix has the
 * following ordering:
 *
 * 	roll
 * 	pitch
 * 	yaw
 * 	x_sensor2world
 * 	y_sensor2world
 * 	z_sensor2world
 * 	x_point2sensor
 * 	y_point2sensor
 * 	z_point2sensor
 * 	ts_std
 */
typedef Eigen::Matrix<double,
        NUM_SCANPOINT_INPUT_VARS, NUM_SCANPOINT_INPUT_VARS> scanpoint_cov_t;

/* the jacobain matrix of the transform from input random variables to
 * a sensor position in world coordinates.  The input variables are
 * the same as for the sensor covariance matrix, and the output size
 * is the length of the position vector (i.e. 3) */
typedef Eigen::Matrix<double,
        3, NUM_SENSOR_INPUT_VARS> sensor_jacobian_t;

/* The jacobian matrix of the transform from input random variables to
 * a scanpoint position in world coordinates.  The input variables are
 * the same as for the scanpoint covariance matrix, and the output
 * size is the length of the position vector (3). */
typedef Eigen::Matrix<double,
        3, NUM_SCANPOINT_INPUT_VARS> scanpoint_jacobian_t;

/* The input vector for a sensor position distribution has these
 * dimensions.  This is used as an input vector to be multiplied by
 * the corresponding jacobian */
typedef Eigen::Matrix<double,
        NUM_SENSOR_INPUT_VARS, 1> sensor_jacobian_input_t;

/* The input vector for a scan point position distribution has these
 * dimensions.  This is used as an input vector to be multiplied by
 * the corresponding jacobian */
typedef Eigen::Matrix<double,
        NUM_SCANPOINT_INPUT_VARS, 1> scanpoint_jacobian_input_t;

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

		/* Euler angles corresponding to this pose's rotation */
		double roll, pitch, yaw; /* units: radians */

		/**
		 * The following values are used to cache expensive
		 * matrix computations that are required for each scan
		 * point. The notation is a concatenation of the
		 * multiplied matrices.  For example:
		 *
		 * 	RzRydRxRtsRl2s = Rz * Ry * (dRx/droll) * R_ts 
		 * 				* sensor_calib.R
		 */
		Eigen::Matrix3d RzRydRxRtsRl2s;
		Eigen::Matrix3d RzdRyRxRtsRl2s;
		Eigen::Matrix3d dRzRyRxRtsRl2s;
		Eigen::Matrix3d RzRyRxRtspRl2s;
		Eigen::Matrix3d RzRyRxRtsRl2s;
		Eigen::Matrix3d RzRyRxRl2s;

		/**
		 * The covariance matrix for the input random variables
		 * used for modeling the scan point's position in
		 * world coordinates.
		 */
		scanpoint_cov_t input_scanpoint_cov;

		/*-----------------*/
		/* computed values */
		/*-----------------*/
		
		/**
		 * The output mean of the sensor's world position,
		 * as determined by all input parameters.
		 */
		Eigen::Vector3d output_sensor_mean;

		/**
		 * The output covariance matrix of the sensor's world
		 * position, as determined by all input parameters.
		 */
		pos_cov_t output_sensor_cov;

		/**
		 * The output mean of the scanpoint's world position,
		 * as determined by all input parameters
		 */
		Eigen::Vector3d output_scanpoint_mean;

		/**
		 * The output covariance matrix of the scanpoint's world
		 * position, as determined by all input parameters.
		 */
		pos_cov_t output_scanpoint_cov;

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
};

#endif