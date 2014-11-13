#include "planar_region_graph.h"
#include <geometry/octree/octdata.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <float.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>

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
#define DEFAULT_DISTANCE_THRESHOLD  1.0
#define DEFAULT_FIT_TO_ISOSURFACE   false
#define APPROX_ZERO                 0.00001

/*--------------------------*/
/* function implementations */
/*--------------------------*/
		
planar_region_graph_t::planar_region_graph_t()
{
	this->init(DEFAULT_PLANARITY_THRESHOLD,
			DEFAULT_DISTANCE_THRESHOLD,
			DEFAULT_FIT_TO_ISOSURFACE,
			COALESCE_WITH_L_INF_NORM);
}

void planar_region_graph_t::init(double planethresh, double distthresh,
				bool fitiso, COALESCING_STRATEGY strat)
{
	this->planarity_threshold = planethresh;
	this->distance_threshold  = distthresh;
	this->fit_to_isosurface   = fitiso;
	this->strategy            = strat;
}
	
regionmap_t::const_iterator planar_region_graph_t::lookup_face(
					const node_face_t& f) const
{
	seedmap_t::const_iterator fit;

	/* look up face to find its seed */
	fit = this->seeds.find(f);
	if(fit == this->seeds.end())
		return this->end();

	/* get the region information */
	return this->regions.find(fit->second);
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
					boundary, blacklist,
					this->planarity_threshold)));
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
		/* ensure the normal is oriented for this region */
		rit->second.region.orient_normal();

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
	priority_queue<planar_region_pair_t> pq;
	regionmap_t::iterator rit, sit;
	faceset_t::iterator fit;
	planar_region_pair_t pair, old_pair;
	progress_bar_t progbar;
	size_t num_faces, i, pq_size, original_num_regions;
	int ret;

	/* prepare progress bar */
	progbar.set_name("Coalescing");
	progbar.set_color(progress_bar_t::BLUE);

	/* create a queue with all possible region merges */
	i = 0;
	for(rit = this->regions.begin(); rit != this->regions.end(); rit++)
	{
		/* update user on status */
		progbar.update(i++, this->regions.size());

		/* iterate through the neighbors of this region to
		 * get pairs of regions */
		for(fit = rit->second.neighbor_seeds.begin();
				fit != rit->second.neighbor_seeds.end(); 
					fit++)
		{
			/* to prevent duplication, only add a pair if the
			 * neighbor's seed is greater than the current
			 * region seed (since all linkages are 
			 * bidirectional, not doing this would result in
			 * twice as many pairs as needed). */
			if(*fit < rit->first)
				continue;

			/* generate the pairing of neighboring regions */
			pair.first = rit->first; /* this region */
			pair.second = *fit; /* the neighboring region */
			ret = this->compute_planefit(pair);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);

			/* add this pair to the queue */
			pq.push(pair);
		}
	}

	/* redo progress bar for next stage */
	progbar.clear();
	pq_size = 0;
	original_num_regions = this->regions.size();

	/* cycle through queue, get next region to merge */
	while(!pq.empty())
	{
		/* get next pair to check */
		pair = pq.top();
		pq.pop();

		/* update user on status */
		if(pq_size < pq.size())
			/* queue is growing */
			progbar.set_color(progress_bar_t::RED);
		else if(pq_size == pq.size())
			/* queue remains the same size */
			progbar.set_color(progress_bar_t::YELLOW);
		else
			/* queue is shrinking */
			progbar.set_color(progress_bar_t::GREEN);
		pq_size = pq.size();
		progbar.update(original_num_regions - this->regions.size(),
				original_num_regions);

		/* check if distance thresholds met.  if not, quit */
		if(pair.err > this->distance_threshold)
			break; /* all remaining pairs won't pass either */
		
		/* check for duplicate pairs in the queue -- only need
		 * to process each one once */
		if(pair.equivalent_to(old_pair))
			continue;
		old_pair = pair; /* duplicates will always be adjacent */

		/* get info for the two regions involved */
		rit = this->regions.find(pair.first);
		sit = this->regions.find(pair.second);

		/* check if regions still exist */
		if(rit == this->regions.end() || sit == this->regions.end())
			continue; /* not valid pair anymore */
		
		/* check if planarity threshold met.  If not, then
		 * abort this merge */
		if((rit->second.get_planarity() < this->planarity_threshold)
			|| (sit->second.get_planarity() 
				< this->planarity_threshold))
		{
			/* one or both of the input planes don't meet
			 * the planarity threshold, which indicates that
			 * the geometry described by these planes may not
			 * be well-represented by a plane, so they shouldn't
			 * be merged. */
			continue;
		}


		/* use checksum to see if we need to recalc plane */
		num_faces = rit->second.region.num_faces()
				+ sit->second.region.num_faces();
		if(num_faces != pair.num_faces)
		{
			/* regions have been modified, compute
			 * fitting planes the pair */
			ret = this->compute_planefit(pair);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);
	
			/* since we had to recompute the plane information,
			 * this pair may not actually be as low-cost as
			 * we thought.  We should reinsert it into the
			 * queue to see if it is still on top */
			if(pair.err <= this->distance_threshold)
				pq.push(pair);
			continue;
		}

		/* merge the regions */
		ret = this->merge_regions(pair);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);
		
		/* Insert new neighbor merges into the queue.
		 *
		 * Note that the second region in our pair no
		 * longer exists, so just insert neighbor-pairs
		 * of the first region. */
		for(fit = rit->second.neighbor_seeds.begin();
				fit != rit->second.neighbor_seeds.end(); 
					fit++)
		{
			/* generate the pairing of neighboring regions */
			pair.first = rit->first; /* this region */
			pair.second = *fit; /* the neighboring region */
			ret = this->compute_planefit(pair);
			if(ret)
				return PROPEGATE_ERROR(-4, ret);

			/* add this pair to the queue */
			if(pair.err <= this->distance_threshold)
				pq.push(pair);
		}
	}

	/* success */
	return 0;
}
		
