#include "scan_model.h"
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <util/error_codes.h>
#include <util/rotLib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <Eigen/Dense>

/**
 * @file scan_model.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the scan_model_t class, which is used to
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

using namespace std;
using namespace Eigen;

/* function implementations */
		
/*--------------*/
/* constructors */
/*--------------*/
		
scan_model_t::scan_model_t()
{
	/* initialize parameters to assume input variables with no
	 * randomness, just as a default value */
	this->ts_std = 0.0; /* units: seconds */
	/* this->sensor_calib does not need initing */
	/* this->pose does not need initing */
	/* this->roll,pitch,yaw don't need initing */
	/* cached matricies don't need initing */
	this->input_scanpoint_cov = scanpoint_cov_t::Zero();
	this->output_sensor_mean = Vector3d::Zero();
	this->output_sensor_cov = pos_cov_t::Zero();
	this->output_scanpoint_mean = Vector3d::Zero();
	this->output_scanpoint_cov = pos_cov_t::Zero();
}

/*--------------------------*/
/* initialization functions */
/*--------------------------*/

int scan_model_t::set_sensor(const string& sensor_name,
                             double timesync_err,
                             const system_path_t& path)
{
	int ret;

	/* record the time synchronization standard deviation */
	this->ts_std = timesync_err;

	/* extract the extrinsic sensor calibration from the system path */
	ret = path.get_extrinsics_for(this->sensor_calib, sensor_name);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* all other values depend on the frame or scan point */
	return 0;
}

int scan_model_t::set_frame(double time, const system_path_t& path)
{
	Vector3d euler, T_ts;
	Matrix3d wx, wx_sq, Rx, Ry, Rz, dRx, dRy, dRz, RtsRl2s;
	double w_abs, w0, w1, w2, theta, sintheta, costheta;
	double cr, sr, cp, sp, cy, sy;
	int ret;
		
	/* the rotation matrix associated with the orientation
	 * of this pose */
	Eigen::Matrix3d R_s2w; /* system -> world */
	Eigen::Matrix3d R_dr; /* Rz * Ry * dRx/droll */
	Eigen::Matrix3d R_dp; /* Rz * dRy/dpitch * Rx */
	Eigen::Matrix3d R_dy; /* dRz/dyaw * Ry * Rx */

	/* useful representations of the rotation matrix defined
	 * by how the uncertainty in timestamp and the current
	 * angular velocity affect the pose of the sensor.
	 *
	 * R_ts = I3 + [w]_x * sin(|w|*ts)
	 * 		+ [w]_x^2 * (1 - cos(|w|*ts))
	 *
	 * R_ts_p = [w]_x*cos(|w|*ts)*|w| + [w]_x^2*sin(|w|*ts)*|w|
	 */
	Eigen::Matrix3d R_ts; /* rotation matrix of ang. vel. */
	Eigen::Matrix3d R_ts_p; /* derivative of R_ts w.r.t ts */

	/* The covariance matrix for the input random variables
	 * for modeling the pose position. */
	sensor_cov_t input_sensor_cov;
		
	/* The jacobian computed for the sensor transform
	 * function.  This matrix is useful for estimating
	 * the gaussian parameters of the output distribution
	 * for the sensor point position. */
	sensor_jacobian_t J_sensor;

	/* --- start computation --- */

	/* compute the system pose for this timestamp */
	ret = path.compute_pose_at(this->pose, time);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* import covariance information about the 6-dof pose,
	 * and store in both the input covariance matrices */
	input_sensor_cov = sensor_cov_t::Zero();
	// TODO import pose uncertainties
	input_sensor_cov(6,6) = this->ts_std * this->ts_std; /* variance */
	
	/* compute the roll, pitch, and yaw for this pose */
	R_s2w = this->pose.R.toRotationMatrix();
	rotLib::rot2rpy(R_s2w, euler);
	this->roll = euler(0);
	this->pitch = euler(1);
	this->yaw = euler(2);

	/* also compute rotation matrices for each axis */
	cr = cos(this->roll);
	sr = sin(this->roll);
	cp = cos(this->pitch);
	sp = sin(this->pitch);
	cy = cos(this->yaw);
	sy = sin(this->yaw);
	Rx << 1,0,0,0,cr,-sr,0,sr,cr;
	Ry << cp,0,sp,0,1,0,-sp,0,cp;
	Rz << cy,-sy,0,sy,cy,0,0,0,1;
	dRx << 0,0,0,0,-sr,-cr,0,cr,-sr;
	dRy << -sp,0,cp,0,0,0,-cp,0,-sp;
	dRz << -sy,-cy,0,cy,-sy,0,0,0,1;
	R_dr = Rz * Ry * dRx;
	R_dp = Rz * dRy * Rx;
	R_dy = dRz * Ry * Rx;

	/* precompute matrix representations of angular velocity */
	w_abs = this->pose.w.norm();
	w0 = this->pose.w(0)/w_abs;
	w1 = this->pose.w(1)/w_abs;
	w2 = this->pose.w(2)/w_abs;
	wx <<   0, -w2,  w1,
	       w2,   0, -w0,
	      -w1,  w0,   0;
	wx_sq = wx * wx;

	/* precompute rotation matrices that affect the system caused
	 * by timestamp uncertainty and angular velocity of system */
	theta = w_abs * this->ts_std;
	sintheta = sin(theta);
	costheta = cos(theta);	
	R_ts = Matrix3d::Identity() + (wx*sintheta)
	             + (wx_sq*(1-costheta));
	R_ts_p = (wx*costheta + wx_sq*sintheta)*w_abs;

	/* populate the jacobian to use for estimating the sensor
	 * position's distribution */
	T_ts = R_ts * this->sensor_calib.T;
	J_sensor.block(0,0,3,1) = R_dr * T_ts;
	J_sensor.block(0,1,3,1) = R_dp * T_ts;
	J_sensor.block(0,2,3,1) = R_dy * T_ts;
	J_sensor.block(0,3,3,3) = Matrix3d::Identity(); /* always */
	J_sensor.block(0,6,3,1) = (R_s2w * R_ts_p * this->sensor_calib.T) 
				+ this->pose.v;

	/* the mean of the output distribution should be the deterministic
	 * position of the sensor in world coordinates, since all
	 * input errors are assumed to be zero-mean */
	this->output_sensor_mean = (R_s2w * this->sensor_calib.T) 
	                           + this->pose.T; /* deterministic pos */
	
	/* compute the output distribution for the sensor position */
	this->output_sensor_cov = J_sensor * input_sensor_cov
					* J_sensor.transpose();

	/* populate caches for when computing scan point jacobians */
	RtsRl2s = R_ts * this->sensor_calib.R;
	this->RzRydRxRtsRl2s = R_dr * RtsRl2s;
	this->RzdRyRxRtsRl2s = R_dp * RtsRl2s;
	this->dRzRyRxRtsRl2s = R_dy * RtsRl2s;
	this->RzRyRxRtspRl2s = R_s2w * R_ts_p * this->sensor_calib.R;
	this->RzRyRxRtsRl2s  = R_s2w * RtsRl2s;
	this->RzRyRxRl2s     = R_s2w * this->sensor_calib.R;

	/* populate the input covariance matrix for the scan point
	 * distribution computation with the covariances between
	 * the sensor position's output distribution and the r,p,y
	 * of the system pose. */
	this->input_scanpoint_cov = scanpoint_cov_t::Zero();
	this->input_scanpoint_cov.block(0,0,3,3)
			= input_sensor_cov.block(0,0,3,3); /* r,p,y cov */

	/* populate the input covariance matrix for the scan point
	 * distribution computation with covariance values between
	 * the timestamp error and the output of the sensor position's
	 * distribution */
	this->input_scanpoint_cov.block(3,3,3,3) = this->output_sensor_cov;
	// TODO how does r,p,y correlate with sensor position */
	// TODO how does timestamp correlate with sensor position */

	/* update the input covariance matrix for the scan point
	 * with the uncertainty with respect to the timestamp.
	 * This should be the variance of the timestamp, which is
	 * the square of the standard deviation. */
	this->input_scanpoint_cov(9,9) = this->ts_std * this->ts_std;

	/* success */
	return 0;
}

