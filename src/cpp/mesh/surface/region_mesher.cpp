#include "region_mesher.h"
#include <io/mesh/mesh_io.h>
#include <xmlreader/xmlsettings.h>
#include <geometry/shapes/plane.h>
#include <geometry/octree/octree.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/surface/node_corner_map.h>
#include <mesh/surface/node_corner.h>
#include <image/color.h>
#include <util/error_codes.h>
#include <util/set_ops.h>
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

/* the following defines are used to access parameters stored
 * in the .xml settings file. */
#define XML_COALESCE_DISTTHRESH  "octsurf_coalesce_distthresh"
#define XML_COALESCE_PLANETHRESH "octsurf_coalesce_planethresh"
#define XML_USE_ISOSURFACE_POS   "octsurf_use_isosurface_pos"
#define XML_MIN_SINGULAR_VALUE   "octsurf_min_singular_value"
#define XML_MAX_COLINEARITY      "octsurf_max_colinearity"

/*-----------------------------------*/
/* mesher_t function implementations */
/*-----------------------------------*/
			
mesher_t::mesher_t()
{
	/* set default parameters */
	this->import(string(""));
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
			
int mesher_t::import(const std::string& xml_settings)
{
	XmlSettings settings;

	/* check if the file is empty */
	if(xml_settings.empty())
	{
		/* no file provided, use default settings */
		this->coalesce_distthresh = 2.0;
		this->coalesce_planethresh = 0.0;
		this->use_isosurface_pos = false;
		this->min_singular_value = 0.1;
		this->max_colinearity = 0.99;
		return 0;
	}

	/* open and parse xml file */
	if(!settings.read(xml_settings))
	{
		/* unable to open file */
		cerr << "[mesher_t::import]\tUnable to import xml settings "
		     << "from: " << xml_settings << endl;
		return -1;
	}

	/* read in the settings information */
	if(settings.is_prop(XML_COALESCE_DISTTHRESH))
		this->coalesce_distthresh = settings.getAsDouble(
					XML_COALESCE_DISTTHRESH);
	if(settings.is_prop(XML_COALESCE_PLANETHRESH))
		this->coalesce_planethresh = settings.getAsDouble(
					XML_COALESCE_PLANETHRESH);
	if(settings.is_prop(XML_USE_ISOSURFACE_POS))
		this->use_isosurface_pos = settings.getAsUint(
					XML_USE_ISOSURFACE_POS);
	if(settings.is_prop(XML_MIN_SINGULAR_VALUE))
		this->min_singular_value = settings.getAsDouble(
					XML_MIN_SINGULAR_VALUE);
	if(settings.is_prop(XML_MAX_COLINEARITY))
		this->max_colinearity = settings.getAsDouble(
					XML_MAX_COLINEARITY);

	/* success */
	return 0;
}
			
int mesher_t::init(const octree_t& tree,
			const planar_region_graph_t& region_graph,
			const corner_map_t& corner_map)
{
	pair<planemap_t::iterator, bool> ins; /* access this->regions */
	planemap_t::iterator pit; /* access this->regions */
	regionmap_t::const_iterator rit, sit; /* access region_graph */
	pair<vertmap_t::iterator, bool> vins; /* access this->vertices */
	vertmap_t::iterator vit; /* access this->vertices */
	faceset_t::const_iterator fit, nit;
	pair<faceset_t::const_iterator, faceset_t::const_iterator> faces;
	corner_t c;
	vertex_info_t vinfo;
	size_t ci;
	int ret;

	/* clear any existing data */
	this->clear();

	/* iterate through all regions in this graph */
	for(rit = region_graph.begin(); rit != region_graph.end(); rit++)
	{
		/* store this region information */
		ins = this->regions.insert(pair<node_face_t, region_info_t>(
			rit->first, region_info_t(rit)));
		if(!(ins.second))
			return -1; /* unable to insert region info */

		/* for each region, iterate through its faces, in
		 * order to iterate through its corners */
		for(fit = rit->second.get_region().begin();
			fit != rit->second.get_region().end(); fit++)
		{
			/* for each face, iterate through corners */
			for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
			{
				/* get the value of this corner */
				c.set(tree, *fit, ci);

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
			}
		}
	}

	/* for each of the vertices we just added, we need to add
	 * them to their respective regions, so that each region can
	 * know which vertices it contains.
	 *
	 * For now, we don't care about getting the order right, just
	 * that the list is complete.
	 */
	for(vit=this->vertices.begin(); vit!=this->vertices.end(); vit++)
	{
		/* iterate over the regions that intersect this vertex */
		for(fit=vit->second.begin(); fit!=vit->second.end(); fit++)
		{
			/* get the region info for the current seed face */
			pit = this->regions.find(*fit);
			if(pit == this->regions.end())
				return -3; /* this region SHOULD exist */

			/* add the current vertex to this region */
			pit->second.add(vit->first);
		}

		/* now that we have prepared this vertex, we can compute
		 * its ideal 3D position based on the set of regions
		 * that intersect it. */
 		ret = this->compute_vertex_pos(vit);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);
	}

	/* at this point, all the corners know which regions they
	 * touch, and all the regions know which corners they touch,
	 * but the regions do NOT yet know what the appropriate order
	 * of their boundary vertices is yet. */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
	{
		/* analyze this region to find its boundary */
		ret = pit->second.populate_boundaries(corner_map);
		if(ret)
			return PROPEGATE_ERROR(-5, ret);
	}

	/* success */
	return 0;
}
			