int planar_region_graph_t::writeobj(const std::string& filename,
					bool project) const
{
	ofstream outfile;
	regionmap_t::const_iterator it;

	/* prepare to export to file */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write the faces for each region */
	for(it = this->regions.begin(); it != this->regions.end(); it++)
		it->second.region.writeobj(outfile, project); 
					/* write to file */
	
	/* success */
	outfile.close();
	return 0;
}
		
void planar_region_graph_t::writeobj_linkages(ostream& os) const
{
	regionmap_t::const_iterator it, nit;
	faceset_t::const_iterator fit;
	Vector3d p, n;
	int num_verts;

	/* iterate over the regions in this graph */
	for(it = this->regions.begin(); it != this->regions.end(); it++)
	{
		/* write vertex for this region */
		p = it->second.region.get_plane().point;
		n = it->second.region.get_plane().normal;
		n = p + 0.1*n;
		os << "v " << p.transpose() << " 0 255 0" << endl
		   << "v " << n.transpose() << " 255 255 255" << endl;
		num_verts = 0;

		/* iterate over the neighbors of this region */
		for(fit = it->second.neighbor_seeds.begin();
			fit != it->second.neighbor_seeds.end(); fit++)
		{
			/* get neighbor region info */
			nit = this->regions.find(*fit);
			if(nit == this->regions.end())
				continue; /* region doesn't exist */

			/* get center position for this neighbor seed */
			p = nit->second.region.get_plane().point;

			/* write arrow from this region to the neighbor */
			os << "v " << p.transpose() << " 255 0 0" << endl
			   << "f -1 " << (-num_verts-2) 
			   << " " << (-num_verts-3) << endl;
			num_verts++;
		}
	}
}

/*------------------*/
/* helper functions */
/*------------------*/

int planar_region_graph_t::compute_planefit(planar_region_pair_t& pair)
{
	std::vector<Eigen::Vector3d, 
			Eigen::aligned_allocator<Eigen::Vector3d> > centers;
	regionmap_t::iterator fit, sit;
	size_t i, nf;
	double d, v;

	/* get the regions referenced in the input pair */
	fit = this->regions.find(pair.first);
	if(fit == this->regions.end())
		return -1;
	sit = this->regions.find(pair.second);
	if(fit == this->regions.end())
		return -2;

	/* get the center points for each face in each of the regions.
	 *
	 * We only need to compute them if they are not already cached */
	if(fit->second.centers.size() != fit->second.region.num_faces())
	{
		/* get face center positions for first region */
		fit->second.centers.clear();
		fit->second.variances.clear();
		fit->second.region.find_face_centers(fit->second.centers,
					fit->second.variances,
					this->fit_to_isosurface);	
	}
	if(sit->second.centers.size() != sit->second.region.num_faces() )
	{
		/* get face center positions for second region */
		sit->second.centers.clear();
		sit->second.variances.clear();
		sit->second.region.find_face_centers(sit->second.centers,
					sit->second.variances,
					this->fit_to_isosurface);	
	}

	/* perform PCA on these points */
	centers.insert(centers.end(), fit->second.centers.begin(),
				fit->second.centers.end());
	centers.insert(centers.end(), sit->second.centers.begin(),
				sit->second.centers.end());
	pair.plane.fit(centers);
	
	/* verify the checksum */
	pair.num_faces = centers.size();
	if(pair.num_faces != fit->second.region.num_faces() 
				+ sit->second.region.num_faces())
		return -3;

	/* determine the error by iterating over the points */
	pair.err = 0;
	nf = fit->second.variances.size();
	for(i = 0; i < pair.num_faces; i++)
	{
		/* get variance of center position for face */
		v = (i >= nf) ? sit->second.variances[i-nf] 
			: fit->second.variances[i];

		/* get normalized distance of this point to plane */
		if(v <= 0)
			d = DBL_MAX;
		else
			d = pair.plane.distance_to(centers[i]) / sqrt(v);

		/* incorporate this error value into the total
		 * error for the planar region */
		switch(this->strategy)
		{
			default:
			case COALESCE_WITH_L_INF_NORM:
				/* check if this is the maximum */
				if(d > pair.err)
					pair.err = d;
				break;

			case COALESCE_WITH_L2_NORM:
				pair.err += d*d;
				break;
		}
	}

	/* finalize the error based on our stragety */
	switch(this->strategy)
	{
		default:
			/* don't need to do anything */
			break;

		case COALESCE_WITH_L2_NORM:
			/* we want the root-mean-squared-error */
			pair.err = sqrt(pair.err / pair.num_faces);
			break;
	}

	/* success */
	return 0;
}
		
