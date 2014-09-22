#include "node_corner.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <Eigen/Dense>
#include <iostream>
#include <set>

/**
 * @file    node_corner.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Analyzes attributes of the corners of nodes in octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the node_corner::corner_t class, which describe 
 * the attributes of the octree at node corners.
 *
 * Note that the octree natively stores data at the center of nodes,
 * and in order to understand the value of the interpolated data at
 * the corners, you need this class.
 */

using namespace std;
using namespace Eigen;
using namespace node_corner;

/* the following constants are used for processing */

#define APPROX_ZERO  0.000000001

/*--------------------------*/
/* function implementations */
/*--------------------------*/
			
void corner_t::set(const octree_t& tree, octnode_t* n, size_t ind)
{
	std::set<octnode_t*> neighs;

	/* make call to other overloaded function */
	this->set(tree, n, ind, neighs);
}

void corner_t::set(const octree_t& tree, octnode_t* n, size_t ind,
					std::set<octnode_t*> neighs)
{
	Vector3d p, disp;
	double hr, r;
	octnode_t* nn;
	size_t i;

	/* clear neighbor information */
	neighs.clear();
	neighs.insert(n);

	/* get the position of the corner in question */
	p = get_corner_pos(n, ind);

	/* sample around this position to find the nodes surrounding
	 * the corner. */
	r = tree.get_resolution();
	hr = r / 2.0; /* all nodes are larger than this, so it can be
	               * treated as an epsilon distance */
	for(i = 0; i < NUM_CORNERS_PER_CUBE; i++)
	{
		/* get a position slightly
		 * offset from the corner */
		disp = relative_child_pos(i); /* position of i'th
				corner for a node */
		disp = p - hr*disp; /* a point inside a node from its
					    i'th corner */

		/* determine the node
		 * that contains this point */
		nn = tree.get_root()->retrieve(disp);
		if(nn == NULL)
			continue; /* don't use this */

		/* save the node */
		neighs.insert(nn);

		/* don't bother with this node if it is bigger than
		 * the one we already had */
		if(nn->halfwidth > n->halfwidth + hr)
			continue;

		/* check if this is a better node */
		if(nn->halfwidth + hr < n->halfwidth)
		{
			/* use this node instead */
			n = nn;
			ind = i;
		}
		else
		{
			/* if got here, then the nodes are
			 * the same size.  Choose the one with
			 * the smaller corner index */
			if(i < ind)
			{
				/* choose this node */
				n = nn;
				ind = i;
			}
		}
	}
	
	/* we've chosen the best node/index pair to represent
	 * this corner */
	this->node = n;
	this->index = ind;
}
			
bool corner_t::is_contained_in(const octnode_t* n) const
{
	Vector3d p;
	double d;

	/* check trivial case */
	if(this->node == n)
		return true;

	/* now we actually have to do some math */
	p = this->get_position();
	
	/* check that p is within cube around center of this node,
	 * with some accounting for rounding error */
	d = (p - n->center).cwiseAbs().maxCoeff();
	return (d <= n->halfwidth + APPROX_ZERO);
}
			
void corner_t::writeobj(std::ostream& os) const
{
	/* export point at corner position */
	os << "v " << this->get_position().transpose();
	
	/* colored based on the index */
	switch(this->index)
	{
		case 0:
			os << " 255 255 255" << endl;
			break;
		case 1:
			os << " 255 255 0" << endl;
			break;
		case 2:
			os << " 0 0 255" << endl;
			break;
		case 3:
			os << " 255 0 0" << endl;
			break;
		case 4:
			os << " 255 0 255" << endl;
			break;
		case 5:
			os << " 255 100 0" << endl;
			break;
		case 6:
			os << " 0 255 0" << endl;
			break;
		case 7:
			os << " 255 100 255" << endl;
			break;
		default:
			os << " 0 0 0" << endl;
			break;
	}
}
