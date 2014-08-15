#include "node_boundary.h"
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/partition/node_set.h>
#include <util/error_codes.h>
#include <vector>
#include <map>

/**
 * @file node_boundary.cpp
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

using namespace octtopo;
using namespace std;

/* function implementations */

int node_boundary_t::populate(const octtopo_t& topo)
{
	map<octnode_t*, octneighbors_t>::const_iterator it;
	vector<octnode_t*> all_neighs;
	size_t f, i, n;
	int ret;

	/* iterate through the nodes in this tree */
	for(it = topo.begin(); it != topo.end(); it++)
	{
		/* ignore exterior nodes */
		if(!(octtopo_t::node_is_interior(it->first)))
			continue;

		/* get all neighbors of this node */
		all_neighs.clear();
		for(f = 0; f < NUM_FACES_PER_CUBE; f++)
			it->second.get(all_cube_faces[f], all_neighs);

		/* check if any of this node's neighbors
		 * are exterior.  If so, then this is a 
		 * boundary node */
		n = all_neighs.size();
		for(i = 0; i < n; i++)
			if(!octtopo_t::node_is_interior(all_neighs[n]))
			{
				/* it is a boundary node, copy its
				 * structure to this object */
				ret = this->boundary.add(it->first,
							it->second);
				if(ret)
					return PROPEGATE_ERROR(-1, ret);

				/* don't need to search any 
				 * other neighbors */
				break;
			}
	}

	/* success */
	return 0;
}
		
int node_boundary_t::get_boundary_neighbors(octnode_t* node,
					vector<octnode_t*>& neighs) const
{
	octneighbors_t edges;
	size_t fi;
	int ret;

	/* clear any existing values in neighs container */
	neighs.clear();

	/* find the node in our structure */
	ret = this->boundary.get(node, edges);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* store all neighbors */
	for(fi = 0; fi < NUM_FACES_PER_CUBE; fi++)
		edges.get(all_cube_faces[fi], neighs);

	/* success */
	return 0;
}
