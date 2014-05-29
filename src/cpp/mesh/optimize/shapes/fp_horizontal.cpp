#include "fp_horizontal.h"
#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <geometry/poly_intersect/poly2d.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <cmath>

/**
 * @file   fp_horizontal.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines the fp_horizontal_t class, describes fp geometry
 *
 * @section DESCRIPTION
 *
 * The fp_horizontal_t class is used to describe the geometry of the floor
 * or ceiling of a room of an extruded floorplan mesh.  This horizontal is 
 * defined as a polygon whose normal is vertical.
 *
 * This class is used to analyze the position of this geometry within
 * the context of a carving defined by an octree.  The octree carving
 * can be used to align the floorplan geometry to be consistent with the
 * carving geometry.
 */

using namespace std;
using namespace Eigen;
using namespace fp;
using namespace poly2d;

/* function implementations */

void fp_horizontal_t::init(const fp::floorplan_t& f, unsigned int ri,
                           bool isfloor, double off)
{
	double h;

	/* initialize structure elements */
	this->offset_gap = off;

	/* initialize cost to zero */
	this->offset_cost = 0;

	/* copy position information from floorplan about this surface */
	this->norm_up = isfloor;
	this->z = isfloor ? f.rooms[ri].min_z : f.rooms[ri].max_z;
	h = this->z + this->get_norm()*this->offset_gap;
	this->shape.init(f, ri, ri, h, h);
}
		
double fp_horizontal_t::get_offset_cost() const
{
	/* return the computed offset cost, along with an additive
	 * rigitity term that is mean to prevent the horizontal from
	 * drifting too far */
	return (this->offset_cost);
}

/*-----------------------------------*/
/* overloaded functions from shape_t */
/*-----------------------------------*/

unsigned int fp_horizontal_t::num_verts() const
{
	/* just pass the shape's value */
	return this->shape.num_verts();
}

Vector3d fp_horizontal_t::get_vertex(unsigned int i) const
{
	/* just pass the shape's value */
	return this->shape.get_vertex(i);
}
		
bool fp_horizontal_t::intersects(const Vector3d& c, double hw) const
{
	/* check intersection with poly */
	return this->shape.intersects(c, hw);
}
		
octdata_t* fp_horizontal_t::apply_to_leaf(const Vector3d& c, double hw,
					octdata_t* d)
{
	double p;
	MARK_USED(c);

	/* the cost of a particular offset is based on how much
	 * of the surface this shape intersects in the carving.  Ideally, 
	 * this would be zero at just the 'interior' side of the surface 
	 * described by the octree, so that the fp floor or ceiling gets
	 * as close to the boundary of the room as possible without 
	 * intersecting any exterior nodes.
	 *
	 * The weighting shown below is meant to enforce this approach,
	 * so that higher cost is given to intersected nodes that are
	 * exterior and planar.  There is also an additive term meant
	 * to increase the cost as the horizontal gets farther from 
	 * its original position.
	 */

	/* get probability of the current node being exterior */
	if(d == NULL)
		p = 0.5; /* null node has no observation */
	else if(!(d->is_interior()))
		p = (1 - d->get_probability()); /* prob. of exterior node */
	else
		p = 0; /* interior nodes get no cost */

	/* the cost should grow as the node gets more exterior,
	 * more planar, or if the horizontal is offset by a large
	 * distance from its original position */
	this->offset_cost += p*hw*hw*(d->get_planar_prob());

	/* return the same data as given */
	return d;
}
