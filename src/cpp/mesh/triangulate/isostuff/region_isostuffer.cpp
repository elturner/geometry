#include "region_isostuffer.h"
#include <geometry/shapes/plane.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octtopo.h>
#include <geometry/quadtree/quadtree.h>
#include <mesh/surface/region_mesher.h>
#include <mesh/surface/planar_region.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner.h>
#include <io/mesh/mesh_io.h>
#include <image/color.h>
#include <util/error_codes.h>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <iostream>
#include <map>

/**
 * @file   region_isostuffer.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @breif  Performs 2D isosurface stuffing for planar region geometry
 *
 * @section DESCRIPTION
 *
 * This file contains the region_isostuffer_t class, which is
 * used to represent the mesh generated by running 2D isosurface
 * stuffing on a planar region in order to represent its geometry
 * in a final model.
 *
 * Note that this technique was original used in Turner and Zakhor 2013,
 * at 3DV 2013.
 */

using namespace std;
using namespace Eigen;
using namespace node_corner;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void region_isostuffer_t::clear()
{
	/* clear all info */
	this->quadtree.clear();
	this->vert2d_ind.clear();
}
		
int region_isostuffer_t::populate(const octree_t& octree, 
				const region_mesher::region_info_t& reginfo,
				const map<corner_t, size_t>& vert3d_ind)
{
	map<corner_t, size_t>::const_iterator vit;
	cornerset_t::const_iterator cit;
	faceset_t::const_iterator fit;
	octtopo::CUBE_FACE f, opp_f;
	Vector3d p3d;
	Vector2d center, p2d;
	corner_t corner2;
	double radius, res, hw, err;
	bool domdir;

	/* clear any existing info */
	this->clear();

	/* get the properties of this region (size and orientation) */
	const planar_region_t& region = reginfo.get_region();
	f = region.find_dominant_face();
	opp_f = octtopo::get_opposing_face(f);
	this->mapping_matrix(f);
	this->plane = region.get_plane();

	/* prepare to construct quadtree:
	 *
	 * get the radius as a power of two of the resolution 
	 * map center to 2D coordinates */
	res = octree.get_resolution();
	radius = octree.get_root()->halfwidth;
	center = this->M * octree.get_root()->center;
	err = res / 4; /* everything is larger than this length */

	/* we are going to use a quadtree to generate the mesh of this
	 * region.  This is the same method used in Turner and Zakhor
	 * at 3DV 2013. */
	this->quadtree.set(res, center, radius);

	/* iterate over the boundary vertices of this region */
	for(cit = reginfo.begin(); cit != reginfo.end(); cit++)
	{
		/* this corner should be a boundary vertex */
		vit = vert3d_ind.find(*cit);
		if(vit == vert3d_ind.end())
		{
			/* this map should have all vertices defined,
			 * something went wrong here */
			return -1;
		}

		/* get the position of this corner in 3D space */
		cit->get_position(octree, p3d);

		/* compute projected position onto 2D space */
		p2d = this->M * p3d;

		/* get corner discretization in 2D */
		corner2.set(center, res, p2d); 

		/* store in local map */
		this->vert2d_ind.insert(pair<corner_t, size_t>(
					corner2, vit->second));	
	}

	/* iterate over the faces of this region */
	for(fit = region.begin(); fit != region.end(); fit++)
	{
		/* check if it is oriented correctly */
		if(fit->direction == f || fit->direction == opp_f)
		{
			/* we want to add the geometry of this face
			 * to the geometry of the quadtree. First, we
			 * need to get its projected position and size */
			fit->get_center(p3d);
			hw = fit->get_halfwidth();
			p2d = this->M * p3d;
		
			/* add this area to the quadtree */
			this->quadtree.subdivide(p2d, hw);
		}
	}

	/* iterate again, locking any boundary faces */
	for(fit = region.begin(); fit != region.end(); fit++)
	{
		/* check if it is oriented correctly */
		domdir = (fit->direction == f || fit->direction == opp_f);
		
		/* Check if this face is on the boundary.  If so,
		 * then we want to make sure it doesn't get simplified
		 * later, so we should put some data here. */
		this->lock_if_boundary_face(*fit, domdir, 
				octree, vert3d_ind, err);
	}

	/* now that we've populated the quadtree with all
	 * the appropriate faces of this region, we want to
	 * simplify the quadtree geometry to ensure minimal
	 * triangles are used to represent the planar region. */
	this->quadtree.simplify();

	/* the quadtree now represents the interior area
	 * of the region */
	return 0;
}
		
