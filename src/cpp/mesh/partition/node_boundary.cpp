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
	node_face_t face;
	node_face_info_t faceinfo;
	multimap<octnode_t*, node_face_t> node_face_map;
	multimap<octnode_t*, node_face_t>::iterator nfit;
	size_t f, i, n;
	bool found_exterior;
	int ret;

	/* iterate through the nodes in this tree, storing
	 * the boundary nodes */
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

	/* now that we've stored the bounary nodes, we can determine the
	 * set of boundary faces.  Each boundary node can contribute one
	 * or more boundary faces. */
	for(it = this->boundary.begin(); it != this->boundary.end(); it++)
	{
		/* iterate over faces, looking for exterior neighbors */
		for(f = 0; f < NUM_FACES_PER_CUBE; f++)
		{
			/* get nodes neighboring on this face */
			neighs.clear();
			it->second.get(all_cube_faces[f], neighs);
			
			/* check for external nodes */
			n = neighs.size();
			for(i = 0; i < n; i++)
			{
				/* ignore if interior */
				if(octtopo_t::node_is_interior(neighs[i]))
					continue;

				/* neighbor marks exterior boundary, 
				 * so export intersection of faces */
				if(neighs[i]->halfwidth 
						< it->first->halfwidth)
				{
					/* the neighbor's face is smaller
					 * than this node's face, so we 
					 * want the boundary to be
					 * represented by the neighbor's
					 * face. */

					/* prepare face */
					face.node = neighs[i];
					face.f = octtopo::get_opposing_face(
						all_cube_faces[f]);
					faceinfo.init(face);
					
					/* we want the face to reference
					 * the interior side, even if it
					 * is bigger */
					face.node = it->first;
					face.f = all_cube_faces[f];
				}
				else
				{
					/* since this node's face is
					 * smaller than the neighbor's
					 * face, we want to use this
					 * face as the boundary
					 * representation */

					/* prepare face */
					face.node = it->first;
					face.f = octtopo::all_cube_faces[f];
					faceinfo.init(face);
				}
				
				/* insert face info into this
				 * structure */
				this->faces.insert(std::pair<node_face_t,
					node_face_info_t>(face,faceinfo));
				
				/* keep track of which node
				 * created this face */
				node_face_map.insert(
					std::pair<octnode_t*,node_face_t>(
						it->first, face));
			}

			/* check if it is an interior node against 
			 * null space */
			if(neighs.empty())
			{
				/* prepare face */
				face.node = it->first;
				face.f = octtopo::all_cube_faces[f];
				faceinfo.init(face);
				this->faces.insert(std::pair<node_face_t,
					node_face_info_t>(face,faceinfo));
				
				/* keep track of which node
				 * created this face */
				node_face_map.insert(
					std::pair<octnode_t*,node_face_t>(
						it->first, face));
			}
		}
	}

	/* now that we've fully populated the faces multimap, we want
	 * to populate the linkages between faces.  For that, we use
	 * the now-popualted node_face_map, and link any two faces if:
	 *
	 * 	- They share a node
	 * 	- They originate from nodes that are neighbors
	 */
	for(nfit=node_face_map.begin(); nfit!=node_face_map.end(); nfit++)
	{
		/* note that any face that is present through this iterator
		 * must already be a member of this->faces */
		
		// TODO
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
		
/*--------------------------------------*/
/* node_face_t function implementations */
/*--------------------------------------*/

void node_face_info_t::init(octnode_t* n, octtopo::CUBE_FACE ff)
{
	/* copy over parameters */
	this->halfwidth = n->halfwidth;
	
	/* determine position of face */
	switch(ff)
	{
		case FACE_ZMINUS:
			this->x = n->center(0);
			this->y = n->center(1);
			this->z = n->center(2) - this->halfwidth;
			break;
		case FACE_YMINUS:
			this->x = n->center(0);
			this->y = n->center(1) - this->halfwidth;
			this->z = n->center(2);
			break;
		case FACE_XMINUS:
			this->x = n->center(0) - this->halfwidth;
			this->y = n->center(1);
			this->z = n->center(2);
			break;
		case FACE_XPLUS:
			this->x = n->center(0) + this->halfwidth;
			this->y = n->center(1);
			this->z = n->center(2);
			break;
		case FACE_YPLUS:
			this->x = n->center(0);
			this->y = n->center(1) + this->halfwidth;
			this->z = n->center(2);
			break;
		case FACE_ZPLUS:
			this->x = n->center(0);
			this->y = n->center(1);
			this->z = n->center(2) + this->halfwidth;
			break;
	}

	/* clear neighbor list */
	this->neighbors.clear();
}
