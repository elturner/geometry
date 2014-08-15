#include "node_boundary.h"
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/partition/node_set.h>
#include <util/error_codes.h>
#include <string>
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
	vector<octnode_t*> neighs;
	size_t f, i, n;
	bool found_exterior;
	int ret;

	/* iterate through the nodes in this tree */
	for(it = topo.begin(); it != topo.end(); it++)
	{
		/* ignore exterior nodes */
		if(!(octtopo_t::node_is_interior(it->first)))
			continue;

		/* check all neighbors of this node */
		found_exterior = false;
		for(f = 0; f < NUM_FACES_PER_CUBE && !found_exterior; f++)
		{
			/* get the neighbors for this face */
			neighs.clear();
			it->second.get(all_cube_faces[f], neighs);

			/* check if facing some null space */
			if(neighs.empty())
			{
				/* if a face has no neighbors,
				 * then this node is next to null
				 * space, which is exterior */
				found_exterior = true;
				break;
			}

			/* check if any of this node's neighbors
			 * are exterior.  If so, then this is a 
			 * boundary node.
			 *
			 * Alternatively, if this node has no neighbors
			 * at all, then is also a boundary, since null
			 * space is assumed to be exterior. */
			n = neighs.size();
			for(i = 0; i < n && !found_exterior; i++)
				if(!octtopo_t::node_is_interior(neighs[i]))
					found_exterior = true;
		}
	
		/* check if current node is a boundary */
		if(found_exterior)
		{
			/* it is a boundary node, copy its
			 * structure to this object */
			ret = this->boundary.add(it->first,
					it->second);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
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
		
int node_boundary_t::writeobj(const string& filename) const
{
	int ret;

	/* use the stored topology to write the file */
	ret = this->boundary.writeobj(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}
