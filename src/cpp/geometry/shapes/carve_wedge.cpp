#include "carve_wedge.h"
#include <geometry/carve/gaussian/carve_map.h>
#include <geometry/octree/octdata.h>
#include <geometry/poly_intersect/pcube.h>
#include <geometry/poly_intersect/get_polygon_normal.h>
#include <stdlib.h>
#include <iostream>
#include <Eigen/Dense>

/**
 * @file carve_wedge.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The carve_wedge_t class is used to represent a volume in 3D space,
 * and correlate the continuous mapping functions from a carve_map_t
 * to this volume, which allows a carve_map_t to be expressed over
 * a finite volume where it has the most impact.
 *
 * The wedge is formed from four carve_map_t objects, which all
 * contribute to the mapping inside the volume of the wedge.
 *
 * This wedge is also used as a shape that can intersect with octrees,
 * allowing for efficient insertion of carve maps into octrees.
 *
 * This file requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

/* the following defines are used in this code */

/* each wedge is represented by a set of surface triangles, which
 * are used to test intersections with the cubes that represent the
 * nodes of an octree.  A wedge is oriented as the following:
 *
 *            scanpoint #i+1, maps[3]                             
 *                                                                 
 *              5         scanpoint #i+1                           
 *              /--_    2       maps[1]                               
 *             /  - --_.                                              
 *            /    -  / -                                              
 *           /      -/   -                                             
 * next     /       /-    -                                            
 * sensor  /       /  -    -  scanpoint #i                             
 * pos    /       /    - 4  -       maps[2]                            
 *     3 .------ / ------.   -                                          
 *        -     /          .  -                                        
 *         -   /             . -                                       
 *          - /                .-                                      
 *            .--------------------                                    
 *           0                      1                             
 *         current                     scanpoint #i               
 *         sensor                      maps[0]                    
 *         position                                               
 *                                                                
 */

#define NUM_TRIANGLES_PER_WEDGE 10 /* number of triangles to test */
#define NUM_EDGES_PER_WEDGE 15 /* number of edges to test */

#define NUM_VERTS_PER_TRI 3 /* basic geometry */

/* the following define the triangle indices that
 * represent the polygons in this wedge to test for
 * intersections. These indices are into the columns
 * of this->verts, and index starting at 0. */
static const unsigned int tri_inds[NUM_TRIANGLES_PER_WEDGE][3] = {
				{0, 1, 2}, /* 0: intra-frame current */
				{3, 5, 4}, /* 1: intra-frame next */
				{0, 3, 4}, /* 2: intra-scan #i, half */
				{0, 4, 1}, /* 3: intra-scan #i, half */
				{0, 2, 5}, /* 4: intra-scan #i+1, half */
				{0, 5, 3}, /* 5: intra-scan #i+1, half */
				{1, 4, 5}, /* 6: intra-points, half */
				{1, 5, 2}, /* 7: intra-points, half */
				{0, 1, 5}, /* 8: cross-support, half */
				{3, 4, 2}  /* 9: corss-support, half */};

/* the following define all edges of the shape, where each pair
 * represents two vertex indices, indexed from zero. */
static const unsigned int edge_inds[NUM_EDGES_PER_WEDGE][2] = {
				{0, 1}, /* intra current frame */
				{1, 2},
				{2, 0},
				{3, 4}, /* intra next frame */
				{4, 5},
				{5, 3},
				{0, 3}, /* parallel between frames */
				{1, 4},
				{2, 5},
				{0, 4}, /* cross-frame diagonals, #i */
				{3, 1},
				{0, 5}, /* cross-frame diagonals, #i+1 */
				{3, 2},
				{1, 5}, /* cross-diagonals, scanpoints */
				{4, 2}};

/* function implementations */

/*--------------*/
/* constructors */
/*--------------*/

carve_wedge_t::carve_wedge_t()
{
	unsigned int i;

	/* initialize empty maps */
	for(i = 0; i < NUM_MAPS_PER_WEDGE; i++)
		this->maps[i] = NULL;
}

void carve_wedge_t::init(carve_map_t* a1, carve_map_t* a2,
                         carve_map_t* b1, carve_map_t* b2,
                         double nb)
{
	Vector3d u, as, bs, a1p, a2p, b1p, b2p;
	double s;

	/* save these maps to this object */
	this->maps[0] = a1;
	this->maps[1] = a2;
	this->maps[2] = b1;
	this->maps[3] = b2;

	/* define vertex positions for this wedge.  Note that we
	 * want the scanpoint vertex positions to be spread farther 
	 * than the mean positions for each carve map, so that the carving
	 * can apply to the full spread of each distribution. */
	
	/* get mean of points */
	a1->get_sensor_mean(as);
	a1->get_scanpoint_mean(a1p);
	a2->get_scanpoint_mean(a2p);
	b1->get_sensor_mean(bs);
	b1->get_scanpoint_mean(b1p);
	b2->get_scanpoint_mean(b2p);

	/* find position of vertex #0 */
	this->verts[0] = as;
	
	/* find position of vertex #1 */
	u = (a1p - as);
	u.normalize(); /* unit vector in direction away from shape */
	s = a1->get_scanpoint_var(); /* variance in this direction */
	this->verts[1] = a1p + nb*sqrt(s)*u;

	/* find position of vertex #2 */
	u = (a2p - as);
	u.normalize(); /* unit vector in direction away from shape */
	s = a2->get_scanpoint_var(); /* variance in this direction */
	this->verts[2] = a2p + nb*sqrt(s)*u;

	/* find position of vertex #3 */
	this->verts[3] = bs;

	/* find position of vertex #4 */
	u = (b1p - bs);
	u.normalize(); /* unit vector in direction away from shape */
	s = b1->get_scanpoint_var(); /* variance in this direction */
	this->verts[4] = b1p + nb*sqrt(s)*u;

	/* find position of vertex #5 */
	u = (b2p - bs);
	u.normalize(); /* unit vector in direction away from shape */
	s = b2->get_scanpoint_var(); /* variance in this direction */
	this->verts[5] = b2p + nb*sqrt(s)*u;
}
		
