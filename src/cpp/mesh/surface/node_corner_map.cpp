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
			
void corner_info_t::add(const faceset_t& fs)
{ 
	this->faces.insert(fs.begin(), fs.end());
}
			
void corner_info_t::add(const node_face_t& f)
{
	/* add the face */
	this->faces.insert(f);
}
			
void corner_info_t::writeobj_edges(std::ostream& os,
				const octree_t& tree,
				const Eigen::Vector3d& mypos) const
{
	cornerset_t::const_iterator eit;
	Vector3d p, off;
	
	/* compute a position offset from mypos */
	off << 0.003,0.003,0.003;
	off += mypos;

	/* iterate over the edges connected to this corner */
	for(eit = this->edges.begin(); eit != this->edges.end(); eit++)
	{
		/* print out the edge defined between the corners
		 * of this and eit */
		eit->get_position(tree, p);
		os << "v " << p.transpose() << " 0 255 0" << endl;
		os << "v " << mypos.transpose() << " 255 0 0" << endl;
		os << "v " << off.transpose() << " 255 255 255" << endl;

		/* draw a triangle between these corners */
		os << "f -1 -2 -3" << endl;
	}
}

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
	corner_t c;
	corner_t min_c, max_c;
	faceset_t::const_iterator fit;
	double hw;
	size_t ci;

	/* iterate over the corners of this face */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* construct this corner */ 
		c.set(tree, f, ci);
		
		/* add to this map */
		ins = this->corners.insert(pair<corner_t, 
				corner_info_t>(c, corner_info_t()));

		/* Either the corner was just added, or it was already
		 * present.  In either case, add this face to the info
		 * for this corner. */
		ins.first->second.add(f);

		/* record the min and max observed */
		if(ci == 0)
			min_c = max_c = c;
		else
			c.update_bounds(min_c, max_c);
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
			c.set(tree, *fit, ci);

			/* check if this corner is in bounds of
			 * the current face */
			if(!(c.within_bounds(min_c, max_c)))
				continue; /* don't care about this corner */
				
			/* add this face to the neighbor's corner */
			ins = this->corners.insert(
					pair<corner_t, corner_info_t>(
					c, corner_info_t()));
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

int corner_map_t::populate_edges(const octree_t& tree)
{
	ccmap_t::iterator cit, eit, eit_best;
	faceset_t::iterator fit;
	corner_t e;
	size_t ci;

	/* iterate over every corner stored in this map */
	for(cit = this->corners.begin(); cit != this->corners.end(); cit++)
	{
		/* We want to find every edge that attaches to this corner.
		 * In order to do this, we can iterate over the faces that
		 * adjoin this corner, and search the edges of these faces.
		 *
		 * We need to use the neighboring
		 * faces in order to make sure that each edge we add 
		 * is the minimal edge (and doesn't get overlayed by 
		 * another, smaller edge). 
		 */
		for(fit = cit->second.begin_faces(); 
				fit != cit->second.end_faces(); fit++)
		{
			/* iterate over the corners of the current face */
			for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
			{
				/* get position of ci'th corner */
				e.set(tree, *fit, ci);

				/* we only care about corners that are
				 * along a manhattan edge with *cit.  We
				 * can test this by looking at the hamming
				 * distance between the two corners.  If
				 * the hamming distance is zero, then they
				 * are the same corner.  If the hamming
				 * distance is greater than one, then the
				 * corners differ by more than one axis,
				 * and we don't care about it. */
				if(cit->first.hamming_dist(e) != 1)
					continue;
				
				/* get this corner in our map.  Note, this
				 * corner SHOULD be in our map, since the
				 * face is in our map */
				eit_best = this->corners.find(e);
				if(eit_best == this->corners.end())
					return -1;

				/* Check that they form minimal edges by 
				 * iterating down the edge.  We can do 
				 * this because corners are stored via 
				 * indices in each dimension.
				 *
				 * For each discrete position along the 
				 * edge, check if that position is stored
				 * as a corner in this map. If so, then 
				 * that corner must touch this face and so
				 * the one closest to cit must form an edge
				 * with cit.
				 */
				for(e.increment_towards(cit->first); 
					e != cit->first;
					e.increment_towards(cit->first))
				{
					/* check if e exists in this map */
					eit = this->corners.find(e);
					if(eit == this->corners.end())
						continue;

					/* if got here, then we found a 
					 * valid corner along the path of 
					 * e that's closer than the original
					 * value of e, so we want to use 
					 * this one instead */
					eit_best = eit;
				}

				/* we want to assign an edge between *cit
				 * and *eit_best */
				cit->second.add(eit_best->first);
				eit_best->second.add(cit->first);
			}
		}
	}

	/* success */
	return 0;
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
			
pair<cornerset_t::const_iterator, cornerset_t::const_iterator>
			corner_map_t::get_edges_for(const corner_t& c) const
{
	pair<cornerset_t::const_iterator, cornerset_t::const_iterator> p;
	ccmap_t::const_iterator it;

	/* look up this corner */
	it = this->corners.find(c);
	if(it == this->corners.end())
		return p;

	/* get the edges for this corner */
	p.first  = it->second.edges.begin();
	p.second = it->second.edges.end();

	/* success */
	return p;
}
			
void corner_map_t::writeobj_edges(std::ostream& os, 
				const octree_t& tree) const
{
	ccmap_t::const_iterator cit;
	Vector3d mypos;

	/* iterate over the corners in this map */
	for(cit = this->corners.begin(); cit != this->corners.end(); cit++)
	{
		/* compute the position of this corner */
		cit->first.get_position(tree, mypos);

		/* export its edges */
		cit->second.writeobj_edges(os, tree, mypos);
	}
}
			
void corner_map_t::writeobj_edges(std::ostream& os, const octree_t& tree,
					const corner_t& c) const
{
	ccmap_t::const_iterator cit;
	Vector3d mypos;

	/* find this corner */
	cit = this->corners.find(c);
	if(cit == this->corners.end())
		return;
		
	/* compute the position of this corner */
	cit->first.get_position(tree, mypos);

	/* export its edges */
	cit->second.writeobj_edges(os, tree, mypos);
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
