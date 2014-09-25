#include "node_corner_map.h"
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner.h>
#include <Eigen/Dense>
#include <set>
#include <map>

/**
 * @file    node_corner_map.cpp
 * @author  Eic Turner <elturner@eecs.berkeley.edu>
 * @brief   Defines the node_corner::corner_map_t class
 *
 * @section DESCRIPTION
 *
 * This file contains the classes used to map the properties
 * of node corners in a model.  It adds to the node_corner
 * namespace.
 */

using namespace std;
using namespace Eigen;
using namespace node_corner;

/*----------------------------------------*/
/* corner_info_t function implementations */
/*----------------------------------------*/

/*---------------------------------------*/
/* corner_map_t function implementations */
/*---------------------------------------*/

void corner_map_t::add(const octree_t& tree, octnode_t* n)
{
	pair<ccmap_t::iterator, bool> ins;
	corner_t corner;
	size_t ci;

	/* iterate over the corners of this node */
	for(ci = 0; ci < NUM_CORNERS_PER_CUBE; ci++)
	{
		/* construct this corner */ 
		corner.set(tree, n, ci);

		/* add to this map */
		ins = this->corners.insert(pair<corner_t, corner_info_t>(
					corner, corner_info_t()));

		/* Either the corner was just added, or it was already
		 * present.  In either case, add this node to the info
		 * for this corner. */
		ins.first->second.add(n);
	}
}

void corner_map_t::add_all(const octree_t& tree)
{
	/* use recursive helper function */
	this->add_all(tree, tree.get_root());
}
			
void corner_map_t::add(const octree_t& tree, const node_face_t& f)
{
	pair<ccmap_t::iterator, bool> ins;
	corner_t corner;
	size_t ci;

	/* iterate over the corners of this face */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* construct this corner */ 
		corner.set(tree, f, ci);

		/* add to this map */
		ins = this->corners.insert(pair<corner_t, corner_info_t>(
					corner, corner_info_t()));

		/* Either the corner was just added, or it was already
		 * present.  In either case, add this face to the info
		 * for this corner. */
		ins.first->second.add(f);
	}
}
			
void corner_map_t::add(const octree_t& tree, const node_face_t& f,
					const node_face_info_t& neighs)
{
	pair<ccmap_t::iterator, bool> ins;
	corner_t corner, min_c, max_c;
	faceset_t::const_iterator fit;
	double hw;
	size_t ci;

	/* iterate over the corners of this face */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* construct this corner */ 
		corner.set(tree, f, ci);

		/* add to this map */
		ins = this->corners.insert(pair<corner_t, corner_info_t>(
					corner, corner_info_t()));

		/* Either the corner was just added, or it was already
		 * present.  In either case, add this face to the info
		 * for this corner. */
		ins.first->second.add(f);

		/* record the min and max observed */
		if(ci == 0)
			min_c = max_c = corner;
		else
			corner.update_bounds(min_c, max_c);
	}

	/* iterate over the neighboring faces of f, searching
	 * for smaller faces */
	hw = f.get_halfwidth();
	for(fit = neighs.begin(); fit != neighs.end(); fit++)
	{
		/* check if the neighbor is smaller */
		if(hw <= fit->get_halfwidth())
			continue; /* don't care about this one */

		/* if the neighbor is smaller, then one of the
		 * neighbor's corners will occur along the edge
		 * of this face, and we want this face to be associated
		 * with that corner 
		 *
		 * So we need to iterate over the corners of the
		 * neighbor face, and determine if it is contained
		 * within the face f */
		for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
		{
			/* get this corner of the neighbor face */
			corner.set(tree, *fit, ci);

			/* check if this corner is in bounds of
			 * the current face */
			if(!(corner.within_bounds(min_c, max_c)))
				continue; /* don't care about this corner */
				
			/* add this face to the neighbor's corner */
			ins = this->corners.insert(
					pair<corner_t, corner_info_t>(
					corner, corner_info_t()));
			ins.first->second.add(f);
		}
	}
}
			
void corner_map_t::add(const octree_t& tree,
			const node_boundary_t& boundary)
{
	facemap_t::const_iterator it;

	/* iterate through the faces in this object */
	for(it = boundary.begin(); it != boundary.end(); it++)
		this->add(tree, it->first, it->second); /* add it */
}

pair<set<octnode_t*>::const_iterator, set<octnode_t*>::const_iterator>
			corner_map_t::get_nodes_for(const corner_t& c) const
{
	pair<set<octnode_t*>::const_iterator, 
		set<octnode_t*>::const_iterator> p;
	ccmap_t::const_iterator it;

	/* look up this corner */
	it = this->corners.find(c);
	if(it == this->corners.end())
		return p; /* corner is not in map */

	/* get the nodes for this corner */
	p.first = it->second.nodes.begin();
	p.second = it->second.nodes.end();

	/* success */
	return p;
}

pair<faceset_t::const_iterator, faceset_t::const_iterator>
			corner_map_t::get_faces_for(const corner_t& c) const
{
	pair<faceset_t::const_iterator, faceset_t::const_iterator> p;
	ccmap_t::const_iterator it;

	/* look up this corner */
	it = this->corners.find(c);
	if(it == this->corners.end())
		return p;

	/* get the faces for this corner */
	p.first = it->second.faces.begin();
	p.second = it->second.faces.end();

	/* success */
	return p;
}
			
void corner_map_t::add_all(const octree_t& tree, octnode_t* node)
{
	size_t i;

	/* check for null nodes */
	if(node == NULL)
		return; /* don't do anything */

	/* check if this is a leaf node */
	if(node->data != NULL || node->isleaf())
		this->add(tree, node);

	/* recurse through children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		this->add_all(tree, node->children[i]);
}