int mesher_t::compute_vertex_pos(vertmap_t::iterator vit)
{
	faceset_t::const_iterator fit;
	planemap_t::const_iterator rit;
	size_t i, num_regions, num_dims;
	double thresh, s;
	Vector3d x, v;

	/* how we project this vertex position is based on 
	 * how many regions intersect it.
	 *
	 * If we make the normal vectors of all of these planes
	 * into the rows of a matrix, the null space of that matrix
	 * represents the variance */
	num_dims = 3;
	num_regions = vit->second.size();
	MatrixXd N(num_regions, num_dims);
	VectorXd P(num_regions);
	for(fit=vit->second.begin(), i=0; fit!=vit->second.end(); fit++,i++)
	{
		/* get the information for this region */
		rit = this->regions.find(*fit);
		if(rit == this->regions.end())
			return -1;

		/* add the normal vector of this plane to our matrix */
		N.block<1,3>(i,0) 
			= rit->second.get_plane().normal.transpose();
	
		/* add the plane offset to the RHS of the equation */
		P(i) = rit->second.get_plane().normal.dot(
				rit->second.get_plane().point);
	}

	/* Solve for the null space of this matrix by taking the SVD.
	 * Since the columns of the U matrix are ordered in descending
	 * order, all the small (or zero) eigenvalues will be at the end
	 */
	JacobiSVD<MatrixXd> svd(N, 
			Eigen::ComputeFullV | Eigen::ComputeThinU);

	/* The number of "large" eigenvalues determines the number of
	 * constraints on this vertex position.
	 *
	 * 	1 "large" eigenvalue  --> a plane
	 * 	2 "large" eigenvalues --> a line (intersect two planes)
	 * 	3 "large" eigenvalues --> a point (intesect three planes)
	 *
	 * Large is determined by the threshold min_singular_value
	 *
	 * We iterate over the basis vectors described in V in order
	 * to have the kernel contribute to the least-squares solution
	 * for the intersection position.
	 */
	VectorXd S = svd.singularValues();
	MatrixXd U = svd.matrixU();

	/* first initialize our threshold values and output vector
	 * to prepare for the least-squares solution */
	thresh = this->min_singular_value * S(0);
	x << 0,0,0;

	/* iterate over the basis vectors in order to exercise the
	 * null space to get our svd solution as close to the initial
	 * condition as possible */
	for(i = 0; i < num_dims; i++)
	{
		/* get current basis vector */
		v = svd.matrixV().block<3,1>(0,i);
		s = (S.rows() <= (int) i) ? 0.0 : S(i);

		/* Is this basis vector part of (our definition of)
		 * the null space? */
		if(s < thresh)
		{
			/* part of the null space, so this basis vector
			 * should NOT contribute to the intersection
			 * position.  Instead, we should try to emulate
			 * the original corner position in this dimension.
			 */
			x += vit->second.position.dot(v) * v;
		}
		else
		{
			/* this vector is part of the kernal, and
			 * should contribute to the least-squares
			 * solution */
			VectorXd u = U.block(0,i,num_regions,1);
			x += (P.dot(u) / s) * v;
		}
	}

	/* set the vertex position to be the value computed */
	vit->second.position = x;

	/* success */
	return 0;
}
			
