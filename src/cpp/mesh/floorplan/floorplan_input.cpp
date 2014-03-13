#include "floorplan.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <string.h>
#include <string>
#include <sstream>

/**
 * @file floorplan_input.cpp
 * @author Eric Turner
 *
 * @section DESCRIPTION
 *
 * This file defines classes used to define
 * a 2D floorplan with extruded height information.
 * A floorplan is composed of a set of rooms, where each
 * room is a 2D triangulation with a set floor and ceiling
 * height.
 *
 * This file implements the file parsing and construction
 * helper functions, which are used to initially form
 * a floorplan object from file.
 */

using namespace std;
using namespace fp;

/*
 * File Format for .fp (all units in meters)
 * -----------------------------------------
 *
 * 	<resolution>		// the resolution of model, in meters
 *	<num_verts>
 *	<num_tris>
 *	<num_rooms>
 *	<x1> <y1>		// first vertex position (two doubles)
 *	...
 *	<xn> <yn>		// n'th vertex position (two doubles)
 *	<i1> <j1> <k1>		// first triangle (three ints)
 *	...
 *	<im> <jm> <km>		// m'th triangle (three ints)
 *	<z1_min> <z1_max> <num_tris> <t_1> <t_2> ... <t_k1>	// room 1
 *	...
 *	<zr_min> <zr_max> <num_tris> <t_1> <t_2> ... <t_kr>	// room r
 *
 */

/****** MESH_T FUNCTIONS *******/

int floorplan_t::import_from_fp(const string& filename)
{
	ifstream infile;
	vertex_t v;
	triangle_t t;
	room_t r;
	string buf;
	stringstream ss;
	int i, j, ti, ret, num_verts, num_tris, num_rooms;

	/* attempt to open file */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
		return -1;

	/* read header information */

	/* resolution */
	getline(infile, buf);
	ret = sscanf(buf.c_str(), "%lf", &(this->res));
	if(ret != 1)
	{
		/* unable to read double */
		infile.close();
		return -2;
	}
	
	/* num vertices */
	getline(infile, buf);
	ret = sscanf(buf.c_str(), "%d", &num_verts);
	if(ret != 1)
	{
		/* unable to read int */
		infile.close();
		return -3;
	}
	
	/* num triangles */
	getline(infile, buf);
	ret = sscanf(buf.c_str(), "%d", &num_tris);
	if(ret != 1)
	{
		/* unable to read int */
		infile.close();
		return -4;
	}
	
	/* num rooms */
	getline(infile, buf);
	ret = sscanf(buf.c_str(), "%d", &num_rooms);
	if(ret != 1)
	{
		/* unable to read int */
		infile.close();
		return -4;
	}

	/* read structures */

	/* read vertices */
	this->verts.reserve(num_verts);
	for(i = 0; i < num_verts; i++)
	{
		/* reset the buffer vertex */
		v.clear();

		/* read next line and parse position */
		getline(infile, buf);
		ret = sscanf(buf.c_str(), "%lf %lf", &(v.x), &(v.y));
		if(ret != 2)
		{
			infile.close();
			return -5;
		}

		/* store vertex */
		this->add(v);
	}

	/* read triangles */
	this->tris.reserve(num_tris);
	for(i = 0; i < num_tris; i++)
	{
		/* read next line and parse triangle */
		getline(infile, buf);
		ret = sscanf(buf.c_str(), "%d %d %d",
			&(t.verts[0]), &(t.verts[1]), &(t.verts[2]));
		if(ret != (int) NUM_VERTS_PER_TRI)
		{
			infile.close();
			return -6;
		}

		/* store triangle */
		this->add(t);
	}

	/* read rooms */
	this->rooms.reserve(num_rooms);
	for(i = 0; i < num_rooms; i++)
	{
		/* clear buffer struct */
		r.clear();
		
		/* read next line and parse room */
		getline(infile, buf);
		ss.clear();
		ss.str(buf);

		/* parse the room heights */
		ss >> r.min_z;
		ss >> r.max_z;
		ss >> num_tris;

		/* parse the triangles of this room */
		for(j = 0; j < num_tris; j++)
		{
			/* check that stream has more elements */
			if(ss.eof())
			{
				/* bad line */
				infile.close();
				return -7;
			}

			/* get next triangle index in this room */
			ss >> ti;
			r.tris.insert(ti);
		}

		/* add room to floorplan */
		this->add(r);
	}

	/* the input file goes on to define rooms, but we don't care
	 * about that in this context, so ignore it. */
	infile.close();

	/* associate neighboring triangles */
	this->map_neighbors();

	/* success */
	return 0;
}

void floorplan_t::add(const vertex_t& v)
{
	/* add to floorplan */
	this->verts.push_back(v);

	/* reset properties of new vertex */
	this->verts.back().ind = this->verts.size() - 1;
	this->verts.back().tri_neighs.clear();
}