int region_isostuffer_t::compute_verts(mesh_io::mesh_t& mesh,
				const color_t& color, quadnode_t* q)
{
	map<corner_t, size_t>::const_iterator vit;
	Vector2d pts2d;
	Vector3d pts3d;
	corner_t corner;
	mesh_io::vertex_t vert;
	size_t i, v_ind;
	int ret;

	/* check arguments */
	if(q == NULL)
		q = this->quadtree.get_root();

	/* only triangulate leaf nodes */
	if(!(q->isleaf()))
	{
		/* not a leaf node, so recurse through its children
		 * until we find leaf nodes. */
		for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
			if(q->children[i] != NULL)
			{
				ret = this->compute_verts(mesh, color, 
						q->children[i]);
				if(ret)
					return PROPEGATE_ERROR(-1, ret);
			}
		
		/* success */
		return 0;
	}

	/* if got here, then we're a leaf */

	/* iterate over each corner */
	for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
	{
		/* get position of corners of node */
		pts2d = q->corner_position(i);
		corner.set(this->quadtree.get_root()->center,
			this->quadtree.get_resolution(),
			pts2d);

		/* get info about this corner */
		vit = this->vert2d_ind.find(corner);
		if(vit != this->vert2d_ind.end())
			continue; /* already exists */

		/* This is a new vertex, internal only
		 * to this region. */
		v_ind = this->add_vertex(pts2d, color, mesh);

		/* add it to the vert map */
		this->vert2d_ind.insert(
			pair<corner_t, size_t>(
				corner, v_ind));
	}

	/* success */
	return 0;
}
		
