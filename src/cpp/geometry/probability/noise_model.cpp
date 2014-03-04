#include "noise_model.h"
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/probability/noisy_timestamp.h>
#include <geometry/probability/noisy_scan.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <string>
#include <stdlib.h>

/**
 * @file noise_model.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the noise_model_t class, which models the
 * random distribution of scan points collected by the mobile system.
 *
 * This file requires Eigen.
 */

using namespace std;
using namespace Eigen;

/* function implementations */
		
noise_model_t::noise_model_t()
{
	/* all internal structures automatically constructed,
	 * just need to put default values for other parameters */
}
		
noise_model_t::~noise_model_t()
{
	/* all deconstructors automatically called */
}
		
int noise_model_t::set_path(const string& madfile,
                            const string& conffile)
{
	int ret;

	/* initialize the path */
	this->path.clear();
	
	/* read in 3D path info from file */
	ret = this->path.readmad(madfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret); /* unable to read mad */
	
	/* read in scanner transform information */
	ret = this->path.parse_hardware_config(conffile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
	return 0;
}
		
int noise_model_t::set_sensor(const string& sn)
{
	int ret;

	/* retrieve this sensor's transform from the path info */
	ret = this->path.get_extrinsics_for(this->sensor_calib, sn);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}
		
void noise_model_t::set_timestamp(double ts, double n)
{
	/* set clock parameters, which model noise around this timestamp */
	this->clock.set_timestamp(ts);
	this->clock.set_noise(n);
}
		
void noise_model_t::set_scan(const fss::point_t& p)
{
	/* set the scan's intrinsic randomness off of what was
	 * read in from the fss file */
	this->scan.set(p.x, p.y, p.z, p.stddev, p.width); 
}
		
int noise_model_t::generate_sample(Vector3d& sensor_pos,
                                   Vector3d& scan_pos) const
{
	pose_t system_pose;
	transform_t system2world, sensor_trans;
	double ts;
	int ret;

	/* concatenate all sources of randomness to generate a sample
	 * position for the initialized scan point */

	/* sample a timestamp to use */
	ts = this->clock.generate_sample();

	/* use this timestamp to interpolate a position along the
	 * path.  This is represented by a translation and rotation */
	ret = this->path.compute_pose_at(system_pose, ts);
	if(ret)
		return PROPEGATE_ERROR(-2, ret); /* can't find pose */

	/* system -> world */
	system2world.T = system_pose.T;
	system2world.R = system_pose.R.toRotationMatrix();

	/* The system pose gives us:  system -> world
	 * we want:  sensor -> world */
	sensor_trans = this->sensor_calib; /* sensor -> system */
	sensor_trans.cat(system2world); /* sensor -> system
	                                 *    + system -> world */

	/* this represents our random sample of the sensor
	 * position in world coordinates, so save it to the provided
	 * Eigen structure */
	sensor_pos = sensor_trans.T;

	/* from here, get a random sample of the scan point */
	scan_pos = this->scan.generate_sample();

	/* transform this point from sensor coordinates to world coordinates
	 * by using the sensor pos transform */
	sensor_trans.apply(scan_pos);

	/* success */
	return 0;
}