int mesher_t::simplify()
{
	map<corner_t, size_t> simplify_counts;
	map<corner_t, size_t>::iterator cit;
	cornerset_t to_remove;
	cornerset_t::iterator rit;
	planemap_t::iterator pit;
	vertmap_t::const_iterator vit, vit_prev, vit_next;
	vector<corner_t>::iterator bit, bitalt;
	Vector3d edge_prev, edge_next;
	size_t bi, num_boundaries, vi, vi_prev, vi_next, num_verts;
	double c_ang;

	/* prep the map of simplification counts.  This map
	 * stores how many regions vote to simplify each vertex.
	 *
	 * If all the regions that are connected to a vertex vote to
	 * simplify it, then it will be simplified. */
	for(vit=this->vertices.begin(); vit != this->vertices.end(); vit++)
		simplify_counts.insert(
			pair<corner_t, size_t>(vit->first, 0));

	/* iterate over the regions */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
	{
		/* iterate over the boundaries of this region */
		num_boundaries = pit->second.boundaries.size();
		for(bi = 0; bi < num_boundaries; bi++)
		{
			/* check if boundary is big enough to be 
			 * simplified.  Note that you need at least
			 * three elements to make a non-trivial boundary */
			num_verts = pit->second.boundaries[bi].size();
			if(num_verts < 3)
				continue;
			
			/* iterate over the vertices of this boundary */
			for(vi = 0; vi < num_verts; vi++)
			{
				/* get the info for this vertex */
				vit = this->vertices.find(
					pit->second.boundaries[bi][vi]);
				if(vit == this->vertices.end())
					return -1; /* bad vertex */

				/* for this vertex, we want to check
				 * if it is shared by at most two
				 * regions (including this one) */
				if(vit->second.size() > 2)
					continue; /* don't simplify 
						   * corners */

				/* now we must check this vertex's
				 * position against its neighbors */

				/* get neighbors */
				vi_prev = (vi + num_verts - 1) % num_verts;
				vi_next = (vi + 1)             % num_verts;

				/* retrieve neighbor info */
				vit_prev = this->vertices.find(
						pit->second.boundaries[
							bi][vi_prev]);
				if(vit_prev == this->vertices.end())
					return -2;
				vit_next = this->vertices.find(
						pit->second.boundaries[
							bi][vi_next]);
				if(vit_next == this->vertices.end())
					return -3;

				/* check how colinear these points are 
				 * by computing the magnitude of the
				 * cosine between their vectors */
				edge_prev = 
					(vit_prev->second.get_position() 
					- vit->second.get_position());
				edge_prev.normalize();
				edge_next = 
					(vit_next->second.get_position() 
					- vit->second.get_position());
				edge_next.normalize();
				c_ang = abs(edge_prev.dot(edge_next)); 
				
				/* check this value against our threshold */
				if(c_ang < this->max_colinearity)
					continue; /* can't simplify */

				/* this region votes to simplify this
				 * vertex out of existence, so we update
				 * the vote map */
				simplify_counts[vit->first]++;
			}
		}
	}

	/* iterate over the map of simplification votes, and find
	 * which vertices can be simplified */
	for(cit = simplify_counts.begin(); 
			cit != simplify_counts.end(); cit++)
	{
		/* get this vertex's info */
		vit = this->vertices.find(cit->first);
		if(vit == this->vertices.end())
			return -4;

		/* check if all regions that contain this vertex
		 * voted to have it simplified */
		if(vit->second.size() > cit->second)
			continue; /* not enough votes */

		/* this vertex has been voted off the island. */
		to_remove.insert(cit->first);
	}

	/* iterate through the regions, removing these vertices */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
	{
		/* iterate over the boundaries of this region */
		num_boundaries = pit->second.boundaries.size();
		for(bi = 0; bi < num_boundaries; bi++)
		{
			/* iterate over the vertices of this boundary */
			for(bit = pit->second.boundaries[bi].begin(); 
				bit != pit->second.boundaries[bi].end(); ) 
			{
				/* check if this vertex should be removed */
				if(to_remove.count(*bit) > 0)
				{
					/* remove the vertex from this
					 * boundary */
					bitalt = bit;
					bit = pit->second.boundaries
							[bi].erase(bitalt);
				}
				else
					bit++;
			}
		}

		/* remove the vertices from this region */
		for(rit = to_remove.begin(); rit != to_remove.end(); rit++)
			pit->second.vertices.erase(*rit);
	}

	/* success */
	return 0;
}
			