int region_isostuffer_t::triangulate(mesh_io::mesh_t& mesh, 
				const color_t& color, quadnode_t* q)
{
	vector<quadnode_t*> neighs;
	size_t v_inds[quadnode_t::CHILDREN_PER_QUADNODE];
	map<corner_t, size_t>::const_iterator vit, edge_a_vit, edge_b_vit;
	Vector2d pts2d, edge_a, edge_b;
	Vector3d pts3d;
	corner_t corner, edge_a_corner, edge_b_corner;
	mesh_io::vertex_t vert;
	mesh_io::polygon_t poly;
	size_t i, num_neighs, center_ind, edge_a_ind, edge_b_ind;
	bool min_feature;
	double res, err;
	int ret;

	/* check arguments */
	if(q == NULL)
		q = this->quadtree.get_root();

	/* only triangulate leaf nodes */
	if(!(q->isleaf()))
	{
		/* not a leaf node, so recurse through its children
		 * until we find leaf nodes. */
		for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
			if(q->children[i] != NULL)
			{
				ret = this->triangulate(mesh, color, 
						q->children[i]);
				if(ret)
					return PROPEGATE_ERROR(-1, ret);
			}
		
		/* success */
		return 0;
	}

	/* if got here, then we're a leaf */

	/* iterate over each corner */
	for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
	{
		/* get position of corners of node */
		pts2d = q->corner_position(i);
		corner.set(this->quadtree.get_root()->center,
			this->quadtree.get_resolution(),
			pts2d);

		/* get info about this corner */
		vit = this->vert2d_ind.find(corner);
		if(vit == this->vert2d_ind.end())
		{
			/* can't find node */
			cerr << "[region_isostuffer_t::triangulate]\t"
			     << "Error!  Can't find vertex index for "
			     << "corner:\t";
			corner.writecsv(cerr);
			cerr << endl << endl;
			return -1;
		}
			
		/* Since the vertex already exists,
		 * its index is taken
		 * from the map */
		v_inds[i] = vit->second;
	}

	/* get the neighboring nodes to this leaf */
	res = this->quadtree.get_resolution();
	err = res/4; /* no feature in tree should be this small */
	q->get_neighbors_under(neighs, 
			this->quadtree.get_root(), err);

	/* triangulate this leaf based on its neighbors.
	 *
	 * There are two ways to triangulate a node.  If the
	 * node is smaller than all of its neighbors, we can
	 * just put a square there (only two triangles). */
	num_neighs = neighs.size();
	min_feature = true;
	for(i = 0; i < num_neighs; i++)
		if(neighs[i]->halfwidth < q->halfwidth)
		{
			/* found a neighboring node smaller than q,
			 * so we cannot triangulate q with the
			 * simple method...bummer. */
			min_feature = false;
			break;
		}		

	/* check if we can use simple triangulation */
	if(min_feature)
	{
		/* just put a square (via two triangles) */
		poly.set(v_inds[0], v_inds[1], v_inds[2]);
		if(!(poly.is_degenerate()))
			mesh.add(poly);
		poly.set(v_inds[0], v_inds[2], v_inds[3]);
		if(!(poly.is_degenerate()))
			mesh.add(poly);

		/* success */
		return 0;
	}

	/* If got here, then we need to use more complicated mesh
	 * for this node.
	 *
	 * Since node is square, we can put a vertex at its center,
	 * and add edges to all neighboring sides, ensuring watertightness.
	 */

	/* put a vertex at the center */
	center_ind = this->add_vertex(q->center, color, mesh);

	/* add triangles from center to all neighboring nodes */
	for(i = 0; i < num_neighs; i++)
	{
		/* get corners of neighbor that touch this node */

		/* get position of corners */
		ret = q->edge_in_common(edge_a, edge_b, neighs[i], err);
		if(ret)
		{
			cerr << endl
			     << "[region_isostuffer_t::triangulate]\t"
			     << "Error " << ret << ": Unable to find "
			     << "edge in common between nodes:" << endl
			     << "\t\tq->center: " << q->center.transpose()
			     << endl
			     << "\t\tq->halfwidth: " << q->halfwidth
			     << endl
			     << "\t\tneigh[" << i << "]->center: " 
			     << neighs[i]->center.transpose() << endl
			     << "\t\tneigh[" << i << "]->halfwidth: "
			     << neighs[i]->halfwidth << endl << endl;
			continue;
		}

		/* convert to corner objects */
		edge_a_corner.set(this->quadtree.get_root()->center,
			res, edge_a);
		edge_b_corner.set(this->quadtree.get_root()->center,
			res, edge_b);

		/* look up vertices of these corners */
		edge_a_vit = this->vert2d_ind.find(edge_a_corner);
		edge_b_vit = this->vert2d_ind.find(edge_b_corner);
		if(edge_a_vit == this->vert2d_ind.end()
				|| edge_b_vit == this->vert2d_ind.end())
		{
			/* report the error */
			cerr << endl
			     << "[region_isostuffer_t::triangulate]\t"
			     << "Error occurred when triangulating node:\n"
			     << "\t\tmy center: " << q->center.transpose()
			     << endl
			     << "\t\tmy halfwidth: " << q->halfwidth
			     << endl
			     << "\t\tneigh center: " 
			     << neighs[i]->center.transpose() << endl
			     << "\t\tneigh halfwidth: " 
			     << neighs[i]->halfwidth << endl
			     << "\t\tedge_a: " << edge_a.transpose()
			     << endl
			     << "\t\t\tfound: " 
			     << (edge_a_vit != this->vert2d_ind.end())
			     << endl
			     << "\t\tedge_b: " << edge_b.transpose()
			     << endl
			     << "\t\t\tfound: " 
			     << (edge_b_vit != this->vert2d_ind.end())
			     << endl << endl;
			continue; /* couldn't find them! */
		}
		edge_a_ind = edge_a_vit->second;
		edge_b_ind = edge_b_vit->second;

		/* create triangle between center and this neighbor */
		poly.set(center_ind,edge_a_ind,edge_b_ind);
		if(!(poly.is_degenerate()))
			mesh.add(poly);
	}

	/* success */
	return 0;
}
		
