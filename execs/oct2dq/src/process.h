#ifndef PROCESS_H
#define PROCESS_H

/**
 * @file   process.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This class contains all process data for the oct2dq program
 *
 * @section DESCRIPTION
 *
 * This class represents the processing pipeline for the oct2dq program.
 * It contains all necessary data products and has functions that
 * process these data products appropriately.
 */

#include "oct2dq_run_settings.h"
#include <geometry/octree/octree.h>
#include <geometry/quadtree/quadtree.h>
#include <geometry/quadtree/quaddata.h>
#include <geometry/transform.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/wall_sampling/wall_region_info.h>
#include <mesh/wall_sampling/horizontal_region_info.h>
#include <Eigen/Dense>
#include <vector>
#include <map>
#include <set>

/**
 * The process_t class contains all necessary data products
 * for this program.
 */
class process_t
{
	/* parameters */
	private:

		/* the carved geometry */
		octree_t tree;
		node_boundary_t boundary;
		planar_region_graph_t region_graph;

		/* the computed wall samples */

		/**
		 * The following list represents the details on regions
		 * that were selected to be representative of walls.
		 */
		std::vector<wall_region_info_t> walls;

		/**
		 * The following list represents the details on regions
		 * that were selected to be representative of floors.
		 */
		std::vector<horizontal_region_info_t> floors;

		/**
		 * The following list represents the details on regions
		 * that were selected to be representative of ceilings.
		 */
		std::vector<horizontal_region_info_t> ceilings;

		/**
		 * The following list of z-elevations represents
		 * the partitions between levels of the building.
		 *
		 * There are ( 1 + level_splits.size() ) levels 
		 * of the building, split by horizontal planes at the 
		 * elevations listed here.
		 *
		 * So, if a z-value is between ls[0] and ls[1], then
		 * it is in the 2nd level.  If its height is less than
		 * ls[0], then it is in the first (lowest) level.
		 *
		 * units:  meters
		 */
		std::vector<double> level_splits;

		/**
		 * This represents the generated wall samples for each level
		 *
		 * This list contains the wall sampling generated for each
		 * level of the scanned environment.  The list is length N,
		 * where N is the number of levels discovered in the
		 * environment.
		 *
		 * Each structure contains the wall samples generated
		 * from the planar regions.  Points along each face
		 * are added to the structure with varying weight.
		 */
		std::vector<quadtree_t> sampling;

		/**
		 * The mapping between the generated wall samples
		 * and the originating wall regions.  The walls
		 * are represented by their index in the "walls"
		 * list.
		 *
		 * Each quaddata maps to the list of walls that
		 * contributed to that data sample.
		 *
		 * This map contains samples across all levels in
		 * the environment.
		 */
		std::map<quaddata_t*, std::set<size_t> > ws_to_walls;

	/* functions */
	public:

		/**
		 * Initializes the data given the input files
		 *
		 * Note, the input arguments may be modified to ensure
		 * that they represent valid values.
		 *
		 * @param args  The parsed program arguments
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int init(oct2dq_run_settings_t& args);

		/**
		 * Analyzes the planar regions to find representative
		 * floors, walls, and ceilings.
		 *
		 * This step takes the planar regions formed by calling
		 * init(), and identifies floors, walls, and ceilings.
		 * This step is necessary before calling
		 * compute_wall_samples(), which uses the found surfaces
		 * to form 2D wall sampling.
		 *
		 * @param args   The parsed program arguments
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int identify_surfaces(const oct2dq_run_settings_t& args);

		/**
		 * Uses the identified surfaces computed in 
		 * identify_surfaces() to estimate the bounds on
		 * each level scanned.
		 *
		 * This computation will estimate how many levels of a 
		 * building were scanned, and produce a partitioning
		 * of these levels.
		 *
		 * @param args    The parsed program arguments
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int compute_level_splits(const oct2dq_run_settings_t& args);

		/**
		 * Computes the locations and strengths of wall samples
		 *
		 * This function will analyze the planar regions computed
		 * for a model, and will find a set of wall samples from
		 * these regions.
		 *
		 * @param args   The parsed program arguments
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int compute_wall_samples(const oct2dq_run_settings_t& args);

		/**
		 * Computes pose indices for the wall samples
		 *
		 * Will use the provided fss files to perform ray
		 * tracing in the octree, in order to determine which
		 * poses saw which wall samples.  This information is
		 * recorded in the wall samples.
		 *
		 * This function should be called after 
		 * compute_wall_samples() but before export_data().
		 *
		 * @param args   The parsed program arguments
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int compute_pose_inds(const oct2dq_run_settings_t& args);

		/**
		 * Exports all data products
		 *
		 * @param args   The parsed program arguments
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int export_data(const oct2dq_run_settings_t& args) const;

	/* helper functions */
	private:

		/**
		 * Computes the 'strength' value for a given region
		 *
		 * The strength value indicates how likely the region
		 * is to be used as a source of wall samples.  A
		 * region that is larger, flatter, and more vertically-
		 * aligned should have a higher strength value.
		 *
		 * @param it    Iterator to the region info to analyze
		 * @param args  The parsed input arguments
		 *
		 * @return      The strength value for this region
		 */
		double compute_region_strength(
				regionmap_t::const_iterator it,
				const oct2dq_run_settings_t& args) const;

		/**
		 * Analyzes the given scan to determine if the current
		 * pose should be associated with wall samples
		 *
		 * @param pose            The pose to analyze
		 * @param pose_ind        The index of the pose to analyze
		 * @param point_pos_orig  The world-coordinates of the 
		 *                        scan point
		 * @param pose_choice_counts    Scorings for how often each
		 *                              wall sample gets chosen for
		 *                              a pose
		 * @param args            The parsed input arguments
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int analyze_scan(const transform_t& pose, size_t pose_ind,
				const Eigen::Vector3d& point_pos_orig,
				std::map<quaddata_t*, 
				std::pair<size_t, size_t> >& 
				pose_choice_counts,
				const oct2dq_run_settings_t& args);

		/**
		 * Returns true iff the two given wall samples share
		 * an originating wall.
		 *
		 * @param a   The first wall sample
		 * @param b   The second wall sample
		 *
		 * @return    Returns true iff the wall samples share a wall
		 */
		bool shares_a_wall(quaddata_t* a, quaddata_t* b) const;

		/**
		 * Returns the level index of the building level
		 * that contains the specified elevation.
		 *
		 * Will search through the building level splits to
		 * find the level index corresponding to the specified
		 * z-value.
		 *
		 * z is assumed to be in units of meters.
		 *
		 * @param z   The elevation to check
		 *
		 * @return    The level index of the specified elevation.
		 */
		size_t level_of_elevation(double z) const;
};

#endif
