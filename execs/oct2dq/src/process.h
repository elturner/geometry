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
#include <mesh/surface/planar_region_graph.h>

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
		planar_region_graph_t region_graph;

	/* functions */
	public:

		/**
		 * Initializes the data given the input files
		 *
		 * @param args  The parsed program arguments
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int init(const oct2dq_run_settings_t& args);
};

#endif 