int mesher_t::compute_mesh(mesh_io::mesh_t& mesh) const
{
	return -1; // TODO implement me
}
			
int mesher_t::writeobj_vertices(std::ostream& os) const
{
	planemap_t::const_iterator pit;
	vertmap_t::const_iterator vit;
	faceset_t::const_iterator fit;
	Vector3d p;
	color_t c;
	int i, n;
	
	for(vit=this->vertices.begin(); vit!=this->vertices.end(); vit++)
	{
		/* export vertex center */
		c.set_random();
		p = vit->second.position;
		os << "v " << p.transpose() 
		   <<  " " << c.get_red_int()
		   <<  " " << c.get_green_int()
		   <<  " " << c.get_blue_int() << endl;

		/* iterate over the regions that intersect this vertex */
		for(fit=vit->second.begin(); fit!=vit->second.end(); fit++)
		{
			/* get the region info for the current seed face */
			pit = this->regions.find(*fit);
			if(pit == this->regions.end())
				return -1; /* this region SHOULD exist */

			/* project the point onto this region's plane */
			p = vit->second.position;
			pit->second.get_plane().project_onto(p);

			/* export it */
			os << "v " << p.transpose()
			   << " 255 255 255" << endl;
		}

		/* export some triangles */
		n = (int) vit->second.size();
		for(i = -1; i >= -n; i--)
			os << "f " << (-n-1) 
			   <<  " " << i
			   <<  " " << (i == -n ? -1 : i-1)
			   << endl;
	}

	/* success */
	return 0;
}
			