int planar_region_graph_t::merge_regions(const planar_region_pair_t& pair)
{
	regionmap_t::iterator fit, sit, neighinfo;
	faceset_t::iterator faceit, nit;

	/* retrieve the region information to modify */
	fit = this->regions.find(pair.first);
	if(fit == this->regions.end())
		return -1; /* can't merge non-existent region */
	sit = this->regions.find(pair.second);
	if(sit == this->regions.end())
		return -2; /* can't merge non-existent region */

	/* iterate over the faces in the second region */
	for(faceit = sit->second.region.begin();
			faceit != sit->second.region.end(); faceit++)
	{
		/* add this face to the first region's set */
		fit->second.region.add(*faceit);

		/* update seed information for this face to first region */
		this->seeds[*faceit] = fit->first; /* now in first region */
	}
	
	/* verify that checksum matches */
	if(pair.num_faces != fit->second.region.num_faces())
		return -3;

	/* iterate over the neighbors of the second region */
	for(nit = sit->second.neighbor_seeds.begin();
			nit != sit->second.neighbor_seeds.end(); nit++)
	{
		/* get info for this neighbor */
		neighinfo = this->regions.find(*nit);
		if(neighinfo == this->regions.end())
			return -4;

		/* perform following only if neighbor isn't first region */
		if(*nit != fit->first)
		{
			/* add this neighbor to the first region */
			fit->second.neighbor_seeds.insert(*nit);

			/* add first region as neighbor of this neighbor */
			neighinfo->second.neighbor_seeds.insert(fit->first);
		}
		
		/* remove second region as a neighbor of this neighbor */
		neighinfo->second.neighbor_seeds.erase(sit->first);
	}

	/* copy cached values to the newly edited first region */
	fit->second.centers.insert(fit->second.centers.end(),
			sit->second.centers.begin(),
			sit->second.centers.end());
	fit->second.variances.insert(fit->second.variances.end(),
			sit->second.variances.begin(),
			sit->second.variances.end());
	fit->second.planarity = min(fit->second.get_planarity(), 
					sit->second.get_planarity());

	/* update region's plane information */
	fit->second.region.set_plane(pair.plane);
	fit->second.region.orient_normal(); /* normal points inwards */

	/* remove second region from this graph */
	this->regions.erase(sit);

	/* success */
	return 0;
}

/*-----------------------------------------------*/
/* planar_region_info_t function implementations */
/*-----------------------------------------------*/

planar_region_info_t::planar_region_info_t(const node_face_t& f,
				const node_boundary_t& boundary,
				faceset_t& blacklist, double planethresh)
{
	/* initialize planarity value to be not-yet-computed */
	this->planarity = -1;

	/* initialize the region based on floodfill of this face */
	this->region.floodfill(f, boundary, blacklist, planethresh);

	/* leave the neighbor seeds structure empty for now */
}
		
double planar_region_info_t::get_planarity()
{
	/* compute and cache the planarity */
	this->planarity = this->compute_planarity();

	/* return the computed value */
	return this->planarity;
}
		
double planar_region_info_t::compute_planarity() const
{
	faceset_t::iterator it;
	double p, p_best;

	/* check if the value has been cached */
	if(this->planarity >= 0)
		return this->planarity;

	/* compute the value and then return it */
	p_best = -1;
	for(it = this->region.begin(); it != this->region.end(); it++)
	{
		/* get the planarity value for the current face */
		p = it->get_planarity();

		/* incorporate into the region-wide planarity */
		if(p < p_best || p_best < 0)
			p_best = p; /* record the min */
	}

	/* return the computed planarity */
	return p_best;
}