void floorplan_t::add(const triangle_t& t)
{
	unsigned int ii, num_verts;
	int ti, vi;

	/* add to floorplan */
	this->tris.push_back(t);

	/* reset properties */
	ti = this->tris.size() - 1;
	this->tris.back().ind = ti;

	/* search for neighbors */
	num_verts = this->verts.size();
	for(ii = 0; ii < NUM_VERTS_PER_TRI; ii++)
	{
		/* get vertex index */
		vi = this->tris.back().verts[ii];
		
		/* check if in floorplan */
		if(vi < (int)num_verts && vi >= 0)
		{
			/* add new triangle to this vertex */
			this->verts[vi].tri_neighs.insert(ti);
		}
		else
		{
			/* invalid vertex */
			this->tris.back().verts[ii] = -1;
		}
	}

	/* set triangle neighbors to be invalid for now */
	for(ii = 0; ii < NUM_EDGES_PER_TRI; ii++)
		this->tris.back().neighs[ii] = -1;
}

void floorplan_t::add(const room_t& r)
{
	/* add to floorplan */
	this->rooms.push_back(r);

	/* reset properties */
	this->rooms.back().ind = this->rooms.size() - 1;
}
	
void floorplan_t::map_neighbors()
{
	set<int>::iterator tit;
	unsigned int ri, ii, ti, num_tris, num_rooms;
	int vi;

	/* iterate over triangles */
	num_tris = this->tris.size();
	for(ti = 0; ti < num_tris; ti++)
	{
		/* iterate over the vertices of
		 * this triangle */
		for(ii = 0; ii < NUM_VERTS_PER_TRI; ii++)
		{
			/* get vertex index */
			vi = this->tris[ti].verts[ii];

			/* iterate through neighboring triangles
			 * of this vertex */
			for(tit = this->verts[vi].tri_neighs.begin();
				tit != this->verts[vi].tri_neighs.end(); 
					tit++)
			{
				/* attempt to make ti and *tit neighbors */
				this->tris[ti].make_neighbors_with(
					this->tris[*tit]);
			}
		}
	}

	/* map room heights to vertices */
	num_rooms = this->rooms.size();
	for(ri = 0; ri < num_rooms; ri++)
	{
		/* iterate over triangle indices for this room */
		for(tit = this->rooms[ri].tris.begin();
			tit != this->rooms[ri].tris.end(); tit++)
		{
			/* iterate over the vertices of this triangle */
			for(ii = 0; ii < NUM_VERTS_PER_TRI; ii++)
			{
				/* get the current vertex */
				vi = this->tris[*tit].verts[ii];

				/* update the heights of this vertex
				 * based on the current room heights */
				if(this->verts[vi].min_z 
						>= this->verts[vi].max_z)
				{
					/* vertex heights have not yet
					 * been set, so set it to this
					 * room's */
					this->verts[vi].min_z 
						= this->rooms[ri].min_z;
					this->verts[vi].max_z 
						= this->rooms[ri].max_z;
				}
				else
				{
					/* compare this room's heights to
					 * the existing heights for this
					 * vertex.  If this room's heights
					 * are more restrictive, replace
					 * the vertex's heights */
					this->verts[vi].min_z
						= (this->verts[vi].min_z 
						< this->rooms[ri].min_z)
						? this->rooms[ri].min_z
						: this->verts[vi].min_z;
					this->verts[vi].max_z
						= (this->verts[vi].max_z 
						> this->rooms[ri].max_z)
						? this->rooms[ri].max_z
						: this->verts[vi].max_z;
				}
			}
		}
	}
}

/********** TRIANGLE_T I/O FUNCTIONS **************/

bool triangle_t::make_neighbors_with(triangle_t& other)
{
	set<int> my_verts;
	set<int> o_verts;
	vector<int> intersection;
	vector<int>::iterator vit;
	unsigned int ii;

	/* compute intersection of the vertices of these
	 * triangles. Must be two to share an edge */
	for(ii = 0; ii < NUM_VERTS_PER_TRI; ii++)
	{
		my_verts.insert(this->verts[ii]);
		o_verts.insert(other.verts[ii]);
	}
	intersection.resize(NUM_VERTS_PER_TRI);
	vit = set_intersection(my_verts.begin(), my_verts.end(),
				o_verts.begin(), o_verts.end(),
				intersection.begin());
	intersection.resize(vit - intersection.begin());

	/* check size of intersection */
	if(intersection.size() != NUM_VERTS_PER_EDGE)
		return false;

	/* these triangles share an edge, so update neighbor
	 * information */
	for(ii = 0; ii < NUM_EDGES_PER_TRI; ii++)
	{
		/* modify this triangle */
		if(this->verts[ii] != intersection[0] 
				&& this->verts[ii] != intersection[1])
		{
			this->neighs[ii] = other.ind;
			break;
		}
	}

	for(ii = 0; ii < NUM_EDGES_PER_TRI; ii++)
	{
		/* modify other triangle */
		if(other.verts[ii] != intersection[0]
				&& other.verts[ii] != intersection[1])
		{
			other.neighs[ii] = this->ind;
			break;
		}
	}

	/* successfully made neighbors */
	return true;
}
