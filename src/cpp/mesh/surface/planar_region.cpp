#include "planar_region.h"
#include <mesh/surface/node_boundary.h>
#include <geometry/shapes/plane.h>
#include <geometry/octree/octtopo.h>
#include <iostream>
#include <queue>
#include <set>
#include <stdlib.h>

/**
 * @file   planar_region.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines planar region class, made up of a set of node_face_t's
 * 
 * @section DESCRIPTION
 *
 * This file implements the planar_region_t class, which is used to cluster
 * node_face_t objects into large, planar regions.  This class gives
 * a representation of such regions, as well as a way to generate them
 * from a set of nodes.
 *
 * Note that this class operates on node_boundary_t objects, which should
 * already be populated from an octtopo_t topology.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void planar_region_t::floodfill(const node_face_t& seed,
				const node_boundary_t& boundary,
				faceset& blacklist)
{
	queue<node_face_t> to_check;
	pair<faceset::const_iterator, faceset::const_iterator> range;
	faceset::const_iterator it;

	/* clear any existing information for this region */
	this->clear();

	/* prepare plane geometry based on seed face */
	seed.get_center(this->plane.point);
	octtopo::cube_face_normals(seed.direction, this->plane.normal);

	/* check everything in the queue until we run out */
	for(to_check.push(seed); !(to_check.empty()); to_check.pop())
	{
		/* check if next value is on blacklist */
		if(blacklist.count(to_check.front()))
			continue; /* can't use this one */

		/* check that face is direction in same orientation */
		if(to_check.front().direction != seed.direction)
			continue;

		/* add to our region and to the blacklist */
		this->add(to_check.front());
		blacklist.insert(to_check.front());

		/* add neighbors to list to check */
		range = boundary.get_neighbors(to_check.front());
		for(it = range.first; it != range.second; it++)
			to_check.push(*it);
	}
}

void planar_region_t::writeobj(ostream& os) const
{
	faceset::iterator it;
	int r, g, b;

	/* generate a random color */
	r = 128 + (rand() % 64);
	g = 128 + (rand() % 64);
	b = 128 + (rand() % 64);

	/* write the faces with this color */
	for(it = this->faces.begin(); it != this->faces.end(); it++)
		it->writeobj(os, r, g, b);
}
