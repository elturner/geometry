#include "bloated_fp.h"
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <geometry/poly_intersect/poly2d.h>
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <Eigen/Dense>

/**
 * @file   bloated_fp.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The shape object representing a floorplan with buffer distance
 *
 * @section DESCRIPTION
 *
 * This file defines the bloated_fp_t, which implements
 * the shape_t interface for octrees.  It is used to intersect
 * an entire floorplan with an octree.  Any nodes that are significantly
 * far away from the floorplan are removed from an internal list.
 *
 * The goal is to find all octree nodes that are 'explosions'.  That
 * is, are very far away from the floorplan, indicating they are outside
 * the building, which means they shouldn't be modeled.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

bloated_fp_t::bloated_fp_t()
{
	/* set default values */
	this->floor_height   = 1;
	this->ceiling_height = 0;
	this->bounds_x[0]    = 1;
	this->bounds_x[1]    = 0;
	this->bounds_y[0]    = 1;
	this->bounds_y[1]    = 0;
}

void bloated_fp_t::init(const fp::floorplan_t& f, double buffer)
{
	size_t ri, num_rooms;

	/* check if valid */
	num_rooms = f.rooms.size();
	if(num_rooms == 0)
	{
		/* no room or height info */
		this->init(f, 0, 0, buffer);
		return;
	}

	/* find the min/max height of this floorplan */
	this->floor_height   = f.rooms[0].min_z;
	this->ceiling_height = f.rooms[0].max_z;
	for(ri = 1; ri < num_rooms; ri++)
	{
		/* update bounds */
		if(this->floor_height > f.rooms[ri].min_z)
			this->floor_height = f.rooms[ri].min_z;
		if(this->ceiling_height < f.rooms[ri].max_z)
			this->ceiling_height = f.rooms[ri].max_z;
	}

	/* initialize with default heights */
	this->init(f, this->floor_height, this->ceiling_height, buffer);
}

void bloated_fp_t::init(const fp::floorplan_t& f,
		          double fh, double ch, double buffer)
{
	size_t ti, num_tris, vi, num_verts;
	Vector2d c, dv, v;

	/* save height information about this room */
	this->floor_height = fh - buffer;
	this->ceiling_height = ch + buffer;

	/* we will also copy over the triangle information */
	num_tris = f.tris.size();
	this->tris.resize(6, num_tris);

	/* iterate over the triangles of this room */
	for(ti = 0; ti < num_tris; ti++)
	{
		/* populate geometry of this triangle */
		this->tris(0,ti) = f.verts[f.tris[ti].verts[0]].x;
		this->tris(1,ti) = f.verts[f.tris[ti].verts[0]].y;
		this->tris(2,ti) = f.verts[f.tris[ti].verts[1]].x;
		this->tris(3,ti) = f.verts[f.tris[ti].verts[1]].y;
		this->tris(4,ti) = f.verts[f.tris[ti].verts[2]].x;
		this->tris(5,ti) = f.verts[f.tris[ti].verts[2]].y;
	
		/* compute centroid of triangle */
		c(0) = this->tris(0,ti)+this->tris(2,ti)+this->tris(4,ti);
		c(1) = this->tris(1,ti)+this->tris(3,ti)+this->tris(5,ti);
		c /= fp::NUM_VERTS_PER_TRI;

		/* add buffer to 'bloat' the triangle */
		for(vi = 0; vi < fp::NUM_VERTS_PER_TRI; vi++)
		{
			/* get this vertex */
			v << this->tris(2*vi,ti), this->tris(2*vi+1,ti);
			
			/* compute distance from centroid */
			dv = v - c;
			dv.normalize();
			dv *= buffer;

			/* add buffer to vertex */
			v += dv;
			this->tris(2*vi,ti) = v(0);
			this->tris(2*vi+1,ti) = v(1);
		}
	}

	/* clear output list */
	this->whitelist.clear();
	
	/* check for edge case */
	num_verts = f.verts.size();
	if(num_verts == 0)
		return; /* can't do much more */

	/* update the 2D bounding box */
	this->bounds_x[0] = this->bounds_x[1] = f.verts[0].x;
	this->bounds_y[0] = this->bounds_y[1] = f.verts[0].y;
	for(vi = 1; vi < num_verts; vi++)
	{
		/* update bounds */
		if(this->bounds_x[0] > f.verts[vi].x)
			this->bounds_x[0] = f.verts[vi].x;
		if(this->bounds_x[1] < f.verts[vi].x)
			this->bounds_x[1] = f.verts[vi].x;
		if(this->bounds_y[0] > f.verts[vi].y)
			this->bounds_y[0] = f.verts[vi].y;
		if(this->bounds_y[1] < f.verts[vi].y)
			this->bounds_y[1] = f.verts[vi].y;
	}

	/* add buffer to the bounding box */
	this->bounds_x[0] -= buffer;
	this->bounds_x[1] += buffer;
	this->bounds_y[0] -= buffer;
	this->bounds_y[1] += buffer;
}
		
