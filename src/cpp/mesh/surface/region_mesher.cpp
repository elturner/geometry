#include "region_mesher.h"
#include <io/mesh/mesh_io.h>
#include <xmlreader/xmlsettings.h>
#include <geometry/shapes/plane.h>
#include <geometry/octree/octree.h>
#include <mesh/triangulate/isostuff/region_isostuffer.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/surface/node_corner_map.h>
#include <mesh/surface/node_corner.h>
#include <image/color.h>
#include <util/error_codes.h>
#include <util/set_ops.h>
#include <Eigen/Dense>
#include <iostream>
#include <fstream>
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
#define XML_NODE_OUTLIERTHRESH   "octsurf_node_outlierthresh"
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
		this->node_outlierthresh = 1.0;
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
	if(settings.is_prop(XML_NODE_OUTLIERTHRESH))
		this->node_outlierthresh = settings.getAsDouble(
					XML_NODE_OUTLIERTHRESH);
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
	regionmap_t::const_iterator rit; /* access region_graph */
	vertmap_t::iterator vit; /* access this->vertices */
	faceset_t::const_iterator fit;
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
			ret = this->add_face(*fit, tree, region_graph,
						corner_map);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);
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

	/* success */
	return 0;
}
			
int mesher_t::add_face(const node_face_t& f, const octree_t& tree,
				const planar_region_graph_t& region_graph,
				const corner_map_t& corner_map)
{
	pair<faceset_t::const_iterator, faceset_t::const_iterator> faces;
	regionmap_t::const_iterator sit; /* access region_graph */
	pair<vertmap_t::iterator, bool> vins[NUM_CORNERS_PER_SQUARE]; 
					/* access this->vertices */
	bool corner_added[NUM_CORNERS_PER_SQUARE];
	faceset_t::const_iterator nit; /* access corner's neighbor faces */
	corner_t c, c_next; /* corners of f */
	vertex_info_t vinfo;
	size_t ci, ci_next;
	double res;

	/* for each face, iterate through corners */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* get the value of this corner */
		c.set(tree, f, ci);

		/* prepare info for this corner */
		vinfo.clear();
		c.get_position(tree, vinfo.position);

		/* get the faces that touch this corner */
		faces = corner_map.get_faces_for(c);
		
		/* check which regions each of these faces are in */
		for(nit = faces.first; nit != faces.second; nit++)
		{
			/* get the info for this face */
			sit=region_graph.lookup_face(*nit);
			if(sit == region_graph.end())
				return -1;

			/* record this region as intersecting this corner */
			vinfo.add(sit->first);
		}

		/* check if we care about this corner
		 * (we only care if it touches multiple regions) */
		if(vinfo.size() < 2)
		{
			corner_added[ci] = false;
			continue; /* don't care */
		}
		corner_added[ci] = true;
				
		/* if there are multiple regions touching
		 * this corner, then we should record it
		 *
		 * if we have already seen this corner,
		 * then don't bother continuing. */
		vins[ci] = this->vertices.insert(pair<corner_t,
					vertex_info_t>(c,vinfo));
		if(!(vins[ci].second))
		{
			/* this corner already exists
			 * in our map, just add region
			 * set and don't bother with 
			 * the rest */
			vins[ci].first->second.add(vinfo);
			continue;
		}
	}

	/* now check between adjacent corners that were added
	 * as boundary vertices.  If two corners share multiple
	 * common regions, AND the face is larger than the min
	 * resolution of the tree, then there should be other
	 * corners between the ones we just added.
	 *
	 * If those corners do not exist in the corner map, the
	 * we want to create them for this data structure. */
	res = tree.get_resolution();
	if(2*f.get_halfwidth() <= res)
		return 0; /* no space for extra verts to exist */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* get index of next corner */
		ci_next = (ci + 1) % NUM_CORNERS_PER_SQUARE;

		/* check if both this and the next corner were
		 * added as boundary vertices */
		if(!corner_added[ci] || !corner_added[ci_next])
			continue; /* ignore this pair */

		/* these two corners are both boundary vertices.  Before
		 * continuing, we want to make sure that they share
		 * multiple regions in common. */
		set_ops::intersect(vinfo.regions, 
				vins[ci].first->second.begin(),
				vins[ci].first->second.end(),
				vins[ci_next].first->second.begin(),
				vins[ci_next].first->second.end() );
		if(vinfo.size() < 2)
			continue; /* don't bother with this */

		/* iterate from this corner to the next, checking
		 * that all boundary vertices in between are properly
		 * defined. */
		c      = vins[ci].first->first;
		c_next = vins[ci_next].first->first;
		c.increment_towards(c_next); /* only look exclusive */
		for( ; c != c_next; c.increment_towards(c_next))
		{
			/* check if a boundary vertex exists at c's
			 * location already */
			faces = corner_map.get_faces_for(c);
			if(faces.first != faces.second)
			{
				/* a corner is already defined here */
				continue;
			}

			/* no corner is defined here, so we want to
			 * add it to our boundary vertex list */
			c.get_position(tree, vinfo.position);
			this->vertices.insert(pair<corner_t,
					vertex_info_t>(c,vinfo));
		}
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
			
int mesher_t::compute_mesh(mesh_io::mesh_t& mesh,
					const octree_t& tree) const
{
	planemap_t::const_iterator pit;
	cornerset_t::const_iterator cit;
	vertmap_t::const_iterator vit;
	map<corner_t, size_t> vert_inds;
	pair<map<corner_t, size_t>::iterator, bool> ins;
	mesh_io::vertex_t v;
	int ret;

	/* iterate over the vertices in this mesher.  We want to add them
	 * to the output mesh */
	for(vit = this->vertices.begin(); vit != this->vertices.end();vit++)
	{
		/* attempt to add this vertex to our map */
		ins = vert_inds.insert(pair<corner_t, size_t>(
					vit->first, vert_inds.size()));
		if(!(ins.second))
			return -1; /* already in map? */

		/* copy the position to the vertex structure */
		v.x = vit->second.position(0);
		v.y = vit->second.position(1);
		v.z = vit->second.position(2);

		/* insert into mesh */
		mesh.add(v);
	}

	/* now that we've inserted all the vertices, we can go
	 * through the regions and add triangles */
	for(pit = this->regions.begin(); pit != this->regions.end(); pit++)
	{
		/* make triangles for this region */
		ret = pit->second.compute_mesh_isostuff(mesh, 
						vert_inds, tree);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}
		
	/* success */
	return 0;
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
}

int region_info_t::compute_mesh_isostuff(mesh_io::mesh_t& mesh,
				const std::map<node_corner::corner_t,
						size_t>& vert_ind,
					const octree_t& tree) const
{
	region_isostuffer_t isostuff;
	int ret;

	/* check if this region is empty */
	if(this->region_it->second.get_region().num_faces() == 0)
		return 0; /* do nothing */

	/* represent the geometry of this region by forming a quadtree
	 * representation of the interior area of the planar region */
	ret = isostuff.populate(tree, *this, vert_ind);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* we now want to generate a triangulation of the region
	 * based on the geometry represented by the quadtree. */
	ret = isostuff.triangulate(mesh, vert_ind);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
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
