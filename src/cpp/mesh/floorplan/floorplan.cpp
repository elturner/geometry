#include "floorplan.h"
#include <iostream>
#include <vector>
#include <map>
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
			
void floorplan_t::compute_oriented_boundary(vector<vector<int> >& 
			boundary_list, const set<int>& tris) const
{
	set<int>::const_iterator tit;
	map<int, set<edge_t> > edge_map;
	set<edge_t> all_edges;
	set<edge_t>::const_iterator eit;
	vector<edge_t> to_remove;
	vector<edge_t>::const_iterator vit;
	unsigned int i;
	edge_t e;
	int last;

	/* get all the edges for these triangles */
	for(tit = tris.begin(); tit != tris.end(); tit++)
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			all_edges.insert(this->tris[*tit].get_edge(i));

	/* iterate over the list of edges, looking for opposing edges */
	for(eit = all_edges.begin(); eit != all_edges.end(); eit++)
	{
		/* get flip of this edge */
		e = eit->flip();

		/* check if flip is present */
		if(all_edges.count(e))
		{
			/* remove both, since neither is a boundary
			 * if the other is present */
			to_remove.push_back(*eit);
			to_remove.push_back(e);
		}
	}

	/* remove any edges that were found to be non-boundary */
	for(vit = to_remove.begin(); vit != to_remove.end(); vit++)
		all_edges.erase(*vit);
	to_remove.clear();

	/* order the edges by mapping the start vertex index to each
	 * edge */
	for(eit = all_edges.begin(); eit != all_edges.end(); eit++)
		edge_map[eit->verts[0]].insert(*eit);

	/* initialize output */
	boundary_list.clear();

	/* build boundaries */
	while(!(all_edges.empty()))
	{
		/* start a new boundary */
		boundary_list.push_back(vector<int>(0));

		/* find the next edge to follow, remove it
		 * from the pool of remaining edges */
		eit = all_edges.begin();
		boundary_list.back().push_back(eit->verts[0]);
		boundary_list.back().push_back(eit->verts[1]);
		last = eit->verts[1];
		edge_map[eit->verts[0]].erase(*eit);
		all_edges.erase(eit);

		/* follow this edge as long as possible to form
		 * an oriented boundary */
		while(!(edge_map[last].empty()))
		{
			/* find an edge that starts with the last
			 * element in the current boundary */
			eit = edge_map[last].begin();

			/* check if we've made a loop */
			if(eit->verts[1] == boundary_list.back().front())
			{
				/* back at starting vertex, loop complete */
				all_edges.erase(*eit);
				edge_map[last].erase(eit);
				break;
			}

			/* follow this edge */
			boundary_list.back().push_back(eit->verts[1]);

			/* remove the edge we just used from the pool
			 * of available edges */
			all_edges.erase(*eit);
			edge_map[last].erase(eit);

			/* continue search on latest vertex */
			last = eit->verts[1];
		}
	}
}
			
void floorplan_t::compute_oriented_boundary(
		std::vector<std::vector<int> >& boundary_list) const
{
	set<int> t;
	int i, n;

	/* populate set with all triangle indices */
	n = (int) this->tris.size();
	for(i = 0; i < n; i++)
		t.insert(i);

	/* get boundary */
	this->compute_oriented_boundary(boundary_list, t);
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
	double area;

	/* verify valid room index */
	if(i >= this->rooms.size())
		return 0.0; /* non-existant rooms have no area */

	/* iterate over the triangles in this room */
	area = 0.0; /* initialize area to be zero */
	for(it = this->rooms[i].tris.begin(); 
			it != this->rooms[i].tris.end(); it++)
		area += this->compute_triangle_area(*it);

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
			
void floorplan_t::snap_room_floors()
{
	double area, elev, avg, sum;
	size_t vi, ri, num_verts, num_rooms;

	/* get all the floor heights from each room */
	num_rooms = this->rooms.size();
	avg = sum = 0.0;
	for(ri = 0; ri < num_rooms; ri++)
	{
		/* add this room to our weighted average */
		area = this->compute_room_area(ri);
		elev = this->rooms[ri].min_z;
		avg += area*elev;
		sum += area;
	}
	avg /= sum;

	/* update all rooms to have the same floor */
	for(ri = 0; ri < num_rooms; ri++)
		this->rooms[ri].min_z = avg;
	
	/* update all vertices with the new height */
	num_verts = this->verts.size();
	for(vi = 0; vi < num_verts; vi++)
		this->verts[vi].min_z = avg;
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
