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
	Vector3d norm, avg_norm, a, b, pos, disp;
	mesh_io::polygon_t poly;
	double normmag;
	size_t i, num_faces;

	/* iterate over the corners given */
	for(it = corners.begin(); it != corners.end(); it++)
	{
		/* reset values for this corner */
		avg_norm << 0,0,0; /* reset to zero */
		face_inds.clear();
		poly.clear();
		it->first.get_position(tree, pos);

		/* add all faces to this structure */
		for(fit = it->second.begin_faces(); 
				fit != it->second.end_faces(); fit++)
		{
			/* add this face */
			face_inds.push_back(pair<double, size_t>
					(0.0, this->add(*fit)));
		
			/* compute normal for this face */
			fit->get_normal(norm);
			avg_norm += norm; /* each face counts equally */
		}

		/* compute average normal for all faces */
		num_faces = face_inds.size();
		normmag = avg_norm.norm();
		if(normmag == 0)
			avg_norm << 0,0,-1; /* arbitrary */
		else
			avg_norm /= normmag; /* normalize */

		/* check that we have sufficient faces to make a polygon */
		if(num_faces < 3)
			return -1;

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
			disp -= pos;

			/* compute angle of this vertex with respect
			 * to the coordinates (a,b) */
			face_inds[i].first = atan2(disp.dot(b),disp.dot(a));
		}
		std::sort(face_inds.begin(), face_inds.end());

		/* add a polygon about this corner from all the vertices
		 * from these faces */
		for(i = 0; i < num_faces; i++)
			poly.vertices.push_back(face_inds[i].second);
		this->mesh.add(poly);
	}

	/* success */
	return 0;
}
		
size_t face_mesher_t::add(const node_face_t& face)
{
	pair<map<node_face_t, size_t>::iterator, bool> ins;
	Vector3d pos;
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
	face.get_isosurface_pos(pos);
	vert.x = pos(0);
	vert.y = pos(1);
	vert.z = pos(2);

	/* insert the vertex into the mesh */
	this->mesh.add(vert);
	return ind;
}
