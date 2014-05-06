#include "floorplan.h"
#include <vector>
#include <set>
#include <float.h>

/**
 * @file floorplan.cpp
 * @author Eric Turner
 *
 * @section DESCRIPTION
 *
 * This file implements the classes and functions used to define
 * a 2D floorplan with extruded height information.
 * A floorplan is composed of a set of rooms, where each
 * room is a 2D triangulation with a set floor and ceiling
 * height.
 */

using namespace std;
using namespace fp;

/****** FLOORPLAN_T FUNCTIONS *******/

floorplan_t::floorplan_t()
{
	this->clear();
}

floorplan_t::~floorplan_t()
{
	this->clear();
}
	
void floorplan_t::clear()
{
	this->verts.clear();
	this->tris.clear();
	this->rooms.clear();
}

void floorplan_t::compute_edges(vector<edge_t>& edges) const
{
	unsigned int ti, ni, num_tris;

	/* clear input arguments */
	edges.clear();

	/* iterate over triangles */
	num_tris = this->tris.size();
	for(ti = 0; ti < num_tris; ti++)
	{
		/* check edges of this triangle */
		for(ni = 0; ni < NUM_EDGES_PER_TRI; ni++)
		{
			/* is this edge one-sided? */
			if(this->tris[ti].neighs[ni] < 0)
			{
				/* this edge is one-sided,
				 * so add it to our list */
				edges.push_back(
					this->tris[ti].get_edge(ni));
			}
		}
	}
}

void floorplan_t::compute_edges_for_room(vector<edge_t>& edges,
                                         unsigned int ri) const
{
	set<int>::iterator tit;
	unsigned int ti, t_other, ni;

	/* clear input arguments */
	edges.clear();

	/* iterate over triangles */
	for(tit = this->rooms[ri].tris.begin();
			tit != this->rooms[ri].tris.end(); tit++)
	{
		/* get triangle index */
		ti = *tit;

		/* check edges of this triangle */
		for(ni = 0; ni < NUM_EDGES_PER_TRI; ni++)
		{
			/* is this edge one-sided? */
			t_other = this->tris[ti].neighs[ni];
			if(!this->rooms[ri].tris.count(t_other))
			{
				/* neighboring triangle is not in
				 * room, so edge is one-sided,
				 * so add it to our list */
				edges.push_back(
					this->tris[ti].get_edge(ni));
			}
		}
	}
}
	
void floorplan_t::compute_bounds(double& min_x, double& min_y,
                                 double& max_x, double& max_y) const
{
	int i, num_verts;

	/* initialize bounds */
	min_x = min_y = DBL_MAX;
	max_x = max_y = -DBL_MAX;

	/* iteate over vertices */
	num_verts = this->verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* compare against current bounds */
		if(this->verts[i].x < min_x)
			min_x = this->verts[i].x;
		if(this->verts[i].x > max_x)
			max_x = this->verts[i].x;
		if(this->verts[i].y < min_y)
			min_y = this->verts[i].y;
		if(this->verts[i].y > max_y)
			max_y = this->verts[i].y;
	}
}
			
double floorplan_t::compute_room_area(unsigned int i) const
{
	set<int>::iterator it;
	size_t ti;
	int p, q, r;
	double area, ux, uy, vx, vy;

	/* verify valid room index */
	if(i >= this->rooms.size())
		return 0.0; /* non-existant rooms have no area */

	/* iterate over the triangles in this room */
	area = 0.0; /* initialize area to be zero */
	for(it = this->rooms[i].tris.begin(); 
			it != this->rooms[i].tris.end(); it++)
	{
		/* get the index of this triangle */
		ti = *it;

		/* get the vertices of this triangle */
		p = this->tris[ti].verts[0];
		q = this->tris[ti].verts[1];
		r = this->tris[ti].verts[2];
		
		/* compute the area of this triangle by
		 * taking half the cross-product of two
		 * edges. */

		/* get vectors */
		ux = this->verts[p].x - this->verts[r].x;
		uy = this->verts[p].y - this->verts[r].y;
		vx = this->verts[q].x - this->verts[r].x;
		vy = this->verts[q].y - this->verts[r].y;

		/* compute half of cross product */
		area += (ux*vy - uy*vx)/2;
	}

	/* return the total area for the room */
	return area;
}
			
double floorplan_t::compute_total_area() const
{
	size_t ri, num_rooms;
	double area;

	/* iterate over the rooms in this model */
	area = 0.0;
	num_rooms = this->rooms.size();
	for(ri = 0; ri < num_rooms; ri++)
		area += this->compute_room_area(ri);
	
	/* return the total area of all rooms */
	return area;
}

/********* VERTEX_T FUNCTIONS ****************/
	
vertex_t::vertex_t()
{
	/* set to default */
	this->clear();
}

vertex_t::~vertex_t()
{
	this->clear();
}
	
void vertex_t::clear()
{
	/* set default position at origin */
	this->x = this->y = 0;
	this->min_z = 1;
	this->max_z = -1;
	this->ind = -1;
	this->tri_neighs.clear();
}

/****** EDGE FUNCTIONS ******/

edge_t::edge_t()
{
	this->set(-1, -1);
}

edge_t::edge_t(int i, int j)
{
	this->set(i, j);
}

/****** TRIANGLE FUNCTIONS ******/

triangle_t::triangle_t()
{
	unsigned int i;

	/* set to default values */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		this->verts[i] = -1;
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		this->neighs[i] = -1;
	this->ind = -1;
}

edge_t triangle_t::get_edge(unsigned int ni) const
{
	edge_t e;

	/* check validity of input */
	if(ni < NUM_EDGES_PER_TRI)
	{
		/* set value of e to specified edge */
		e.set(	this->verts[ (ni + 1) % NUM_VERTS_PER_TRI ],
			this->verts[ (ni + 2) % NUM_VERTS_PER_TRI ] );
	}

	/* return the edge */
	return e;
}
	
/*********** ROOM_T FUNCTIONS *****************/

room_t::room_t()
{
	this->clear();
}

room_t::~room_t()
{
	this->clear();
}
	
void room_t::clear()
{
	/* set default values */
	this->tris.clear();
	this->ind = -1;
	this->min_z = 1;
	this->max_z = -1;
}
