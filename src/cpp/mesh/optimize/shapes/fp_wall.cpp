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

void fp_wall_t::init(double r, const floorplan_t& f,
                     const edge_t& e)
{
	unsigned int i;

	/* initialize structure elements */
	this->offset_gap = r;
	this->use_offset = false;
	
	/* copy position information from floorplan about this wall */
	this->edge = e;
	for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
	{
		/* copy position of this vertex */
		this->edge_pos[i](0) = f.verts[e.verts[i]].x;
		this->edge_pos[i](1) = f.verts[e.verts[i]].y;
	}

	/* compute normal direction of edge
	 *
	 * x = -dy
	 * y = dx 
	 */
	this->norm(0) = this->edge_pos[0](1) - this->edge_pos[1](1);
	this->norm(1) = this->edge_pos[1](0) - this->edge_pos[0](0); 

	/* set position of corresponding offset surface vertex by
	 * using the normal vector.  This offset surface should be
	 * 'behind' the wall, since intersections are computed inclusively,
	 * and the normal vector points inward, so the gap between
	 * the offset and the wall should represent the actual interface
	 * between the inside and outside. */
	for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
		this->offset_edge_pos[i] = this->edge_pos[i] - r*this->norm;

	/* compute length and tangent */
	this->tangent = this->edge_pos[1] - this->edge_pos[0];
	this->length = this->tangent.norm();
	this->tangent /= this->length;

	/* store height information */
	this->min_z = min(f.verts[e.verts[0]].min_z,
	                  f.verts[e.verts[1]].min_z);
	this->max_z = max(f.verts[e.verts[0]].max_z,
	                  f.verts[e.verts[1]].max_z);

	/* initialize forces to zero */
	this->num_nodes = 0;
	for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
		this->scalar_sum[i]   = 0.0;
}
		
void fp_wall_t::compute_forces(Eigen::Vector2d& f0, 
                               Eigen::Vector2d& f1)
{
	/* cannot produce valid vectors if not enough observations
	 * have occurred */
	if(this->num_nodes == 0)
	{
		f0(0) = 0; f0(1) = 0;
		f1(0) = 0; f1(1) = 0;
		return;
	}

	/* compute force for vertices */
	f0 = this->norm * this->scalar_sum[0];
	f1 = this->norm * this->scalar_sum[1];
}
		
Vector3d fp_wall_t::get_vertex(unsigned int i) const
{
	Vector3d v;

	/* change the value of i if we want to use the offset
	 * position of this wall */
	if(this->use_offset)
		i += NUM_VERTS_PER_RECT;

	/* construct the vertex based on given index */
	switch(i)
	{
		/* main surface */
		case 0:
			/* upper-right corner */
			v(0) = this->edge_pos[0](0);
			v(1) = this->edge_pos[0](1);
			v(2) = this->max_z;
			break;
		case 1:
			/* upper-left corner */
			v(0) = this->edge_pos[1](0);
			v(1) = this->edge_pos[1](1);
			v(2) = this->max_z;
			break;
		case 2:
			/* lower-left corner */
			v(0) = this->edge_pos[1](0);
			v(1) = this->edge_pos[1](1);
			v(2) = this->min_z;
			break;
		case 3:
			/* lower-right corner */
			v(0) = this->edge_pos[0](0);
			v(1) = this->edge_pos[0](1);
			v(2) = this->min_z;
			break;

		/* secondary surface for gradient calculations */
		case 4:
			/* upper-right corner */
			v(0) = this->offset_edge_pos[0](0);
			v(1) = this->offset_edge_pos[0](1);
			v(2) = this->max_z;
			break;
		case 5:
			/* upper-left corner */
			v(0) = this->offset_edge_pos[1](0);
			v(1) = this->offset_edge_pos[1](1);
			v(2) = this->max_z;
			break;
		case 6:
			/* lower-left corner */
			v(0) = this->offset_edge_pos[1](0);
			v(1) = this->offset_edge_pos[1](1);
			v(2) = this->min_z;
			break;
		case 7:
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
	
	/* determine which surface to use */
	if(!(this->use_offset))
	{
		/* check if this node intersects the main surface, 
		 * which only needs to check a 2D intersection */
		if(line_in_aabb(this->edge_pos[0](0), this->edge_pos[0](1),
				this->edge_pos[1](0), this->edge_pos[1](1),
				bounds_x, bounds_y))
			return true; /* intersects surface */
	}
	else
	{
		/* check if this node intersects the offset surface */
		if(line_in_aabb(this->offset_edge_pos[0](0),
				this->offset_edge_pos[0](1),
				this->offset_edge_pos[1](0),
				this->offset_edge_pos[1](1),
				bounds_x, bounds_y))
			return true; /* intersects surface */
	}

	/* no intersection found */
	return false;
}
		
octdata_t* fp_wall_t::apply_to_leaf(const Vector3d& c, double hw,
					octdata_t* d)
{
	Vector2d p;
	double prob, s, dist;

	/* check if there exist data here */
	if(d == NULL)
		return d;

	/* get the scalar function at this position */
	prob = d->get_probability();
	s = d->is_interior() ? -1 : 1; /* if interior, push out */ 
	s *= (d->get_planar_prob())*(d->get_surface_prob())*hw*hw;

	/* get distance along surface of wall, to properly divide
	 * force between the two vertices */
	p(0) = c(0); p(1) = c(1);	
	dist = this->tangent.dot(p - this->edge_pos[0]) / this->length;
	
	/* add to accumulation of forces */
	this->num_nodes++;
	this->scalar_sum[0] += (1-dist)*s;
	this->scalar_sum[1] += dist*s;

	/* return the same data as given */
	return d;
}
