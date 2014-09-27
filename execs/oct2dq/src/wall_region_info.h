#ifndef WALL_REGION_INFO_H
#define WALL_REGION_INFO_H

/**
 * @file   wall_region_info.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  wall_region_info_t contains useful parameters for wall regions
 *
 * @section DESCRIPTION
 *
 * This file contains the wall_region_info_t class, which is used
 * to house information computed about planar regions that are considered
 * to be representations of walls in the environment.
 */

#include <geometry/shapes/plane.h>
#include <mesh/surface/planar_region.h>
#include <Eigen/Dense>

/**
 * The wall_region_info_t class houses information that is relevent
 * for regions that are considered to be representations of walls
 * in the environment.
 */
class wall_region_info_t
{
	/* parameters */
	public:

		/**
		 * We want to record the region's normal
		 *
		 * Note that this is not the true normal of
		 * the region, but the 'vertically-aligned' normal.
		 * Which means this vector points within the x-y plane,
		 * since it represents a plane that is vertically-aligned.
		 */
		plane_t vertical;

		/**
		 * The following vectors represent basis coordinates
		 * for points along the plane of the region.
		 */
		Eigen::Vector3d a,b;

		/**
		 * The following represent a bounding box for the
		 * region, in the coordinate system defined by (a,b)
		 */
		double a_min, b_min, a_max, b_max;

		/**
		 * This value is the strength of the region
		 *
		 * Stronger means more wall-like.
		 */
		double strength;

	/* functions */
	public:

		/**
		 * Populates the information in this info struct
		 *
		 * @param s     The strength to use
		 * @param reg   The region to analyze
		 */
		void init(double s, const planar_region_t& reg);
};

#endif
