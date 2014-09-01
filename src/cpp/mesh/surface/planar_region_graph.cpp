#include "planar_region_graph.h"
#include <geometry/octree/octdata.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <iostream>
#include <map>
#include <float.h>
#include <Eigen/Dense>

/**
 * @file   planar_region_graph.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Represents the neighbor/connectivity info for regions
 *
 * @section DESCRIPTION
 *
 * Planar regions are used to represent subsets of node faces generated
 * from an octree.  This file contains the planar_region_graph_t, which
 * is used to organize all the regions within a model, and provide
 * connectivity information between regions (i.e. which regions are adjacent
 * to which other regions).
 *
 * This class also is used to generate the set of regions from the model.
 * It is assumed that the topology of the octree has been constructed using
 * an octtopo_t object, and that the boundary faces of that topology have
 * been generated with a node_boundary_t object.
 */

using namespace std;
using namespace Eigen;

/* the following represent the default values for the parameters used
 * in this class */

#define DEFAULT_PLANARITY_THRESHOLD 0.5

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void planar_region_graph_t::init(double planethresh)
{
	this->planarity_threshold = planethresh;
}
		
int planar_region_graph_t::populate(const node_boundary_t& boundary)
{
	return -1; // TODO
}

/*------------------*/
/* helper functions */
/*------------------*/

double planar_region_graph_t::get_face_planarity(const node_face_t& f)
{
	return 0; // TODO
}
		 
void planar_region_graph_t::get_isosurface_pos(const node_face_t& f,
						Vector3d& p)
{
	double mu_i, int_hw, mu_e, ext_hw, mu_s;
	Vector3d normal;

	/* check validity of argument */
	if(f.interior == NULL || f.interior->data == NULL
			|| (f.exterior != NULL && f.exterior->data == NULL))
	{
		/* invalid data given */
		cerr << "[planar_region_graph_t::get_isosurface_pos]\t"
		     << "Given invalid face" << endl;
		return;
	}
	
	/* get the values we need from the originating octdata structs */
	mu_i  = f.interior->data->get_probability(); /* mean interior val */
	int_hw = f.interior->halfwidth;
	if(f.exterior == NULL)
	{
		/* exterior is a null node, which should be counted
		 * as an unobserved external node */
		mu_e   = 0.5; /* value given to unobserved nodes */
		ext_hw = 0;
	}
	else
	{
		/* exterior node exists, get its data */
		mu_e   = f.exterior->data->get_probability();
		ext_hw = f.exterior->halfwidth;
	}
	
	/* get properties of the face */
	f.get_center(p);
	octtopo::cube_face_normals(f.direction, normal);

	/* the face's position is based on where we expect the 0.5 value
	 * to be if we interpolated the pdf between the centers of the
	 * two nodes given.
	 *
	 * The isosurface position would be:
	 *
	 * 	s = (p_i - 0.5) / (p_i - p_e)
	 *	p = <interior center> + norm * s * (int_hw + ext_hw)
	 *
	 * (remember, the norm points from the interior into the exterior)
	 *
	 * Note that:
	 *
	 * 	p_i ~ Guass(mu_i, var_i)
	 * 	p_e ~ Gauss(mu_e, var_e)
	 *
	 * We want to compute the expected position for the isosurface mark,
	 * which would be:
	 *
	 * 	s = (mu_i - 0.5) / (mu_i - mu_e)
	 *
	 * (due to linearization approximation used)
	 *
	 * So we can set the actual isosurface position to be:
	 */
	mu_s = (mu_i - 0.5) / (mu_i - mu_e);
	p += normal*mu_s*(int_hw + ext_hw);
}

double planar_region_graph_t::get_face_pos_var(const node_face_t& f)
{
	double mu_i, var_i, int_hw, mu_e, var_e, ext_hw, mu_s, var_s;
	double ss, var_p;

	/* check validity of argument */
	if(f.interior == NULL || f.interior->data == NULL
			|| (f.exterior != NULL && f.exterior->data == NULL))
	{
		/* invalid data given */
		cerr << "[planar_region_graph_t::get_face_pos_var]\t"
		     << "Given invalid face" << endl;
		return DBL_MAX;
	}

	/* get the values we need from the originating octdata structs */
	mu_i  = f.interior->data->get_probability(); /* mean interior val */
	var_i = f.interior->data->get_uncertainty(); /* variance of 
	                                              * interior value */
	int_hw = f.interior->halfwidth;
	if(f.exterior == NULL)
	{
		/* exterior is a null node, which should be counted
		 * as an unobserved external node */
		mu_e   = 0.5; /* value given to unobserved nodes */
		var_e  = 1.0; /* maximum variance for a value in [0,1] */
		ext_hw = 0;
	}
	else
	{
		/* exterior node exists, get its data */
		mu_e   = f.exterior->data->get_probability();
		var_e  = f.exterior->data->get_uncertainty();
		ext_hw = f.exterior->halfwidth;
	}

	/* the face's position is based on where we expect the 0.5 value
	 * to be if we interpolated the pdf between the centers of the
	 * two nodes given.
	 *
	 * The isosurface position would be:
	 *
	 * 	s = (p_i - 0.5) / (p_i - p_e)
	 *	p = <interior center> + norm * s * (int_hw + ext_hw)
	 *
	 * (remember, the norm points from the interior into the exterior)
	 *
	 * Note that:
	 *
	 * 	p_i ~ Guass(mu_i, var_i)
	 * 	p_e ~ Gauss(mu_e, var_e)
	 *
	 * We want to compute the variance of s.  For ease of computation,
	 * we linearize s(p_i,p_e) about (p_i,p_e) = (mu_i,mu_e), which
	 * gives us:
	 *
	 * 	mu_s  = (mu_i - 0.5) / (mu_i - mu_e)
	 * 	var_s = (1-mu_s^2)*var_i + mu_s^2*var_e
	 * 		- 2*(0.5-mu_e)*(0.5-mu_i)/(mu_i-mu_e)^2*cov(p_i,p_e)
	 *
	 * If we assume that p_i is indepentent from p_e, then:
	 *
	 * 	var_s = (1-mu_s^2)*var_i + mu_s^2*var_e
	 */
	mu_s = (mu_i - 0.5) / (mu_i - mu_e);
	ss = mu_s*mu_s;
	var_s = (1-ss)*var_i + ss*var_e;

	/* now we need to scale this quantity based on the size of the
	 * nodes being analyzed for this face */
	ss = (int_hw + ext_hw);
	var_p = ss*ss*var_s; /* for variance, multiply by square of coef. */
	
	/* return the computed value */
	return var_p;
}