Vector3d bloated_fp_t::get_vertex(unsigned int i) const
{
	Vector3d v;
	bool isceil;
	unsigned int net_index, ti, vi;

	/* get the appropriate vertex by checking if the
	 * specified index counts as a floor or a ceiling vertex */
	isceil = (((int) i) >= fp::NUM_VERTS_PER_TRI*this->tris.cols());
	net_index = (i % (fp::NUM_VERTS_PER_TRI*this->tris.cols()));

	/* get relevant indices */
	ti = net_index / fp::NUM_VERTS_PER_TRI;
	vi = net_index % fp::NUM_VERTS_PER_TRI;

	/* get 2D vertex */
	v(0) = this->tris(2*vi, ti);
	v(1) = this->tris(2*vi+1, ti);

	/* modify it if it's on the ceiling */
	v(2) = isceil ? this->ceiling_height : this->floor_height;

	/* return this vertex position */
	return v;
}
		
bool bloated_fp_t::intersects(const Vector3d& c, double hw) const
{
	Vector2d p, q, r;
	double bx[2]; /* min, max for input box x-direction */
	double by[2]; /* min, max for input box y-direction */
	unsigned int ti, num_tris, vi, vi_next;

	/* NOTE:
	 *
	 * We are testing for intersection with the complement of
	 * this floorplan.  If c/hw intersect with the floorplan,
	 * we want to return false.  If it doesn't intersect, then
	 * return true.
	 */

	/* check intersection along z */
	if(c(2) - hw > this->ceiling_height 
			|| c(2) + hw < this->floor_height)
		return false; /* no intersection possible */
	
	/* now we only have to worry about 2D intersection with an
	 * axis-aligned bounding box */
	bx[0] = c(0) - hw; /* x-min for box */
	bx[1] = c(0) + hw; /* x-max for box */
	by[0] = c(1) - hw; /* y-min for box */
	by[1] = c(1) + hw; /* y-max for box */

	/* check if bounding boxes intersect */
	if(!(poly2d::aabb_in_aabb(bx, by, bounds_x, bounds_y)))
		return false; /* no intersection possible */

	/* check if the vertices of this polygon intersect the box */
	num_tris = this->tris.cols();
	for(ti = 0; ti < num_tris; ti++)
	{
		/* iterate over the vertices of this triangle */
		for(vi = 0; vi < fp::NUM_VERTS_PER_TRI; vi++)
		{
			/* check if this vertex intersects box */
			if(poly2d::point_in_aabb(
					this->tris(2*vi,ti),
					this->tris(2*vi+1,ti),
					bx[0],by[0],bx[1],by[1]))
				return true; /* intersection */
	
			/* check if the edges of each triangle intersect
			 * the box */
			vi_next = (vi+1) % fp::NUM_VERTS_PER_TRI;
			p(0) = this->tris(2*vi,        ti);
			p(1) = this->tris(2*vi     +1, ti);
			q(0) = this->tris(2*vi_next,   ti);
			q(1) = this->tris(2*vi_next+1, ti);

			/* test if line intersects */
			if(poly2d::line_in_aabb(p(0),p(1),q(0),q(1),bx,by))
				return true; /* intersects with edge */
		}
	
		/* since no vertices and no edges intersect, then the only
		 * way an intersection could occur is if the box is entirely
		 * contained within the polygon.  Check if the center of 
		 * the box is inside any of the triangles */
		
		/* test if center of box intersects with triangle */
		if(poly2d::point_in_triangle(
				this->tris(0,ti),
				this->tris(1,ti),
				this->tris(2,ti),
				this->tris(3,ti),
				this->tris(4,ti),
				this->tris(5,ti),
				c(0), c(1)))
			return true; /* center intersects triangle */
	}

	/* no intersections found */
	return false;
}
		
octdata_t* bloated_fp_t::apply_to_leaf(const Vector3d& c,
                                          double hw, octdata_t* d)
{
	/* node size inputs are not used */
	MARK_USED(c);
	MARK_USED(hw);

	/* store data */
	this->whitelist.insert(d);

	/* don't make modifications */
	return d;
}
		
void bloated_fp_t::writeobj(std::ostream& os) const
{
	size_t ti, num_tris;

	/* iterate over triangles */
	num_tris = this->tris.cols();
	for(ti = 0; ti < num_tris; ti++)
	{
		/* print the vertices of this floor triangle */
		os << "v " << this->tris(0,ti)
		   <<  " " << this->tris(1,ti)
		   <<  " " << this->floor_height
		   <<  endl
		   << "v " << this->tris(2,ti)
		   <<  " " << this->tris(3,ti)
		   <<  " " << this->floor_height
		   <<  endl
		   << "v " << this->tris(4,ti)
		   <<  " " << this->tris(5,ti)
		   <<  " " << this->floor_height
		   <<  endl
		   << "f -3 -2 -1" << endl;

		/* print this ceiling triangle */
		os << "v " << this->tris(0,ti)
		   <<  " " << this->tris(1,ti)
		   <<  " " << this->ceiling_height
		   <<  endl
		   << "v " << this->tris(2,ti)
		   <<  " " << this->tris(3,ti)
		   <<  " " << this->ceiling_height
		   <<  endl
		   << "v " << this->tris(4,ti)
		   <<  " " << this->tris(5,ti)
		   <<  " " << this->ceiling_height
		   <<  endl
		   << "f -1 -2 -3" << endl;
	}
}
