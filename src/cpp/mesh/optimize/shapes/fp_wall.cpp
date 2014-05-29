#include "fp_wall.h"
#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <geometry/poly_intersect/poly2d.h>
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <cmath>

/**
 * @file   fp_wall.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines the fp_wall_t class, describes fp geometry
 *
 * @section DESCRIPTION
 *
 * The fp_wall_t class is used to describe the geometry of a wall in an
 * extruded floorplan mesh.  This wall is defined as a rectangle whose
 * normal is horizontal.  It originates from two 2D wall samples within
 * the floorplan.
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

void fp_wall_t::init(const floorplan_t& f, const edge_t& e)
{
	unsigned int i;

	/* initialize structure elements */
	this->offset_gap = 0;

	/* copy position information from floorplan about this wall */
	this->edge = e;
	for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
	{
		/* copy position of this vertex */
		this->edge_pos[i](0) = f.verts[e.verts[i]].x;
		this->edge_pos[i](1) = f.verts[e.verts[i]].y;
		this->offset_edge_pos[i] = this->edge_pos[i];
	}

	/* compute normal direction of edge
	 *
	 * x = -dy
	 * y = dx 
	 */
	this->norm(0) = this->edge_pos[0](1) - this->edge_pos[1](1);
	this->norm(1) = this->edge_pos[1](0) - this->edge_pos[0](0); 
	this->norm.normalize();

	/* compute length and tangent */
	this->tangent = this->edge_pos[1] - this->edge_pos[0];
	this->length = this->tangent.norm();
	this->tangent /= this->length;

	/* store height information */
	this->min_z = min(f.verts[e.verts[0]].min_z,
	                  f.verts[e.verts[1]].min_z);
	this->max_z = max(f.verts[e.verts[0]].max_z,
	                  f.verts[e.verts[1]].max_z);

	/* initialize cost to zero */
	this->offset_cost = 0;
}
		
void fp_wall_t::set_offset(double off)
{
	unsigned int i;

	/* store the specified offset gap distance */
	this->offset_gap = off;
	this->offset_cost = 0; /* with a new offset, reset the cost */

	/* populate the offset surface vertices to be at this distance */
	for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
		this->offset_edge_pos[i] = this->edge_pos[i] 
						+ off*this->norm;
}
		
double fp_wall_t::get_offset_cost() const
{
	/* return the computed offset cost, along with an additive
	 * rigitity term that is mean to prevent the wall from
	 * drifting too far */
	return (this->offset_cost);
}

/*-----------------------------------*/
/* overloaded functions from shape_t */
/*-----------------------------------*/

Vector3d fp_wall_t::get_vertex(unsigned int i) const
{
	Vector3d v;

	/* construct the vertex based on given index */
	switch(i)
	{
		case 0:
			/* upper-right corner */
			v(0) = this->offset_edge_pos[0](0);
			v(1) = this->offset_edge_pos[0](1);
			v(2) = this->max_z;
			break;
		case 1:
			/* upper-left corner */
			v(0) = this->offset_edge_pos[1](0);
			v(1) = this->offset_edge_pos[1](1);
			v(2) = this->max_z;
			break;
		case 2:
			/* lower-left corner */
			v(0) = this->offset_edge_pos[1](0);
			v(1) = this->offset_edge_pos[1](1);
			v(2) = this->min_z;
			break;
		case 3:
			/* lower-right corner */
			v(0) = this->offset_edge_pos[0](0);
			v(1) = this->offset_edge_pos[0](1);
			v(2) = this->min_z;
			break;

		/* check if invalid */
		default:
			cerr << "[fp_wall_t::get_vertex]\tError! Request "
			     << "for vertex #" << i << endl;
			break;
	}

	/* return this vertex */
	return v;
}
		
bool fp_wall_t::intersects(const Vector3d& c, double hw) const
{
	double bounds_x[2];
	double bounds_y[2];

	/* check if heights intersect */
	if(c(2) - hw > this->max_z || c(2) + hw < this->min_z)
		return false; /* can't intersect since heights disjoint */

	/* get geometry of the node that is potentially intersected */
	bounds_x[0] = c(0) - hw;
	bounds_x[1] = c(0) + hw;
	bounds_y[0] = c(1) - hw;
	bounds_y[1] = c(1) + hw;
	
	/* check if this node intersects the offset surface */
	if(line_in_aabb(this->offset_edge_pos[0](0),
			this->offset_edge_pos[0](1),
			this->offset_edge_pos[1](0),
			this->offset_edge_pos[1](1),
			bounds_x, bounds_y))
		return true; /* intersects surface */

	/* no intersection found */
	return false;
}
		
octdata_t* fp_wall_t::apply_to_leaf(const Vector3d& c, double hw,
					octdata_t* d)
{
	double p;

	/* check if there exist data here */
	if(d == NULL)
		return d;

	/* the cost of a particular offset is based on how much
	 * of the wall it intersects.  Ideally, this would be zero
	 * at just the 'interior' side of the surface described by
	 * the octree, so that the fp wall gets as close to the boundary
	 * of the room as possible without intersecting any exterior nodes.
	 *
	 * The weighting shown below is meant to enforce this approach,
	 * so that higher cost is given to intersected nodes that are
	 * exterior and planar.  There is also an additive term meant
	 * to increase the cost as the wall gets farther from its original
	 * position.
	 */

	/* no cost for an interior node */
	if(!(d->is_interior()))
	{
		/* the cost should grow as the node gets more exterior,
		 * more planar, or if the wall is offset by a large
		 * distance from its original position */
		p = (1 - d->get_probability())*hw*hw*(d->get_planar_prob());
		this->offset_cost += p;
	}

	/* return the same data as given */
	return d;
}
