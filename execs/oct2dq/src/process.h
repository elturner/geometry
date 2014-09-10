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
#include <geometry/system_path.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/wall_sampling/wall_sampling.h>
#include <map>

/**
 * The process_t class contains all necessary data products
 * for this program.
 */
class process_t
{
	/* parameters */
	private:

		/* the system path */
		system_path_t path;

		/* the carved geometry */
		octree_t tree;
		node_boundary_t boundary;
		planar_region_graph_t region_graph;

		/* the computed wall samples */

		/**
		 * This represents the generated wall samples
		 *
		 * This structure contains the wall samples generated
		 * from the planar regions.  Points along each face
		 * are added to the structure with varying weight.
		 */
		wall_sampling_t sampling;

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

};

#endif
