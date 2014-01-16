#include "floorplan.h"
#include <vector>
#include <set>
#include <float.h>
#include "../util/parameters.h"

using namespace std;

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

void floorplan_t::compute_edges(vector<edge_t>& edges)
{
	int ti, ni, num_tris;

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
	
void floorplan_t::compute_bounds(double& min_x, double& min_y,
				double& max_x, double& max_y)
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

edge_t::~edge_t()
{
	/* no computation necessary */
}

/****** TRIANGLE FUNCTIONS ******/

triangle_t::triangle_t()
{
	int i;

	/* set to default values */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		this->verts[i] = -1;
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		this->neighs[i] = -1;
	this->ind = -1;
}

triangle_t::~triangle_t()
{
	/* no processing necessary */
}
	
edge_t triangle_t::get_edge(int ni) const
{
	edge_t e;

	/* check validity of input */
	if(ni >= 0 && ni < NUM_EDGES_PER_TRI)
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
