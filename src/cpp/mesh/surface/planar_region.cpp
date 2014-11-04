#include "planar_region.h"
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner.h>
#include <geometry/shapes/plane.h>
#include <geometry/octree/octtopo.h>
#include <iostream>
#include <queue>
#include <set>
#include <stdlib.h>
#include <Eigen/Dense>

/**
 * @file   planar_region.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Defines planar region class, made up of a set of node_face_t's
 * 
 * @section DESCRIPTION
 *
 * This file implements the planar_region_t class, which is used to cluster
 * node_face_t objects into large, planar regions.  This class gives
 * a representation of such regions, as well as a way to generate them
 * from a set of nodes.
 *
 * Note that this class operates on node_boundary_t objects, which should
 * already be populated from an octtopo_t topology.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void planar_region_t::floodfill(const node_face_t& seed,
				const node_boundary_t& boundary,
				faceset_t& blacklist)
{
	/* give the most lax possible threshold, to allow all faces */
	this->floodfill(seed, boundary, blacklist, 0.0);
}

void planar_region_t::floodfill(const node_face_t& seed,
				const node_boundary_t& boundary,
				faceset_t& blacklist, double planethresh)
{
	queue<node_face_t> to_check;
	pair<faceset_t::const_iterator, faceset_t::const_iterator> range;
	faceset_t::const_iterator it;

	/* clear any existing information for this region */
	this->clear();

	/* prepare plane geometry based on seed face */
	seed.get_center(this->plane.point);
	octtopo::cube_face_normals(seed.direction, this->plane.normal);

	/* check if seed face does not meet threshold.  If so, then
	 * it should be in a region by itself */
	if(seed.get_planarity() < planethresh)
	{
		/* seed gets added in a region by itself */
		this->add(seed);
		blacklist.insert(seed);
		return; /* no neighbors for you */
	}

	/* check everything in the queue until we run out */
	for(to_check.push(seed); !(to_check.empty()); to_check.pop())
	{
		/* check if next value is on blacklist */
		if(blacklist.count(to_check.front()))
			continue; /* can't use this one */

		/* check that face is direction in same orientation */
		if(to_check.front().direction != seed.direction)
			continue;

		/* check that face meets planarity threshold */
		if(to_check.front().get_planarity() < planethresh)
			continue;

		/* add to our region and to the blacklist */
		this->add(to_check.front());
		blacklist.insert(to_check.front());

		/* add neighbors to list to check */
		range = boundary.get_neighbors(to_check.front());
		for(it = range.first; it != range.second; it++)
			to_check.push(*it);
	}

	/* make sure the normal direction is consistent */
	this->orient_normal();
}
		
void planar_region_t::find_face_centers(vector<Vector3d,
			Eigen::aligned_allocator<Vector3d> >& centers,
			vector<double>& variances, bool useiso) const
{
	faceset_t::const_iterator it;
	double hw;
	size_t i, n;

	/* resize the vectors appropriately */
	i = max(centers.size(), variances.size());
	n = this->faces.size();
	centers.resize(i+n);
	variances.resize(i+n);

	/* iterate through the faces in this region */
	for(it = this->faces.begin(); it != this->faces.end(); it++, i++)
	{
		if(useiso)
		{
			/* compute the center */
			it->get_isosurface_pos(centers[i]);

			/* compute the variances */
			variances[i] = it->get_pos_variance();
		}
		else
		{
			/* use the grid positions and halfwidth */
			it->get_center(centers[i]);
			
			/* use halfwidth as std. dev. */
			hw = it->get_halfwidth();
			variances[i] = hw*hw;
		}
	}
}
		
double planar_region_t::surface_area() const
{
	faceset_t::const_iterator it;
	double area;

	/* iterate over the faces */
	area = 0;
	for(it = this->faces.begin(); it != this->faces.end(); it++)
		area += it->get_area();

	/* return the sum */
	return area;
}
		