/*----------*/
/* geometry */
/*----------*/

bool carve_wedge_t::intersects(const Eigen::Vector3d& c, double hw) const
{
	double vs[NUM_VERTICES_PER_WEDGE][3];
	double curr_tri[NUM_VERTS_PER_TRI][3];
	double curr_norm[3];
	unsigned int i, j, k;
	double s;
	int ret;

	/* prepare array of transformed vertices, which are the
	 * vertices of this wedge in the transform where this box
	 * is the unit box centered at the origin. */
	s = 0.5/hw; /* scale factor */
	for(i = 0; i < NUM_VERTICES_PER_WEDGE; i++)
		for(j = 0; j < 3; j++) /* iterate over dimensions */
			vs[i][j] = (this->verts[i](j) - c(j)) * s;

	/* perform a simple vertex intersection test on the box */
	ret = trivial_vertex_tests(NUM_VERTICES_PER_WEDGE, vs, false);
	switch(ret)
	{
		/* this case indicates that at least one vertex intersects
		 * the volume of the cube, which is a trivial accept
		 * for the intersection */
		case 1: 
			return true;
		
		/* this case indicates that all vertices are off to one
		 * side of the cube, which prevents the possibility of
		 * intersection, trivial reject. */
		case 0:
			return false;

		/* need to search deeper */
		default:
			break;
	}
	
	/* iterate over the edge of the shape, checking
	 * for box-line intersection */
	for(i = 0; i < NUM_EDGES_PER_WEDGE; i++)
		if(segment_intersects_cube(vs[edge_inds[i][0]],
						vs[edge_inds[i][1]]))
			return true; /* edge intersects */
	
	/* no vertices intersected, and no edges
	 * intersected, so now we try the triangulation
	 * of the faces, to check if those polygons
	 * intersect the cube. */
	for(i = 0; i < NUM_TRIANGLES_PER_WEDGE; i++)
	{
		/* construct array for current triangle */
		for(j = 0; j < NUM_VERTS_PER_TRI; j++) /* iter over verts */
			for(k = 0; k < 3; k++) /* iter over dims */
				curr_tri[j][k] = vs[tri_inds[i][j]][k];

		/* compute normal for this triangle */
		get_polygon_normal(curr_norm, NUM_VERTS_PER_TRI, curr_tri);

		/* test if this triangle intersects the cube */
		if(fast_polygon_intersects_cube(NUM_VERTS_PER_TRI,
					curr_tri, curr_norm, true, true))
			return true; /* triangle intersects the cube */
	}

	/* no intersections were found */
	return false;
}
		
octdata_t* carve_wedge_t::apply_to_leaf(const Eigen::Vector3d& c,
                                        double hw, octdata_t* d) const
{
	double val, xsize;
	unsigned int i;

	/* sample the originating carve maps at this location */
	val = 0;
	xsize = 2*hw;
	for(i = 0; i < NUM_MAPS_PER_WEDGE; i++)
		val += this->maps[i]->compute(c, xsize);

	/* use the average of this sample */
	val /= NUM_MAPS_PER_WEDGE;

	/* check if data already exist for this spot */
	if(d == NULL)
	{
		/* create a new data element */
		d = new octdata_t();
	}
		
	/* add to data */
	d->add_sample(val);
	return d;
}
		
/*-----------*/
/* debugging */
/*-----------*/

void carve_wedge_t::writeobj(std::ostream& os) const
{
	unsigned int i;

	/* print comments */
	os << "# The following shape generated from a carve wedge" << endl;

	/* print all vertices */
	for(i = 0; i < NUM_VERTICES_PER_WEDGE; i++)
		os << "v " << this->verts[i](0)
		   <<  " " << this->verts[i](1)
		   <<  " " << this->verts[i](2) << endl;

	/* print all triangles, note that wavefront obj
	 * indexes from 1, and can handle relative indices,
	 * where the most recent vertex is -1, second-most-recent is -2,
	 * etc. */
	for(i = 0; i < NUM_TRIANGLES_PER_WEDGE; i++)
		os << "f -" << (NUM_VERTICES_PER_WEDGE - tri_inds[i][0])
		   <<  " -" << (NUM_VERTICES_PER_WEDGE - tri_inds[i][1])
		   <<  " -" << (NUM_VERTICES_PER_WEDGE - tri_inds[i][2])
		   << endl;
}
