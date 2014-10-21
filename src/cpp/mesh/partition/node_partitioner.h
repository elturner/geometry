#ifndef NODE_PARTITIONER_H
#define NODE_PARTITIONER_H

/**
 * @file   node_partitioner.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Partitions octree volume into separate objects
 *
 * @section DESCRIPTION
 *
 * The classes defined in this file are used to analyze the nodes of
 * an existing octree.  This set of nodes will be partitioned into
 * individual objects, as well as nodes that represent the permenant
 * building features.
 *
 * Objects in the environment will be divided based on connectivity, in
 * attempt to separate each object from the others.
 *
 * Created July 7th, 2014
 */

#include "node_set.h"
#include <geometry/octree/octtopo.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <string>
#include <vector>

/**
 * The node_partitioner_t class will find a set of objects from an octree
 */
class node_partitioner_t
{
	/* parameters */
	private:

		/**
		 * This list of node sets indicates the partitions formed
		 */
		std::vector<node_set_t> partitions;

	/* functions */
	public:

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Given an octree's topology, partitions object node groups
		 *
		 * This function call will remove any existing data
		 * stored in this struct.  It will (re)populate the struct's
		 * parameters based on the given topology.  Object nodes
		 * in this octree will be partitioned using the union-find
		 * algorithm based on connectivity described in the
		 * octtopo_t value given.
		 *
		 * @param topo The octree's topology
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int partition(const octtopo::octtopo_t& topo);

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Exports the partitioned nodes to a set of OBJ files
		 *
		 * Will write out a list of Wavefront OBJ files, that
		 * are named based on the given prefix.  Each file will
		 * represented a single object detected in the partitioning
		 * of the given tree.
		 *
		 * @param prefix   The naming prefix to use for the output
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writeobjs(const std::string& prefix) const;
};

#endif
