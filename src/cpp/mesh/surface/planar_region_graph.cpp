#include "planar_region_graph.h"
#include <geometry/octree/octdata.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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
#define DEFAULT_DISTANCE_THRESHOLD 1.0

/*--------------------------*/
/* function implementations */
/*--------------------------*/
		
planar_region_graph_t::planar_region_graph_t()
{
	this->init(DEFAULT_PLANARITY_THRESHOLD, DEFAULT_DISTANCE_THRESHOLD);
}

void planar_region_graph_t::init(double planethresh, double distthresh)
{
	this->planarity_threshold = planethresh;
	this->distance_threshold = distthresh;
}
		
int planar_region_graph_t::populate(const node_boundary_t& boundary)
{
	facemap_t::const_iterator it;
	faceset_t::const_iterator fit, nit;
	pair<faceset_t::const_iterator, faceset_t::const_iterator> neighs;
	seedmap_t::const_iterator neigh_seed;
	regionmap_t::iterator rit;
	set<node_face_t> blacklist;
	pair<regionmap_t::iterator, bool> ins;

	/* first, initialize the regions by performing flood fill to
	 * generate the basic regions properties */
	for(it = boundary.begin(); it != boundary.end(); it++)
	{
		/* check if face has been used already */
		if(blacklist.count(it->first))
			continue; /* this face is already in a region */

		/* create a new region with this face as the seed */
		ins = this->regions.insert(pair<node_face_t, 
				planar_region_info_t>(it->first,
				planar_region_info_t(it->first, 
					boundary, blacklist)));
		if(!(ins.second))
		{
			/* this means that it->first was already in the
			 * map, even though it wasn't in the blacklist.
			 * This means we have conflicting info */
			return -1;
		}

		/* add each node face in this region to the seedmap
		 * structure */
		for(fit = ins.first->second.region.begin(); 
			fit != ins.first->second.region.end(); fit++)
		{
			/* for each face in this region, add to seed map */
			this->seeds[*fit] = it->first; /* seed for region */
		}
	}

	/* compute neighbor information for each region
	 *
	 * This requires us to iterate over each region, then iterate
	 * over the faces in that region, then iterate over the nodes in
	 * that face, then iterate over the neighbors of that face, and 
	 * determine if those neighbors are part of different regions. */
	for(rit = this->regions.begin(); rit != this->regions.end(); rit++)
	{
		/* iterate over the faces of this region */
		for(fit = rit->second.region.begin();
				fit != rit->second.region.end(); fit++)
		{
			/* get neighbor information for this face */
			neighs = boundary.get_neighbors(*fit);

			/* now iterate over the neighbors of this face */
			for(nit = neighs.first; nit != neighs.second; nit++)
			{
				/* determine which region this neighbor
				 * lies in, by getting the seed value
				 * that is stored in the seeds map */
				neigh_seed = this->seeds.find(*nit);
				if(neigh_seed == this->seeds.end())
				{
					/* every face should be in
					 * the seeds map, which means
					 * this is an error */
					return -2;
				}

				/* check if the neighbor's seed is
				 * different than this region's */
				if(neigh_seed->second == rit->first)
				{
					/* seeds are the same, which
					 * means that the two faces
					 * belong to the same region,
					 * so we should do nothing */
					continue;
				}

				/* the seeds are different, which means 
				 * these two regions are neighbors.  We
				 * should record this */
				rit->second.neighbor_seeds.insert(
						neigh_seed->second);
			}
		}
	}	

	/* success */
	return 0;
}
		
int planar_region_graph_t::coalesce_regions()
{
	return -1; // TODO implement me
}
		
int planar_region_graph_t::writeobj(const std::string& filename) const
{
	ofstream outfile;
	regionmap_t::const_iterator it;

	/* prepare to export to file */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* generate the regions based on floodfill */
	for(it = this->regions.begin(); it != this->regions.end(); it++)
		it->second.region.writeobj(outfile); /* write to file */

	/* success */
	outfile.close();
	return 0;
}

/*------------------*/
/* helper functions */
/*------------------*/

double planar_region_graph_t::get_face_planarity(const node_face_t& f)
{	
	double mu_i, planar_i, mu_e, planar_e, s;
	
	/* check validity of argument */
	if(f.interior == NULL || f.interior->data == NULL
			|| (f.exterior != NULL && f.exterior->data == NULL))
	{
		/* invalid data given */
		cerr << "[planar_region_graph_t::get_isosurface_pos]\t"
		     << "Given invalid face" << endl;
		return 0;
	}
	
	/* get the values we need from the originating octdata structs */
	mu_i = f.interior->data->get_probability(); /* mean interior val */
	planar_i = f.interior->data->get_planar_prob();
	if(f.exterior == NULL)
	{
		/* exterior is a null node, which should be counted
		 * as an unobserved external node */
		mu_e     = 0.5; /* value given to unobserved nodes */
		planar_e = planar_i; /* just use the same value */
	}
	else
	{
		/* exterior node exists, get its data */
		mu_e     = f.exterior->data->get_probability();
		planar_e = f.exterior->data->get_planar_prob();
	}

	/* In order to get the planarity value for the face,
	 * we perform a weighted average between the values for
	 * the two nodes that define this face.  The first step 
	 * is to find this weighting with respect to the distance
	 * of the isosurface between the node centers.  This
	 * value can be determined with:
	 *
	 * 	s = (p_i - 0.5) / (p_i - p_e)
	 *
	 * Where s is the weighting for the exterior node, and (1-s)
	 * is the weighting for the interior node.
	 */
	s = (mu_i - 0.5) / (mu_i - mu_e);

	/* weight the node's values */
	return s*planar_e + (1-s)*planar_i; 
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

/*-----------------------------------------------*/
/* planar_region_info_t function implementations */
/*-----------------------------------------------*/

planar_region_info_t::planar_region_info_t(const node_face_t& f,
				const node_boundary_t& boundary,
				faceset_t& blacklist)
{
	/* initialize the region based on floodfill of this face */
	this->region.floodfill(f, boundary, blacklist);

	/* leave the neighbor seeds structure empty for now */
}

int planar_region_graph_t::compute_planefit(
				planar_region_pair_t& pair) const
{
	regionmap_t::const_iterator fit, sit;
	vector<Eigen::Vector3d, 
		Eigen::aligned_allocator<Eigen::Vector3d> > centers;
	vector<double> variances;
	size_t i, n;
	double d;

	/* get the regions referenced in the input pair */
	fit = this->regions.find(pair.first);
	if(fit == this->regions.end())
		return -1;
	sit = this->regions.find(pair.second);
	if(fit == this->regions.end())
		return -2;

	/* get the center points for each face in each of the regions */
	fit->second.region.find_face_centers(centers, variances);
	sit->second.region.find_face_centers(centers, variances);

	/* perform PCA on these points */
	pair.plane.fit(centers);

	/* determine the max error by iterating over the points */
	pair.max_err = 0;
	n = centers.size();
	for(i = 0; i < n; i++)
	{
		/* get normalized distance of this point to plane */
		d = pair.plane.distance_to(centers[i])/sqrt(variances[i]);

		/* check if this is the maximum */
		if(d > pair.max_err)
			pair.max_err = d;
	}

	/* success */
	return 0;
}
