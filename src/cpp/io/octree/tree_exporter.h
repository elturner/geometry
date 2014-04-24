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

	/**
	 * Will export leaf nodes labeled as exterior to cubes in OBJ
	 *
	 * This function will generate a Wavefront OBJ file that
	 * contains cubes generated where the exterior-labeled leaf
	 * nodes of the given tree reside.
	 *
	 * @param filename   The path to the .obj file to write
	 * @param tree       The tree to export
	 *
	 * @return           Returns zero on success, non-zero on failure.
	 */
	int export_exterior_cubes_to_obj(const std::string& filename,
	                                 const octree_t& tree);

	/**
	 * Will export a text file with statistical information about leafs
	 *
	 * The text file will have one line for each leaf in the tree. Each
	 * line will contain two values, separated by whitespace, denoting
	 * the probability value of that leaf, and the uncertainty value
	 * of that leaf:
	 *
	 * 	p1 c1
	 * 	p2 c2
	 * 	p3 c3
	 * 	...
	 *
	 * @param filename   The path to the .txt file to write
	 * @param tree       The tree to export
	 *
	 * @return           Returns zero on success, non-zero on failure.
	 */
	int export_stats_to_txt(const std::string& filename,
	                        const octree_t& tree);
}

#endif