void region_isostuffer_t::writeobj(std::ostream& os, quadnode_t* q) const
{
	map<corner_t, size_t>::const_iterator vit;
	Vector2d pts2d;
	Vector3d pts3d;
	corner_t corner;
	color_t c;
	size_t i;

	/* check arguments */
	if(q == NULL)
	{
		/* write some comments at the top */
		os << endl
		   << "# M = " 
		   << this->M(0,0) << " " 
		   << this->M(0,1) << " " 
		   << this->M(0,2) << endl
		   << "#     "
		   << this->M(1,0) << " " 
		   << this->M(1,1) << " " 
		   << this->M(1,2) << endl
		   << endl;

		/* write the boundary verts */
		os << "# The boundary vertex corner indices:" << endl;
		for(vit = this->vert2d_ind.begin(); 
				vit != this->vert2d_ind.end();
					vit++)
		{
			os << "# ";
			vit->first.writecsv(os);
			os << endl;
		}
		os << endl;

		/* assign root node */
		q = this->quadtree.get_root();
	}

	/* only write out leaf nodes */
	if(q->isleaf())
	{
		/* iterate over each corner */
		for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
		{
			/* get position of corners of node */
			pts2d = q->corner_position(i);

			/* check if this is a fixed vertex */
			corner.set(this->quadtree.get_root()->center,
				this->quadtree.get_resolution(),
				pts2d);
			if(this->vert2d_ind.count(corner) > 0)
				c.set(0.0f, 1.0f, 0.0f);
			else
				c.set(1.0f, 0.0f, 0.0f);
		
			/* project quadnode corners back to 3D */
			pts3d = M.transpose() * pts2d;
			this->plane.get_intersection_of(pts3d, pts3d,
						this->nullspace);

			/* write out to file stream */
			os << "v " << pts3d.transpose()
			   <<  " " << c.get_red_int()
			   <<  " " << c.get_green_int()
			   <<  " " << c.get_blue_int()
			   << endl;
		}
	   	os << "f -4 -3 -2 -1" << endl << endl;
	}

	/* recurse */
	for(i = 0; i < quadnode_t::CHILDREN_PER_QUADNODE; i++)
		if(q->children[i] != NULL)
			this->writeobj(os, q->children[i]);
}

/*---------------------------------*/
/* helper function implementations */
/*---------------------------------*/

void region_isostuffer_t::mapping_matrix(octtopo::CUBE_FACE f)
{
	/* store the appropriate matrix */
	switch(f)
	{
		case octtopo::FACE_ZPLUS:
			this->M << 1,0,0,  0,1,0; /* x->x, y->y */
			break;

		case octtopo::FACE_ZMINUS:
			this->M << 0,1,0,  1,0,0; /* x->y, y->x */
			break;

		case octtopo::FACE_YPLUS:
			this->M << 0,0,1,  1,0,0; /* x->y, z->x */
			break;
		
		case octtopo::FACE_YMINUS:
			this->M << 1,0,0,  0,0,1; /* x->x, z->y */
			break;

		case octtopo::FACE_XPLUS:
			this->M << 0,1,0,  0,0,1; /* y->x, z->y */
			break;

		case octtopo::FACE_XMINUS:
			this->M << 0,0,1,  0,1,0; /* y->y, z->x */
			break;
	}

	/* record the normal of this face */
	octtopo::cube_face_normals(f, this->nullspace);
}
		
