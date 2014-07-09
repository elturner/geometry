#ifndef UNION_FIND_H
#define UNION_FIND_H

/**
 * @file   union_find.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Classes to perform the union-find algorithm
 *
 * @section DESCRIPTION
 *
 * This file contains the union_find_t class, which is used to 
 * perform the union-find algorithm.  This algorithm is used to 
 * identify the set of connected components in a graph.
 *
 * This library is used by specifying the number of nodes in a graph,
 * and the edge connections between each node.
 */

#include <cstddef>
#include <vector>

/**
 * The union_find_t class will perform union-find on arbitrary graphs
 */
class union_find_t
{
	/* parameters */
	private:

		/**
		 * This represents the connectivity of the graph so far
		 */
		std::vector<size_t> forest;

	/* functions */
	public:

		/**
		 * Initialize the union-find object
		 *
		 * This call will remove any existing data.  By specifying
		 * the number of nodes in the graph to analyze, this call
		 * will initialize a new forest with no known edges.
		 *
		 * This function should be called once per graph.
		 *
		 * @param N   The number of nodes in the desired graph
		 */
		void init(size_t N);

		/**
		 * Will incorporate edge (a,b) into the represented graph
		 *
		 * Will add the edge described by the given two node
		 * indices into the represented graph.  This function
		 * should be called on each edge of the graph.
		 *
		 * @param a   The first node index of represented edge
		 * @param b   The second node index of represented edge
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int add_edge(size_t a, size_t b);

		/**
		 * Retrieves a list of all unions in the graph
		 *
		 * After all edges have been added, calling this function
		 * will populate the specified vector with all unions
		 * in the represented graph.
		 *
		 * Each element in the given vector is a list of indices
		 * that represent one union.  The total length of the
		 * given vector is the total number of unions in the graph.
		 *
		 * Any contents of the unions vector before this call will
		 * be discarded.
		 *
		 * @param unions   Where to store the lists of union indices
		 */
		void get_unions(std::vector<std::vector<size_t> >& unions);

	/* helper functions */
	private:

		/**
		 * Gets the union root for union-find operations
		 *
		 * This will traverse up the tree described by
		 * the given vector of indices, until a reflexive
		 * index is found.
		 *
		 * This function is recursive.
		 *
		 * @param i       The starting index
		 *
		 * @return        Will return the root index of i
		 */
		int get_root(size_t i);
};

#endif
