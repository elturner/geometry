#include "wall_region_info.h"
#include <geometry/shapes/plane.h>
#include <mesh/surface/planar_region.h>
#include <Eigen/Dense>

/**
 * @file   wall_region_info.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  wall_region_info_t contains useful parameters for wall regions
 *
 * @section DESCRIPTION
 *
 * This file contains the wall_region_info_t class, which is used
 * to house information computed about planar regions that are considered
 * to be representations of walls in the environment.
 */

using namespace std;
using namespace Eigen;

/* function implementation */

void wall_region_info_t::init(double s, const planar_region_t& reg)
{
	/* set the strength */
	this->strength = s;

	/* get coordinate frame along this planar region */
	this->b << 0,0,1;
	const Vector3d& n 
		= reg.get_plane().normal;
	this->a = this->b.cross(n).normalized(); 
				/* most-horizontal coord */
	this->b = n.cross(this->a); 
				/* most-vertical coord */

	/* get a version of the region plane that's perfectly
	 * vertical. */
	this->vertical = reg.get_plane();
		
	/* normal must be horizontal */
	this->vertical.normal(2) = 0; 
	this->vertical.normal.normalize();

	/* get bounding box of the planar region */
	reg.compute_bounding_box(this->a, this->b,
			this->a_min, this->a_max, 
			this->b_min, this->b_max);
}