void region_isostuffer_t::lock_if_boundary_face(const node_face_t& f,
				bool domdir,
				const octree_t& tree,
				const std::map<node_corner::corner_t,
						size_t>& vert3d_ind,
				double err)
{
	bool is_boundary[NUM_CORNERS_PER_SQUARE];
	Vector2d p2d[NUM_CORNERS_PER_SQUARE];
	corner_t c;
	Vector3d p3d;
	Vector2d center, p, dp, norm;
	size_t ci, ci_next, si, num_samples;
	bool any_boundary;
	quadnode_t* q;

	/* iterate over the corners of this face */
	any_boundary = false;
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* get the current corner info */
		c.set(tree, f, ci);

		/* check if this corner is a boundary */
		if(vert3d_ind.count(c) == 0)
		{
			is_boundary[ci] = false;
			continue; /* not a boundary */
		}
	
		/* since this is a boundary corner, save its
		 * 2d position on the quadtree */
		c.get_position(tree, p3d);
		p2d[ci] = this->M * p3d;
		is_boundary[ci] = true;
		any_boundary = true;
	}

	/* short-circuit if no boundaries */
	if(!any_boundary)
		return;

	/* get the 2D position of the center of this face */
	f.get_center(p3d);
	center = this->M * p3d;
	if(!domdir)
	{
		/* this face is not aligned to the dominant
		 * direction of the region, which means that
		 * all the points we process here will be right
		 * on the boundary of the region.  We want to
		 * push this geometry into the interior.
		 *
		 * In other to do that, we need to test which
		 * direction is best. */
		
		/* get the normal direction of face */
		f.get_normal(p3d);
		dp = this->M * p3d;
		dp *= err;

		/* try positive normal */
		p = center + dp;
		q = this->quadtree.get_root()->retrieve(p);
		if(q != NULL && q->isleaf())
		{
			/* this is a good direction */
			center = p;
		}
		else
		{
			/* try negative normal */
			p = center - dp;
			q = this->quadtree.get_root()->retrieve(p);
			if(q != NULL && q->isleaf())
			{
				/* this is a good direction */
				center = p;
			}
			else
			{
				/* neither direction is good, it's
				 * a lost cause */
				return;
			}
		}
	}

	/* determine how many samples to put on each boundary edge.
	 *
	 * Each sample is spaced 'err' apart, and there's a spacing
	 * of 'err' on each side. */
	num_samples = ceil( (2.0*f.get_halfwidth()/err) - 1);

	/* check which edges are boundary edges (i.e. both
	 * corners of the edge are boundary corners) */
	for(ci = 0; ci < NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* check if this corner is a boundary */
		if(!is_boundary[ci])
			continue;

		/* get the starting corner a little inset into the face */
		norm = center - p2d[ci];
		norm.normalize();
		norm *= sqrt(2) * err; /* amount to move starting point */

		/* put starting point towards the center a bit */
		p = p2d[ci] + norm;
		this->quadtree.insert(p, p);
		
		/* now we want to do this same operation for the
		 * whole edge, assuming that the next corner is
		 * also on the boundary.
		 *
		 * get the next corner of this edge */
		ci_next = (ci+1) % NUM_CORNERS_PER_SQUARE;

		/* check if both corners are boundary */
		if(!is_boundary[ci_next])
			continue; /* not boundary edge */

		/* get direction of edge */
		dp = p2d[ci_next] - p2d[ci];
		dp.normalize();
		dp *= err;

		/* since this is a boundary edge, insert
		 * a bunch of points along it to make sure
		 * that it won't get simplified later 
		 *
		 * Note the loop starts at 1 because we've
		 * already placed the first point above. */
		p += dp;
		for(si = 1; si < num_samples; si++)
		{
			/* insert the locking point */
			this->quadtree.get_root()->insert(p, p, 1, 
					this->quadtree.get_max_depth());

			/* update point */
			p += dp;
		}
	}
}
		
size_t region_isostuffer_t::add_vertex(const Eigen::Vector2d& p2d,
				const color_t& color,
				mesh_io::mesh_t& mesh) const
{
	mesh_io::vertex_t vert;
	Vector3d p3d;
	size_t v_ind;

	/* get the 3d coordinates of point */
	p3d = this->M.transpose() * p2d;
	this->plane.get_intersection_of(p3d, p3d, this->nullspace);
	
	/* populate vertex */
	vert.x     = p3d(0);
	vert.y     = p3d(1);
	vert.z     = p3d(2);
	vert.red   = color.get_red_int();
	vert.green = color.get_green_int();
	vert.blue  = color.get_blue_int();

	/* add to mesh */
	v_ind      = mesh.num_verts();
	mesh.add(vert);

	/* return its index */
	return v_ind;
}
