#include "scan_model.h"
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
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
		
scan_model_t::scan_model_t()
{
	// TODO
}

int scan_model_t::set_sensor(const string& sensor_name,
                             double timesync_err,
                             const system_path_t& path)
{
	// TODO
}

int scan_model_t::set_frame(double time, const system_path_t& path)
{
	// TODO
}

int scan_model_t::set_point(const fss::point_t& p)
{
	// TODO
}

void scan_model_t::serialize(ostream& out) const
{
	// TODO
}
