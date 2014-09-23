#include "node_corner.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <Eigen/Dense>
#include <iostream>

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
			
void corner_t::writeobj(std::ostream& os, const octree_t& tree) const
{
	Vector3d p; 

	/* export point at corner position */
	this->get_position(tree, p);
	os << "v " << p.transpose() << endl;
}