int mesher_t::writecsv(std::ostream& os) const
{
	planemap_t::const_iterator pit;
	int ret;

	/* iterate over the regions */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
	{
		/* export this region */
		ret = pit->second.writecsv(os, this->vertices);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}
			
void mesher_t::writeobj_edges(std::ostream& os, const octree_t& tree,
				const node_corner::corner_map_t& cm) const
{
	planemap_t::const_iterator pit;

	/* iterate over the regions */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
		pit->second.writeobj_edges(os, tree, cm);
}

/*----------------------------------------*/
/* vertex_info_t function implementations */
/*----------------------------------------*/
			
vertex_info_t::vertex_info_t()
{ /* don't need to do anything here */ }

vertex_info_t::~vertex_info_t()
{ this->clear(); }
			
void vertex_info_t::clear()
{
	/* remove any referenced regions */
	this->regions.clear();
}

/*----------------------------------------*/
/* region_info_t function implementations */
/*----------------------------------------*/
			
region_info_t::~region_info_t()
{
	this->clear();
}

void region_info_t::clear()
{
	this->vertices.clear();
	this->boundaries.clear();
}
			
int region_info_t::populate_boundaries(
				const node_corner::corner_map_t& cm)
{
	faceset_t common_faces, common_region_faces; 
	pair<cornerset_t::const_iterator, 
				cornerset_t::const_iterator> range;
	pair<faceset_t::const_iterator, faceset_t::const_iterator>
				my_faces, neigh_faces;
	cornerset_t::const_iterator nit;
	cornerset_t unused;
	corner_t c;
	size_t curr_ring;
	bool foundnext;
	
	/* clear any existing boundary info */
	this->boundaries.clear();
	unused.insert(this->vertices.begin(), this->vertices.end());

	/* as long as we have corners left unused, take
	 * the next corner and start a new boundary ring */
	while(!(unused.empty()))
	{
		/* start a new boundary ring */
		curr_ring = this->boundaries.size();
		this->boundaries.resize(curr_ring+1);

		/* pick the next corner to start this ring */
		c = *(unused.begin());
		foundnext = true;
		
		/* iterate over the current ring as long as we can */
		while(foundnext)
		{
			/* add this corner to the boundary */
			this->boundaries[curr_ring].push_back(c);
			unused.erase(c);
		
			/* get the neighbors of this corner */
			range = cm.get_edges_for(c);
			my_faces = cm.get_faces_for(c);

			/* search for the next valid corner in set 
			 * of neighbors */
			foundnext = false;
			for(nit = range.first; nit != range.second; nit++)
			{
				/* check if this neighbor corner is
				 * one of our unused vertices */
				if(unused.count(*nit) == 0)
					continue; /* it's not */

				/* check if this neighbor actually forms
				 * a valid BOUNDARY edge for this region,
				 * which requires these two corners to
				 * only share one face in the region */
				neigh_faces = cm.get_faces_for(*nit);
				set_ops::intersect(common_faces,
						my_faces.first,
						my_faces.second,
						neigh_faces.first,
						neigh_faces.second);
				set_ops::intersect(common_region_faces,
					common_faces.begin(), 
					common_faces.end(), 
					this->region_it->
						second.get_region().begin(),
					this->region_it->
						second.get_region().end());
				if(common_region_faces.size() != 1)
					continue; /* too many shared */

				/* this neighbor is part of the region,
				 * and not yet used, so we should continue
				 * the boundary in this direction */
				c = *nit;
				foundnext = true;
				break;
			}
		}
	}
	
	/* success */
	return 0;
}
			
int region_info_t::writecsv(std::ostream& os, const vertmap_t& vm) const
{
	vertmap_t::const_iterator vit, vit_first;
	size_t bi, vi, num_boundaries, num_verts;

	/* iterate over the boundaries in this region */
	num_boundaries = this->boundaries.size();
	for(bi = 0; bi < num_boundaries; bi++)
	{
		/* iterate over the vertices in this boundary */
		num_verts = this->boundaries[bi].size();
		for(vi = 0; vi < num_verts; vi++)
		{
			/* get info for this vertex */
			vit = vm.find(this->boundaries[bi][vi]);
			if(vit == vm.end())
				return -1;
			if(vi == 0)
				vit_first = vit;

			/* add info to stream */
			os << vit->second.get_position()(0) << ","
			   << vit->second.get_position()(1) << ","
			   << vit->second.get_position()(2) << ",";	
		}

		/* end the line by repeating first vertex */
		if(num_verts > 0)
			os << vit_first->second.get_position()(0) << ","
			   << vit_first->second.get_position()(1) << ","
			   << vit_first->second.get_position()(2);
			
		/* add newline between boundaries within a region */
		os << endl;
	}

	/* success */
	os << endl;
	return 0;
}
			
void region_info_t::writeobj_edges(std::ostream& os, const octree_t& tree,
				const node_corner::corner_map_t& cm) const
{
	cornerset_t::const_iterator cit;

	/* iterate over the vertices of this region's boundary */
	for(cit=this->vertices.begin(); cit != this->vertices.end(); cit++)
	{
		/* write out corner edge info */
		cm.writeobj_edges(os, tree, *cit);
	}
}
