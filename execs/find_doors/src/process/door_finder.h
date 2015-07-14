#ifndef DOOR_FINDER_H
#define DOOR_FINDER_H

/**
 * @file   door_finder.h
 * @author Eric Turner <elturner@indoorreality.com>
 * @brief  Class used to find doors in octree models
 *
 * @section DESCRIPTION
 *
 * Will take an octree and a path, and will estimate the positions
 * of doors in the model.
 */

#include "../structs/door.h"
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/hist/hia_analyzer.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <vector>
#include <string>

/**
 * The door_finder_t class performs analysis on the model to estimate doors
 */
class door_finder_t
{
	/* parameters */
	private:

		/* the minimum width of a door, in meters.
		 *
		 * This is used to help define a search area around
		 * detected doors.
		 */
		double door_min_width;

		/* the maximum width of a door, in meters.
		 *
		 * This is used to help refine geometry search area
		 * for doors.
		 */
		double door_max_width;

		/* the min allowed height for a detected door, in meters
		 */
		double door_min_height;

		/* the max height of a door, in meters.
		 *
		 * This is used as an initial condition for fitting
		 * door geometry.
		 */
		double door_max_height;

		/* the angular stepsize to search for door orientations
		 *
		 * units: radians
		 */
		double angle_stepsize;

		/**
		 * Positions for each door found
		 */
		std::vector<door_t> doors; 

	/* functions */
	public:

		/**
		 * Initializes the parameters used by this object
		 *
		 * @param minwidth   The minimum width of doors
		 * @param maxwidth   The maximum width of doors
		 * @param maxheight  The maximum height of doors
		 * @param anglestep  The angular stepsize (radians)
		 */
		inline void init(double minwidth, double maxwidth,
				double minheight, double maxheight, 
				double anglestep)
		{
			this->door_min_width  = minwidth;
			this->door_max_width  = maxwidth;
			this->door_min_height = minheight;
			this->door_max_height = maxheight;
			this->angle_stepsize  = anglestep;
			this->doors.clear();
		};

		/**
		 * Given an octree model and a path, will estimate
		 * the location of doors
		 *
		 * @param tree         The octree model
		 * @param hia          The hia data for the current level
		 * @param path         The path to traverse
		 *
		 * @return       Returns zero on success, non-zero on
		 *               failure.
		 */
		int analyze(octree_t& tree, 
				const hia_analyzer_t& hia,
				const system_path_t& path);

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports the detected door positions to a text file.
		 *
		 * Each line in the file is a 3D point that is an estimated
		 * location of a door.
		 *
		 * 		<x1> <y1> <z1>
		 * 		<x2> <y2> <z2>
		 * 		...
		 * 		<xn> <yn> <zn>
		 *
		 * Where n is the number of doors.
		 *
		 * @param txtfile   The path to the txt file to write
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int writetxt(const std::string& txtfile) const;

	/* helper functions */
	private:

		/**
		 * Determines position where line intersects its door.
		 *
		 * Given a line segment defined by two points that is
		 * assumed to cross across a door's threshold, will
		 * estimate the position of the that threshold.
		 *
		 * Will search along the line segment, and find the location
		 * of minimum height in the tree.  A door is assumed to be
		 * the most "solid" object intersected by the path.
		 *
		 * The segment will be extruded up and down to estimate
		 * the full height of a door.
		 *
		 * @param doorpos Where to store the estimated position of
		 *                where line AB intersects its door.
		 * @param tree    The octree to analyze
		 * @param A       The first point of the path segment
		 * @param B       The second point of the path segment
		 *
		 * @return        Returns zero on success, non-zero on
		 *                failure.
		 */
		int find_door_intersection(Eigen::Vector3d& doorpos,
				octree_t& tree,
				const Eigen::Vector3d& A,
				const Eigen::Vector3d& B) const;
		
		/**
		 * Remove duplicate door positions
		 *
		 * Will iterate over the doors found so far,
		 * and if two doors are close enough, will merge 
		 * them into a single position.
		 *
		 * NOTE:  assumes all doors are in the same level.
		 *
		 * @param octree        The octree containing the model
		 * @param floor_height  Height of floor for this level
		 * @param ceil_height   Height of ceiling for level
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int remove_duplicates(const octree_t& octree,
				double floor_height, double ceil_height);

		/**
		 * Estimates the geometry of a door given its
		 * center position.
		 *
		 * The detected geometry will be stored in each
		 * door structure.
		 *
		 * @param hia       The hia info for this level
		 * @param door_ind  The index of the door to analyze
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int estimate_door_geom(const hia_analyzer_t& hia, 
					size_t door_ind);
};

#endif