void planar_region_t::compute_bounding_box(
				const Vector3d& a, const Vector3d& b,
				double& a_min, double& a_max,
				double& b_min, double& b_max) const
{
	faceset_t::const_iterator it;
	Vector3d p, q;
	double hw, coord_a, coord_b;

	/* reset bounds to something invalid */
	a_min = 1; a_max = 0; /* if a_min > a_max, then invalid */
	b_min = 1; b_max = 0;

	/* iterate over the faces */
	for(it = this->faces.begin(); it != this->faces.end(); it++)
	{
		/* get points for this face on the plane */
		hw = it->get_halfwidth(); /* get size of this face */
		it->get_center(p); /* get the center of this face */
		
		/* get this point in the a,b coordinate frame */
		p -= this->plane.point; /* all operations relative
		                         * to position of plane */
		coord_a = p.dot(a);
		coord_b = p.dot(b);

		/* update the bounds for this face, taking size
		 * of face into account */
		a_min = min(a_min, coord_a - hw);
		a_max = max(a_max, coord_a + hw);
		b_min = min(b_min, coord_b - hw);
		b_max = max(b_max, coord_b + hw);
	}
}
		
void planar_region_t::orient_normal()
{
	faceset_t::iterator it;
	Vector3d n;
	double total;

	/* prepare values.  We will iterate over
	 * the faces of the region, and get each
	 * face's vote on the normal direction, which
	 * will be tallied in 'total' 
	 *
	 * A positive value indicates the normal
	 * is currently pointing out of the environment.
	 * A negative value indicates it is currently
	 * inwards. */
	total = 0;
	
	/* iterate over the faces of this region */
	for(it = this->begin(); it != this->end(); it++)
	{
		/* get normal for this face */
		it->get_normal(n);

		/* add to total, weighted by
		 * the surface area of the face */
		total += n.dot(this->plane.normal) * it->get_area();
	}

	/* check if we want to flip normal */
	if(total > 0)
	{
		/* flip it */
		this->plane.normal *= -1;
	}
}

void planar_region_t::writeobj(ostream& os, bool project) const
{
	faceset_t::iterator it;
	int r, g, b;
	size_t corner_ind, cii;
	bool use_interior;
	octtopo::CUBE_FACE f;
	octnode_t* node;
	Vector3d corner_pos;

	/* generate a random color */
	r = 64 + (rand() % 128);
	g = 64 + (rand() % 128);
	b = 64 + (rand() % 128);

	/* write the faces with this color */
	for(it = this->faces.begin(); it != this->faces.end(); it++)
	{
		/* check if we should write the face as the original
		 * octnode face, or projected onto the plane geometry */
		if(!project)
		{
			it->writeobj(os, r, g, b);
			continue;
		}

		/* check whether the face's interior node or
		 * its exterior node is smaller */
		use_interior = (it->exterior == NULL 
				|| (it->exterior->halfwidth 
				>= it->interior->halfwidth));
		f = (use_interior ? it->direction 
			: octtopo::get_opposing_face(it->direction));
		node = (use_interior ? it->interior : it->exterior);

		/* we want to project the face onto the plane geometry,
		 * so we need to write out the face from scratch 
		 *
		 * Start by iterating over the face corners, getting
		 * their 3D position */
		for(cii = 0; cii < node_corner::NUM_CORNERS_PER_SQUARE; 
							cii++)
		{
			/* get position of this corner on cube */
			corner_ind = node_corner::get_face_corner(f, cii);
			corner_pos = node_corner::get_corner_pos(node,
							corner_ind);
			
			/* project the corner onto the plane */
			this->plane.project_onto(corner_pos);

			/* write to file */
			os << "v " << corner_pos(0)
			   <<  " " << corner_pos(1)
			   <<  " " << corner_pos(2)
			   <<  " " << r
			   <<  " " << g
			   <<  " " << b << endl;
		}

		/* write surface using the above vertices 
		 *
		 * The vertices are ordered to be counter-clockwise
		 * via the interior node's face.  But we want to look
		 * from the interior to the exterior, which flips its
		 * direction. */
		if(use_interior)
			os << "f -1 -2 -3 -4" << endl;
		else
			os << "f -4 -3 -2 -1" << endl;
	}
}
