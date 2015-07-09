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

#include <geometry/system_path.h>
#include <geometry/octree/octree.h>

/**
 * The door_finder_t class performs analysis on the model to estimate doors
 */
class door_finder_t
{
	/* parameters */
	private:

	/* functions */
	public:

		/**
		 * Given an octree model and a path, will estimate
		 * the location of doors
		 *
		 * @param tree   The octree model
		 * @param path   The path to traverse
		 *
		 * @return       Returns zero on success, non-zero on
		 *               failure.
		 */
		int analyze(octree_t& tree, const system_path_t& path);
};

#endif
