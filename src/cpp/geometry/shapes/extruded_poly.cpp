#include "extruded_poly.h"
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <mesh/floorplan/floorplan.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <Eigen/Dense>

/**
 * @file extruded_poly.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the extruded_poly_t, which implements
 * the shape_t interface for octrees.  It is used to intersect
 * a room from an extruded floor plan with an octree.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

extruded_poly_t::extruded_poly_t()
{
	/* set default values */
	this->room_index = -1;
}
		
void extruded_poly_t::init(const fp::floorplan_t& f,
		           unsigned int gi, unsigned int ri)
{
	map<int, unsigned int> vert_map;
	map<int, unsigned int>::iterator vmit;
	pair<map<int, unsigned int>::iterator, bool> vit;
	set<int>::iterator tit;
	vector<fp::edge_t> orig_edges;
	unsigned int ei, vii, num_tris, ti, num_edges;
	int vi;

	/* save global room index */
	this->room_index = gi;

	/* save height information about this room */
	this->floor_height = f.rooms[ri].min_z;
	this->ceiling_height = f.rooms[ri].max_z;

	/* copy over vertex positions, and generate a mapping
	 * between vertex indices in the floor plan, and vertex
	 * indices to use for this room in isolation. */
	vert_map.clear();

	/* we will also copy over the triangle information */
	num_tris = f.rooms[ri].tris.size();
	this->tris.resize(3, num_tris);

	/* iterate over the triangles of this room */
	ti = 0; /* keep track of new triangle index */
	for(tit = f.rooms[ri].tris.begin();
			tit != f.rooms[ri].tris.end(); tit++)
	{
		/* iterate through vertices associated with current
		 * triangle */
		for(vii = 0; vii < fp::NUM_VERTS_PER_TRI; vii++)
		{
			/* get current vertex index */
			vi = f.tris[*tit].verts[vii];

			/* attempt to insert this vertex. if this vertex
			 * has been seen before, then the size of the
			 * map will not change */
			vit = vert_map.insert(
				make_pair<int, unsigned int>(vi,
					vert_map.size()));
		
			/* copy triangle value into this structure */
			this->tris(vii, ti) = vit.first->second;
		}

		/* move to next triangle */
		ti++;
	}

	/* now that we know the number of unique vertices used
	 * by this room, resize the verts matrix to be appropriate
	 * and populate it with vertex positions */
	this->verts.resize(3, vert_map.size());
	for(vmit = vert_map.begin(); vmit != vert_map.end(); vmit++)
	{
		/* copy original vertex info into this structure
		 * at new index */
		this->verts(0,vmit->second) = f.verts[vmit->first].x;
		this->verts(1,vmit->second) = f.verts[vmit->first].y;
		this->verts(2,vmit->second) = this->floor_height;
	}

	/* compute boundary edges of room, and convert the vertex
	 * indices into the new indices for this object using
	 * the vertex map */
	f.compute_edges_for_room(orig_edges, ri);
	num_edges = orig_edges.size();
	this->edges.resize(2, num_edges);
	for(ei = 0; ei < num_edges; ei++)
	{
		this->edges(0,ei) = vert_map[orig_edges[ei].verts[0]];
		this->edges(1,ei) = vert_map[orig_edges[ei].verts[1]];
	}
}
		
Vector3d extruded_poly_t::get_vertex(unsigned int i) const
{
	Vector3d v;
	bool isceil;

	/* get the appropriate vertex */
	isceil = (((int) i) > this->verts.cols());/* vertex is on ceiling */
	v = this->verts.block(0, (i%this->verts.cols()), 3, 1);

	/* modify it if it's on the ceiling */
	if(isceil)  v(3,1) = this->ceiling_height;

	/* return this vertex position */
	return v;
}
		
bool extruded_poly_t::intersects(const Vector3d& c, double hw) const
{
	/* check intersection along z */
	if(c(2) - hw > this->ceiling_height 
			|| c(2) + hw < this->floor_height)
		return false; /* no intersection possible */
	
	/* now we only have to worry about 2D intersection with an
	 * axis-aligned bounding box */
	// TODO left off here
	
	// TODO
}
		
octdata_t* extruded_poly_t::apply_to_leaf(const Vector3d& c,
                                          double hw, octdata_t* d) const
{
	/* check if leaf is non-null */
	if(d != NULL)
		d->set_fp_room(this->room_index); /* set room label */
	return d;
}
		
void extruded_poly_t::writeobj(std::ostream& os) const
{
	unsigned int i, n, m, num_verts_written, num_edges;

	/* write out the vertices of this shape */
	n = this->verts.cols();
	for(i = 0; i < n; i++)
		os << "v " << this->verts(0, i)
		   <<  " " << this->verts(1, i)
		   <<  " " << this->verts(2, i)
		   << endl;

	/* also export ceiling vertices */
	for(i = 0; i < n; i++)
		os << "v " << this->verts(0, i)
		   <<  " " << this->verts(1, i)
		   <<  " " << this->ceiling_height
		   << endl;

	/* export triangulation of floor and ceiling */
	m = this->tris.cols();
	num_verts_written = 2*n;
	for(i = 0; i < m; i++)
	{
		/* export triangle for floor */
		os << "f -" << (num_verts_written - this->tris(0, i))
		   <<  " -" << (num_verts_written - this->tris(1, i))
		   <<  " -" << (num_verts_written - this->tris(2, i))
		   << endl;

		/* export triangle for ceiling (flip orientation) */
		os << "f -" << (n - this->tris(2, i))
		   <<  " -" << (n - this->tris(1, i))
		   <<  " -" << (n - this->tris(0, i))
		   << endl;
	}

	/* export walls */
	num_edges = this->edges.cols();
	for(i = 0; i < num_edges; i++)
	{
		/* generate rectangle for this wall */
		os << "f -" << (num_verts_written - this->edges(0,i))
		   <<  " -" << (n - this->edges(0,i))
		   <<  " -" << (n - this->edges(1,i))
		   << endl
		   << "f -" << (num_verts_written - this->edges(0,i))
		   <<  " -" << (n - this->edges(1,i))
		   <<  " -" << (num_verts_written - this->edges(1,i))
		   << endl;
	}
}
