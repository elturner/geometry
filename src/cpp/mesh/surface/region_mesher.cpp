#include "region_mesher.h"
#include <io/mesh/mesh_io.h>
#include <geometry/shapes/plane.h>
#include <geometry/octree/octree.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/surface/node_corner_map.h>
#include <mesh/surface/node_corner.h>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <map>
#include <set>

/**
 * @file    region_mesher.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Generates a watertight mesh based on a set of planar regions
 *
 * @section DESCRIPTION
 *
 * This file contains the region_mesher_t class, which generates a unified
 * mesh based on a set of planar regions.  The mesh will be aligned with
 * the planar geometry described by the regions, and will (attempt to) use
 * an efficient number of triangles to represent these surfaces.
 */

using namespace std;
using namespace Eigen;
using namespace region_mesher;
using namespace node_corner;

/*-----------------------------------*/
/* mesher_t function implementations */
/*-----------------------------------*/
			
mesher_t::mesher_t()
{
	/* don't need to do anything here */
}
			
mesher_t::~mesher_t()
{
	/* free all memory */
	this->clear();
}
			
void mesher_t::clear()
{
	/* clear all elements */
	this->vertices.clear();
	this->regions.clear();
}
			
int mesher_t::init(const octree_t& tree,
			const planar_region_graph_t& region_graph,
			const corner_map_t& corner_map)
{
	pair<planemap_t::iterator, bool> ins;
	regionmap_t::const_iterator rit, sit;
	faceset_t::const_iterator fit, nit;
	pair<faceset_t::const_iterator, faceset_t::const_iterator> faces;
	corner_t c;
	vertex_info_t vinfo;
	pair<vertmap_t::iterator, bool> vins;
	size_t ci;

	/* clear any existing data */
	this->clear();

	/* iterate through all regions in this graph */
	for(rit = region_graph.begin(); rit != region_graph.end(); rit++)
	{
		/* store this region information */
		ins = this->regions.insert(pair<node_face_t, region_info_t>(
			rit->first, region_info_t(
				rit->second.get_region().get_plane())));
		if(!(ins.second))
			return -1; /* unable to insert region info */

		/* initialize the list of vertices of this region to
		 * be empty */
		ins.first->second.boundaries.resize(1);

		/* for each region, iterate through its faces, in
		 * order to iterate through its corners */
		for(fit = rit->second.get_region().begin();
			fit != rit->second.get_region().end(); fit++)
		{
			/* for each face, iterate through corners */
			for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
			{
				/* get the value of this corner */
				c.set(tree, *fit,
					node_corner::get_face_corner(
						fit->direction, ci));

				/* prepare info for this corner */
				vinfo.clear();
				c.get_position(tree, vinfo.position);

				/* get the faces that touch this
				 * corner */
				faces = corner_map.get_faces_for(c);

				/* check which regions each of these
				 * faces are in */
				for(nit = faces.first; nit != faces.second;
						nit++)
				{
					/* get the info for this face */
					sit=region_graph.lookup_face(*nit);
					if(sit == region_graph.end())
						return -2;

					/* record this region as
					 * intersecting this corner */
					vinfo.add(sit->first);
				}

				/* check if we care about this corner
				 * (we only care if it touches multiple
				 * regions) */
				if(vinfo.size() < 2)
					continue; /* don't care */

				/* if there are multiple regions touching
				 * this corner, then we should record it
				 *
				 * if we have already seen this corner,
				 * then don't bother continuing. */
				vins = this->vertices.insert(
						pair<corner_t,
						vertex_info_t>(c,vinfo));
				if(!(vins.second))
				{
					/* this corner already exists
					 * in our map, just add region
					 * set and don't bother with 
					 * the rest */
					vins.first->second.add(vinfo);
					continue;
				}

				/* if this corner wasn't seen before, then
				 * add it to the info structure for
				 * this region. */
				ins.first->second.boundaries[0]
							.push_back(c);
			}
		}
	}

	/* at this point, all the corners know which regions they
	 * touch, and all the regions know which corners they touch,
	 * but the regions do NOT yet know what the appropriate order
	 * of their boundary vertices is yet. */
	// TODO

	/* success */
	return 0;
}
			
int mesher_t::compute_mesh(mesh_io::mesh_t& mesh) const
{
	return -1; // TODO implement me
}
			
void mesher_t::writeobj_vertices(std::ostream& os) const
{
	vertmap_t::const_iterator it;

	/* iterate over the vertices in this structure */
	for(it = this->vertices.begin(); it != this->vertices.end(); it++)
		os << "v " << it->second.position.transpose() << endl;
}

/*----------------------------------------*/
/* vertex_info_t function implementations */
/*----------------------------------------*/
			
vertex_info_t::vertex_info_t()
{
	// TODO
}

vertex_info_t::~vertex_info_t()
{
	// TODO
}
			
void vertex_info_t::clear()
{
	/* remove any referenced regions */
	this->regions.clear();
}

/*----------------------------------------*/
/* region_info_t function implementations */
/*----------------------------------------*/
			
region_info_t::region_info_t()
{
	// TODO
}

region_info_t::~region_info_t()
{
	// TODO
}

