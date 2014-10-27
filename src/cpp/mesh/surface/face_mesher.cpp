#include "face_mesher.h"
#include <io/mesh/mesh_io.h>
#include <geometry/octree/octree.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner.h>
#include <mesh/surface/node_corner_map.h>
#include <util/error_codes.h>
#include <map>

/**
 * @file    face_mesher.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Converts octree geometry to a dense mesh
 *
 * @section DESCRIPTION
 *
 * This file contains the face_mesher_t class, which is used
 * to convert from node_boundary_t (which stores the boundary
 * faces of octree geometry) to a mesh that can be exported.
 *
 * The meshing technique used is to represent each face as a
 * vertex in the output, and each corner of the octree as a
 * polygon.  In this sense, it is a dual method (much like
 * dual contouring).
 */

using namespace std;
using namespace Eigen;
using namespace node_corner;

/* the following constants are used for computations in this file */

#define APPROX_ZERO  0.00001

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int face_mesher_t::add(const octree_t& tree, 
                       const node_boundary_t& boundary)
{
	corner_map_t corners;
	int ret;

	/* construct corners from boundary information */
	corners.add(tree, boundary);

	/* add all these corners to this structure */
	ret = this->add(tree, corners);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int face_mesher_t::add(const octree_t& tree,
		       const corner_map_t& corners)
{
	ccmap_t::const_iterator it;
	faceset_t::const_iterator fit;
	vector<pair<double, size_t> > face_inds; /* <sort value, index> */
	Vector3d face_pos, norm, avg_norm, a, b, pos, disp, com;
	mesh_io::polygon_t poly;
	double normmag, total_weight, area;
	size_t i, num_faces;

	/* iterate over the corners given */
	for(it = corners.begin(); it != corners.end(); it++)
	{
		/* reset values for this corner */
		avg_norm << 0,0,0; /* reset to zero */
		com << 0,0,0; /* reset center-of-mass to zero */
		face_inds.clear();
		poly.clear();
		it->first.get_position(tree, pos);
		total_weight = 0;

		/* add all faces to this structure */
		for(fit = it->second.begin_faces(); 
				fit != it->second.end_faces(); fit++)
		{
			/* compute position of this face's center */
			fit->get_isosurface_pos(face_pos);

			/* add this face */
			face_inds.push_back(pair<double, size_t>
					(0.0, this->add(*fit, face_pos)));
		
			/* compute normal for this face */
			fit->get_normal(norm);

			/* each face counts proportional
			 * to its surface area when performing
			 * the weighted average */
			area = fit->get_area();
			avg_norm += norm * area;
			com += face_pos * area;
			total_weight += area;
		}

		/* compute average normal for all faces */
		normmag = avg_norm.norm();
		if(normmag < APPROX_ZERO)
			avg_norm << 0,0,1; /* arbitrary */
		else
			avg_norm /= normmag; /* normalize */
		com /= total_weight; /* compute center-of-mass */

		/* check that we have sufficient faces to make a polygon */
		num_faces = face_inds.size();
		if(num_faces < 3)
		{
			cerr << "[face_mesher_t::add]\tFound corner that "
			     << "is connected to fewer than three faces!"
			     << endl 
			     << "\tCorner pos: " << pos.transpose() << endl
			     << "\tavg_norm: "     << avg_norm << endl
			     << "\tNum faces: "    << num_faces << endl
			     << "\ttotal weight: " << total_weight << endl
			     << endl;
			continue;
		}

		/* the face normals all point outwards (from interior
		 * to exterior), but we want our polygon's normal
		 * to point inwards (from exterior to interior) */
		avg_norm *= -1;

		/* make up some coordinate axis for the tangent
		 * plane of this corner (remember that the corner
		 * will become a polygon) */
		if(abs(avg_norm(0)) < abs(avg_norm(1)))
			a << 1,0,0; /* x-axis less in line with norm */
		else
			a << 0,1,0; /* y-axis less in line with norm */
		b = avg_norm.cross(a);
		b.normalize();
		a = b.cross(avg_norm);

		/* sort the faces around the corner by their angle
		 * along these coordinate axes */
		for(i = 0; i < num_faces; i++)
		{
			/* get displacement of vertex from corner pos */
			disp(0) = this->mesh.get_vert(
					face_inds[i].second).x;
			disp(1) = this->mesh.get_vert(
					face_inds[i].second).y;
			disp(2) = this->mesh.get_vert(
					face_inds[i].second).z;
			disp -= com; /* displacement relative to 
					center-of-mass */

			/* compute angle of this vertex with respect
			 * to the coordinates (a,b) */
			face_inds[i].first = atan2(disp.dot(b),disp.dot(a));
		}
		std::sort(face_inds.begin(), face_inds.end());

		/* add a polygon about this corner from all the vertices
		 * from these faces
		 *
		 * We want to export triangles, rather than n-sided 
		 * polygons, so split the current poly into triangles */
		for(i = 1; i < num_faces-1; i++)
		{
			poly.clear();
			poly.vertices.push_back(face_inds[0].second);
			poly.vertices.push_back(face_inds[i].second);
			poly.vertices.push_back(face_inds[i+1].second);
			this->mesh.add(poly);
		}
	}

	/* success */
	return 0;
}
		
size_t face_mesher_t::add(const node_face_t& face)
{
	Vector3d pos;

	/* compute isosurface position for this face */
	face.get_isosurface_pos(pos);

	/* add to mesh */
	return this->add(face, pos);
}

size_t face_mesher_t::add(const node_face_t& face,
		           const Eigen::Vector3d& pos)
{
	pair<map<node_face_t, size_t>::iterator, bool> ins;
	mesh_io::vertex_t vert;
	size_t ind;

	/* for now, assume we'll be adding this vertex at the end */
	ind = this->mesh.num_verts(); /* index of not-yet-added vertex */

	/* check if face is already in map */
	ins = this->face_index_map.insert(pair<node_face_t, size_t>(face, 
								ind));
	if(!(ins.second))
		/* already present, just return existing index */
		return ins.first->second; 

	/* we need to create a new vertex for this face, and we
	 * have a few options as to how to do that.
	 *
	 * For now, compute the isosurface position at the
	 * center of the face. */
	vert.x = pos(0);
	vert.y = pos(1);
	vert.z = pos(2);

	/* insert the vertex into the mesh */
	this->mesh.add(vert);
	return ind;
}

/*------------------*/
/* helper functions */
/*------------------*/

void face_mesher_t::get_face_pos(Eigen::Vector3d& pos,
				const octree_t& tree, 
				const node_face_t& face, 
				const node_corner::corner_map_t& corner_map)
{
	node_corner::corner_t corner[node_corner::NUM_CORNERS_PER_SQUARE];
	double cval[node_corner::NUM_CORNERS_PER_SQUARE];
	Vector3d cpos[node_corner::NUM_CORNERS_PER_SQUARE];
	Vector3d u, v, cnet, fpos;
	size_t ci;
	double a, b, c, d, uval, vval;

	/* compute face position */
	face.get_isosurface_pos(fpos);

	/* iterate over the corners of this face */
	for(ci = 0; ci < node_corner::NUM_CORNERS_PER_SQUARE; ci++)
	{
		/* get this corner */
		corner[ci].set(tree, face, ci);
		corner[ci].get_position(tree, cpos[ci]);

		/* compute probability value at the corner */
		cval[ci] = face_mesher_t::get_corner_prob(corner[ci], tree,
						corner_map);
	}

	/*
	 * The following graphic demonstrates the location of the 
	 * corners of this face.
	 *
	 * V    [1] .-----<a--.  [0]
	 * ^        |         ^  
	 * |        b         d  
	 * |        v         |  
	 * |        |         |  
	 * |    [2] .---c>----.  [3]
	 * |
	 * .-----------------------------> U
	 *
	 *
	 * The values a,b,c,d indicate the positions along each edge
	 * of the 0.5 value when performing linear interpolation between
	 * probabilities at the corners.
	 *
	 * So, if a=0, that means the 0.5 mark occurs at [0], and if a=0.9,
	 * then the 0.5 mark occurs close to [1].
	 */
	a = b = c = d = 0.5;
	if(abs(cval[0] - cval[1]) > APPROX_ZERO)
		a = (cval[0] - 0.5) / (cval[0] - cval[1]);
	if(abs(cval[1] - cval[2]) > APPROX_ZERO)
		b = (cval[1] - 0.5) / (cval[1] - cval[2]);
	if(abs(cval[2] - cval[3]) > APPROX_ZERO)
		c = (cval[2] - 0.5) / (cval[2] - cval[3]);
	if(abs(cval[3] - cval[0]) > APPROX_ZERO)
		d = (cval[3] - 0.5) / (cval[3] - cval[0]);

	/* since it may be the case that both values on an edge are
	 * greater than 0.5 or both are less than 0.5, then we should
	 * make sure to restrict the output to the range [0,1] */
	a = min(1.0, max(0.0, a));
	b = min(1.0, max(0.0, b));
	c = min(1.0, max(0.0, c));
	d = min(1.0, max(0.0, d));
	
	/* now we want to find the bilinearly interpolated value
	 * in the surface of the face */
	u = cpos[0] - cpos[1]; /* the axis shown as horizontal above */
	v = cpos[1] - cpos[2]; /* the axis shown as vertical above */

	/* now we want to get the displacement of the center
	 * position from the center of the face, in both the u
	 * and v directions.
	 *
	 * This means that we want to average the horizontal
	 * edge values (a,c) to get uval and also average the
	 * vertical edge values (b,d) to get vval.  Since each
	 * value in each direction goes opposing ways, this average
	 * will be a simple subtraction.
	 *
	 * Also, since we're displacing from the center of the face
	 * to the edge of the face, there will be an extra division
	 * by 2 to account for the lengths of u and v. */
	uval = (c - a) / 2;
	vval = (d - b) / 2;

	/* compute the 2D lateral displacement from face center */
	cnet = u*uval + v*vval;

	/* incorporate the best corner position (lateral direction) with
	 * the face's isosurface position (normal direction) */
	pos = fpos + cnet;
}

double face_mesher_t::get_corner_prob(
				const node_corner::corner_t& corner,
				const octree_t& tree,
				const node_corner::corner_map_t& corner_map)
{
	pair<faceset_t::const_iterator, faceset_t::const_iterator> range;
	faceset_t::const_iterator it;
	set<octnode_t*> nodes;
	set<octnode_t*>::iterator sit;
	double num, den, dist;
	Vector3d cpos;

	/* get position of this corner */
	corner.get_position(tree, cpos);

	/* get all the faces associated with this corner */
	range = corner_map.get_faces_for(corner);
	for(it = range.first; it != range.second; it++)
	{
		/* add nodes from this face to the map */
		nodes.insert(it->interior);
		nodes.insert(it->exterior);
	}

	/* iterate over the nodes, generating weighted average */
	num = den = 0;
	for(sit = nodes.begin(); sit != nodes.end(); sit++)
	{
		/* check value */
		if(*sit == NULL || (*sit)->data == NULL)
			continue; /* ignore this one */

		/* get inverse distance of corner to node */
		dist = 1.0 / (cpos - (*sit)->center).norm();

		/* add the probability value of the current node,
		 * inversely weighted by its distance */
		num += dist * (*sit)->data->get_probability();
		den += dist;
	}

	/* check edge case of no nodes */
	if(den == 0)
		return octdata_t::UNOBSERVED_PROBABILITY;

	/* return the weighted average */
	return (num / den);
}
