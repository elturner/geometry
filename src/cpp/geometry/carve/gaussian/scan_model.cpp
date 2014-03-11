#include "scan_model.h"
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <geometry/carve/gaussian/carve_map.h>
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
	
	/* cached matricies don't need initing */
	
	/* output distributions */
	this->output_sensor_mean    = Vector3d::Zero();
	this->output_sensor_cov     = Matrix3d::Zero();
	this->output_scanpoint_mean = Vector3d::Zero();
	this->output_scanpoint_cov  = Matrix3d::Zero();
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
	Vector3d w;
	Matrix3d input_C_pose, input_C_rpy;
	Matrix3d sensor_C_pose, sensor_C_rpy, sensor_C_ts;
	Matrix3d T_l2s_cross;
	int ret;
		
	/* compute the system pose for this timestamp */
	ret = path.compute_pose_at(this->pose, time);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* the following values are useful for caching */
	this->R_s2w = this->pose.R.toRotationMatrix();
	this->R_s2w_t = this->R_s2w.transpose();
	this->R_l2w = this->R_s2w * this->sensor_calib.R;
	this->R_l2w_t = this->R_l2w.transpose();

	/* the mean of the output distribution should be the deterministic
	 * position of the sensor in world coordinates, since all
	 * input errors are assumed to be zero-mean */
	this->output_sensor_mean = (this->R_s2w * this->sensor_calib.T) 
	                           + this->pose.T; /* deterministic pos */

	/* precompute matrix representations of angular velocity */
	w = this->ts_std * this->pose.w;
	this->twwt = w * w.transpose();
	
	/* get noise distribution of input data */
	input_C_pose = 0.0001*Matrix3d::Identity(); // TODO system xyz cov
	input_C_rpy = 0.017*Matrix3d::Identity(); // TODO system rpy cov

	/* compute intermediary terms */
	T_l2s_cross << 0, -this->sensor_calib.T(2), this->sensor_calib.T(1),
	               this->sensor_calib.T(2), 0, -this->sensor_calib.T(0),
		       -this->sensor_calib.T(1), this->sensor_calib.T(0), 0;

	/* compute covariances caused by independent noise sources */
	sensor_C_pose = input_C_pose; /* it's just a translation */
	sensor_C_rpy  = T_l2s_cross * input_C_rpy * T_l2s_cross.transpose();
	sensor_C_ts   = T_l2s_cross * this->twwt  * T_l2s_cross.transpose();
	this->output_sensor_cov = sensor_C_pose+sensor_C_rpy+sensor_C_ts;
	
	/* success */
	return 0;
}

void scan_model_t::set_point(const noisy_scanpoint_t& p)
{
	Vector3d T_p2l; /* mean position of scan point in sensor coords */
	Vector3d T_p2s; /* position of point to system */
	Matrix3d T_p2s_cross; /* cross-product matrix of T_p2s */
	Matrix3d M;
	Matrix3d C_noise; /* internal noise of sensor, in world coords */
	Matrix3d C_ts; /* noise of scan point pos, due to timestamp err */

	/* --- begin computation --- */
	
	/* generate covariance matrix and mean for position of scan 
	 * point in sensor's frame of reference */
	T_p2l = p.get_mean();

	/* the mean of the output distribution should be the deterministic
	 * position of the scan point in world coordinates, since all
	 * input errors are assumed to be zero-mean */
	this->output_scanpoint_mean = this->R_l2w * T_p2l
	                              + this->output_sensor_mean;

	/* compute intermediary values */
	T_p2s = T_p2l;
	this->sensor_calib.apply(T_p2s); /* point in system coords */
	T_p2s_cross << 0, -T_p2s(2), T_p2s(1),
	               T_p2s(2), 0, -T_p2s(0),
		       -T_p2s(1), T_p2s(0), 0; /* cross-product of T_p2s */
	M = R_s2w * T_p2s_cross;

	/* compute covariances caused by individual noise sources.  All
	 * these sources are assumed to be independent */
	C_noise = this->R_l2w * p.get_cov() * this->R_l2w_t; 
	C_ts = M * this->twwt * M.transpose();

	/* compute the output distribution for the scan point position */
	this->output_scanpoint_cov = this->output_sensor_cov+C_ts+C_noise;
}
		
void scan_model_t::populate(carve_map_t& cm) const
{
	/* initialize the given carve map with the output of this
	 * structure's computation */
	cm.init(this->output_sensor_mean,    this->output_sensor_cov,
	        this->output_scanpoint_mean, this->output_scanpoint_cov);
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
