#ifndef NODE_BOUNDARY_H
#define NODE_BOUNDARY_H

/**
 * @file node_boundary.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Classes used to define boundary nodes in octrees
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to formulate the set of boundary
 * nodes in a given octree.  Boundary nodes are nodes that are labeled
 * interior, but are adjacent to exterior nodes.  The "is_interior()"
 * function of octdata objects is used to determine if the nodes
 * are interior or exterior.
 */

#include <geometry/octree/octtopo.h>
#include <mesh/partition/node_set.h>
#include <string>
#include <vector>

/**
 * The node_boundary_t class can compute the subset of nodes
 * that represent the boundary.
 *
 * Boundary nodes are interior nodes that are adjacent to the
 * exterior sections of the tree.
 */
class node_boundary_t
{
	/* parameters */
	private:

		/**
		 * The boundary nodes and their topology
		 *
		 * The node set is populated by interior nodes
		 * that border exterior nodes.  These are defined
		 * as the boundary nodes of the octree.
		 *
		 * this topology represents the connections for 
		 * just the boundary nodes of the octree. it is a subset
		 * of the topology for the whole set of leaf nodes.
		 */
		octtopo::octtopo_t boundary;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Generates a set of boundary nodes from an octree topology
		 *
		 * This call will populate this boundary object.
		 *
		 * @param topo   The octree topology to use to populate this
		 *               object.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate(const octtopo::octtopo_t& topo);

		/**
		 * Clears all info stored in this boundary.
		 *
		 * This will clear any stored boundary information.
		 * To repopulate the boundary, call populate().
		 */
		inline void clear()
		{ this->boundary.clear(); };

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Determines the nodes of the tree adjacent to the
		 * specified node that are also boundary nodes.
		 *
		 * Will find the neighboring boundary nodes of the
		 * given node, which is assumed to be a boundary node.
		 *
		 * Any existing data in the neighs container will be
		 * cleared.
		 *
		 * If the given node is not a boundary node,
		 * then the neighs container will remain empty.
		 *
		 * @param node     The node to analyze
		 * @param neighs   Where to store the neighboring 
		 *                 boundary nodes.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int get_boundary_neighbors(octnode_t* node,
				std::vector<octnode_t*>& neighs) const;

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Exports a Wavefront OBJ file representing the boundary
		 *
		 * Given an output file path, will export the faces
		 * of internal nodes (as measured by 
		 * octdata->is_interior()) that border with external
		 * nodes (or null nodes).  The output will be
		 * formatted as a wavefront .obj file.
		 *
		 * @param filename   The output file location
		 *
		 * @return     Returns zero on success, non-zero
		 *             on failure.
		 */
		int writeobj(const std::string& filename) const;
};

#endif
