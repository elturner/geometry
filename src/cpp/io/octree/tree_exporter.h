#ifndef TREE_EXPORTER_H
#define TREE_EXPORTER_H

/**
 * @file tree_exporter.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains function definitions that are used
 * to export information stored in an octree_t to various
 * formats for visualization purposes.
 */

#include <geometry/octree/octree.h>
#include <string>

/**
 * This namespace houses functions used to export visualizations of octrees
 */
namespace tree_exporter
{
	/**
	 * Will export the center of each leaf node as a vertex in OBJ
	 *
	 * This function will generate a Wavefront OBJ file that contains
	 * vertices located at the center of each leaf node in the given
	 * tree.  These vertices will be colored based on the data
	 * contained in that node.
	 *
	 * @param filename    The path to the .obj file to write
	 * @param tree        The tree to export
	 *
	 * @return            Returns zero on success, non-zero on failure.
	 */
	int export_leafs_to_obj(const std::string& filename,
	                        const octree_t& tree);
}

#endif