void scan_model_t::set_point(const noisy_scanpoint_t& p)
{
	Vector3d T_p2l; /* mean position of scan point in sensor coords */
	
	/* The jacobian computed for the scanpoint transform
	 * function.  This matrix is useful for estimating the
	 * gaussian parameters of the output distribution for
	 * the scan point position. */
	scanpoint_jacobian_t J_scanpoint;

	/* --- begin computation --- */
	
	/* generate covariance matrix and mean for position of scan 
	 * point in sensor's frame of reference */
	T_p2l = p.get_mean();

	/* fully populate input covariance matrix for the input
	 * random variables used to compute scanpoint position */
	this->input_scanpoint_cov.block(6,6,3,3) = p.get_cov();
	cout << "point's original cov: " << endl << p.get_cov() << endl;
	cout << "point's input cov:" << endl << this->input_scanpoint_cov
	     << endl << endl << endl;

	/* populate the jacobian of the scanpoint's transformation
	 * with computed values, using the cached values that were
	 * populated during the call to set_frame()
	 *
	 * This jacobian is evaluated at the mean of the inputs */
	J_scanpoint.block(0,0,3,1) = this->RzRydRxRtsRl2s * T_p2l;
	J_scanpoint.block(0,1,3,1) = this->RzdRyRxRtsRl2s * T_p2l;
	J_scanpoint.block(0,2,3,1) = this->dRzRyRxRtsRl2s * T_p2l;
	J_scanpoint.block(0,3,3,3) = Matrix3d::Identity(); /* always */
	J_scanpoint.block(0,6,3,3) = this->RzRyRxRtsRl2s;
	J_scanpoint.block(0,9,3,1) = this->RzRyRxRtspRl2s * T_p2l;

	/* the mean of the output distribution should be the deterministic
	 * position of the scan point in world coordinates, since all
	 * input errors are assumed to be zero-mean */
	this->output_scanpoint_mean = this->RzRyRxRl2s * T_p2l
	                              + this->output_sensor_mean;
	
	/* compute the output distribution for the scan point position */
	this->output_scanpoint_cov = J_scanpoint
		* this->input_scanpoint_cov * J_scanpoint.transpose(); 
}

/*---------------------*/
/* debugging functions */
/*---------------------*/

void scan_model_t::serialize(ostream& out) const
{
	unsigned int r, c;

	/* print out sensor position mean */
	for(r = 0; r < 3; r++)
		out << this->output_sensor_mean(r) << " ";

	/* print out sensor covariance */
	for(r = 0; r < 3; r++)
		for(c = 0; c < 3; c++)
			out << this->output_sensor_cov(r,c) << " ";
	out << endl;

	/* print out scan point position mean */
	for(r = 0; r < 3; r++)
		out << this->output_scanpoint_mean(r) << " ";

	/* print out sensor covariance */
	for(r = 0; r < 3; r++)
		for(c = 0; c < 3; c++)
			out << this->output_scanpoint_cov(r,c) << " ";
	out << endl;
}
