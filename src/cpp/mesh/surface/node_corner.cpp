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
			
void corner_t::writecsv(std::ostream& os) const
{
	os << this->x_ind << ","
	   << this->y_ind << ","
	   << this->z_ind << ",";
}
			
bool corner_t::within_bounds(const corner_t& min_c,
					const corner_t& max_c) const
{
	/* check that this corner falls within the given bounds */
	if(this->x_ind < min_c.x_ind || this->x_ind > max_c.x_ind)
		return false; /* out of bounds in x */
	if(this->y_ind < min_c.y_ind || this->y_ind > max_c.y_ind)
		return false; /* out of bounds in y */
	if(this->z_ind < min_c.z_ind || this->z_ind > max_c.z_ind)
		return false; /* out of bounds in z */
	return true; /* in bounds */
}
			
void corner_t::update_bounds(corner_t& min_c, corner_t& max_c) const
{
	/* update x-component */
	if(this->x_ind < min_c.x_ind)
		min_c.x_ind = this->x_ind;
	if(this->x_ind > max_c.x_ind)
		max_c.x_ind = this->x_ind;
	
	/* update y-component */
	if(this->y_ind < min_c.y_ind)
		min_c.y_ind = this->y_ind;
	if(this->y_ind > max_c.y_ind)
		max_c.y_ind = this->y_ind;

	/* update z-component */
	if(this->z_ind < min_c.z_ind)
		min_c.z_ind = this->z_ind;
	if(this->z_ind > max_c.z_ind)
		max_c.z_ind = this->z_ind;
}
