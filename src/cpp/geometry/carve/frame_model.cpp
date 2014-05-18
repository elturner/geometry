#include "frame_model.h"
#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/shapes/carve_wedge.h>
#include <geometry/shapes/chunk_exporter.h>
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <geometry/pca/line_fit.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <stdlib.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

/**
 * @file frame_model.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the frame_model_t class, which houses probabilistic
 * information for each scan-point in a frame.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

frame_model_t::frame_model_t()
{
	/* initialize empty structure */
	this->num_points = 0;
	this->map_list = NULL;
	this->is_valid.clear();
}
		
frame_model_t::~frame_model_t()
{
	/* free list if allocated */
	if(this->map_list != NULL)
		delete[] (this->map_list);
	this->map_list = NULL;
	this->is_valid.clear();
}
		
int frame_model_t::init(const fss::frame_t& frame, double ang,
                        double linefit, scan_model_t& model,
                        const system_path_t& path)
{
	noisy_scanpoint_t point;
	unsigned int i;
	int ret;

	/* prepare model for this frame */
	ret = model.set_frame(frame.timestamp, path);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* allocate points if necessary */
	if(frame.points.size() != this->num_points)
	{
		/* check if we already have a list allocated */
		if(this->map_list != NULL)
			delete[] (this->map_list);

		/* allocate a map list */
		this->num_points = frame.points.size();
		this->map_list = new carve_map_t[this->num_points];
		this->is_valid.resize(this->num_points);
	}

	/* iterate over points in frame */
	for(i = 0; i < this->num_points; i++)
	{
		/* get statistics of i'th point */
		point.set(frame.points[i].x,
		          frame.points[i].y,
			  frame.points[i].z,
			  frame.points[i].stddev,
			  frame.points[i].width);
		
		/* check if this point is valid or not */
		this->is_valid[i] = point.has_finite_noise();
		if(!(this->is_valid[i]))
			continue;

		/* model the combined statistics of this point */
		model.set_point(point);

		/* generate a carve map from statistical model */
		model.populate(this->map_list[i]);
	}

	/* analyze planar and corner features within frame */
	ret = this->compute_planar_probs(linefit, ang);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	// TODO do corner features

	/* success */
	return 0;
}
		
void frame_model_t::swap(frame_model_t& other)
{
	carve_map_t* ptr;
	unsigned int np;

	/* swap bool list */
	std::swap(this->is_valid, other.is_valid);

	/* swap sizes */
	np = this->num_points;
	this->num_points = other.num_points;
	other.num_points = np;

	/* swap arrays */
	ptr = this->map_list;
	this->map_list = other.map_list;
	other.map_list = ptr;
}
	
/*----------*/
/* geometry */
/*----------*/

int frame_model_t::export_chunks(octree_t& tree, const frame_model_t& next,
                          double buf, chunk_exporter_t& chunker) const
{
	carve_wedge_t wedge;
	vector<chunk::point_index_t> vals;
	unsigned int i, ta, tb, na, nb, n;
	int ret;

	/* iterate over every edge in this scan */
	n = this->num_points - 1;
	for(i = 0; i < n; i++)
	{
		/* get indices for this wedge */
		this->find_wedge_indices(i, next, ta, tb, na, nb);

		/* prepare wedge index vals
		 *
		 * If we wanted to store all point indices, we would use:
		 * 	
		 * 	vals.resize(4);
		 * 	vals[0].set(hash(si, fi,   ta));
		 * 	vals[1].set(hash(si, fi,   tb));
		 * 	vals[2].set(hash(si, fi+1, na));
		 * 	vals[3].set(hash(si, fi+1, nb));
		 *
		 * For sensor index si, frame index fi, point index i
		 *
		 * But since we're interested in regenerating wedges
		 * from these chunks, we want to store the wedge index,
		 * not the point index.
		 */
		vals.resize(1);
		vals[0].set(i);

		/* generate a wedge from two points in the current
		 * scan and two points in the next scan */
		wedge.init(&(this->map_list[ta]), &(this->map_list[tb]),
		           &(next.map_list[na]), &(next.map_list[nb]), buf);

		/* set the chunker to this shape */
		chunker.set(&wedge, vals);

		/* insert this chunker into the tree to chunk intersected
		 * volume */
		ret = tree.insert(chunker);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}

int frame_model_t::carve(octree_t& tree, const frame_model_t& next,
                         double buf) const
{
	unsigned int i, n;
	int ret;

	/* iterate over every edge in this scan */
	n = this->num_points - 1;
	for(i = 0; i < n; i++)
	{
		/* carve the i'th wedge */
		ret = this->carve_single(tree, next, buf, i);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}
		
int frame_model_t::carve(octnode_t* node, unsigned int depth,
                         const frame_model_t& next, double buf) const
{
	unsigned int i, n;
	int ret;

	/* iterate over every edge in this scan */
	n = this->num_points - 1;
	for(i = 0; i < n; i++)
	{
		/* carve the i'th wedge */
		ret = this->carve_single(node, depth, next, buf, i);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	return 0;
}
		
int frame_model_t::carve_single(octree_t& tree, const frame_model_t& next,
                                double buf, unsigned int i) const
{
	carve_wedge_t wedge;
	unsigned int ta, tb, na, nb;
	int ret;

	/* get indices for this wedge */
	this->find_wedge_indices(i, next, ta, tb, na, nb);

	/* generate a wedge from two points in the current
	 * scan and two points in the next scan */
	wedge.init(&(this->map_list[ta]), &(this->map_list[tb]),
	           &(next.map_list[na]), &(next.map_list[nb]), buf);

	/* carve this wedge in the tree */
	ret = tree.insert(wedge);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}
		
int frame_model_t::carve_single(octnode_t* node, unsigned int depth,
                                const frame_model_t& next, double buf,
                                unsigned int i) const
{
	carve_wedge_t wedge;
	unsigned int ta, tb, na, nb;

	/* check validity of input */
	if(node == NULL)
		return -1;

	/* get indices for this wedge */
	this->find_wedge_indices(i, next, ta, tb, na, nb);

	/* generate a wedge from two points in the current
	 * scan and two points in the next scan */
	wedge.init(&(this->map_list[ta]), &(this->map_list[tb]),
	           &(next.map_list[na]), &(next.map_list[nb]), buf);

	/* carve this wedge in the tree */
	node->insert(wedge, depth);

	/* success */
	return 0;
}

/*-----*/
/* i/o */
/*-----*/

int frame_model_t::serialize_carvemaps(cm_io::writer_t& cos) const
{
	int ret;

	/* write the carve maps from this frame to file */
	ret = cos.write_frame(this->map_list, this->num_points,
			this->is_valid);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int frame_model_t::serialize_wedges(wedge::writer_t& wos,
				unsigned int curr_index,
				const frame_model_t& next) const 
{
	carve_wedge_t wedge;
	vector<int> my_index_map, next_index_map;
	unsigned int i, n, num_valid, ta, tb, na, nb;

	/* determine the new set of indices for just the valid points */
	n = this->num_points;
	my_index_map.resize(n);
	num_valid = 0;
	for(i = 0; i < n; i++)
	{
		/* set to current count of valid indices */
		my_index_map[i] = num_valid;
		
		/* only increment if this too is valid */
		if(this->is_valid[i])
			num_valid++;
	}
	
	/* determine index mapping for next frame as well */
	n = next.num_points;
	next_index_map.resize(n);
	num_valid = 0;
	for(i = 0; i < n; i++)
	{
		/* set to current count of valid indices */
		next_index_map[i] = num_valid;

		/* only increment if this too is valid */
		if(next.is_valid[i])
			num_valid++; /* inc. valids */
	}

	/* iterate over the wedges between these frames */
	n = this->num_points - 1; /* number of edges in scan frame */
	for(i = 0; i < n; i++)
	{
		/* get the indices for this wedge */
		this->find_wedge_indices(i, next, ta, tb, na, nb);
		
		/* export this wedge to file, using the new indices
		 * of just the valid carve maps */
		ta = my_index_map[ta];
		tb = my_index_map[tb];
		na = next_index_map[na];
		nb = next_index_map[nb];
		wos.write(curr_index, ta, tb, curr_index+1, na, nb);	
	}

	/* return the number of wedges exported */
	return n;
}

/*-----------*/
/* debugging */
/*-----------*/

int frame_model_t::writeobj(const string& fn) const
{
	ofstream outfile;
	unsigned int i;

	/* open file for writing */
	outfile.open(fn.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* iterate over points, exporting them */
	for(i = 0; i < this->num_points; i++)
		if(this->is_valid[i])
			this->map_list[i].writeobj(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
		
void frame_model_t::find_wedge_indices(unsigned int i,
		                       const frame_model_t& next,
		                       unsigned int& ta,
		                       unsigned int& tb,
		                       unsigned int& na,
		                       unsigned int& nb) const
{
	unsigned int tn = this->num_points - 1;
	unsigned int nn = next.num_points - 1;

	/* each of the following perturbs the positions of the
	 * given indices so that they reside on valid points.  They
	 * must be perturbed in both directions in order to account
	 * for the edge case of the very first or very last points
	 * being invalid.  For the example of the first N points being
	 * invalid, an input of N-1 will iterate backwards until it
	 * finds that all previous points are invalid, then it will
	 * iterate forwards until finding a valid point. */

	/* get the scanpoints to use from each frame, checking
	 * for and avoiding bad scan points */
	ta = na = i;
	tb = nb = i+1;
	while(!this->is_valid[ta] && ta > 0)
		ta--;
	while(!this->is_valid[ta] && ta < tn)
		ta++;
	while(!this->is_valid[tb] && tb < tn)
		tb++;
	while(!this->is_valid[tb] && tb > 0)
		tb--;

	/* do the same thing for the next frame, checking for
	 * bad scan points */
	while(!next.is_valid[na] && na > 0)
		na--;
	while(!next.is_valid[na] && na < nn)
		na++;
	while(!next.is_valid[nb] && nb < nn)
		nb++;
	while(!next.is_valid[nb] && nb > 0)
		nb--;
}
		
int frame_model_t::compute_planar_probs(double dist, double ang)
{
	double r, d2, e, s;
	line_fit_t line_model;
	Vector3d disp, neigh_pos;
	const Vector3d* imp, *jmp;
	vector<const Vector3d*> neighs; 
	unsigned int i;
	int j, j_min, j_max, m;

	/* verify that we have a valid scan frame */
	if(this->num_points == 0 || this->map_list == NULL)
		return -1;

	/* iterate over the points */
	d2 = dist * dist;
	for(i = 0; i < this->num_points; i++)
	{
		/* get the range of the current scanpoint from its sensor */
		r = this->map_list[i].get_range(); 

		/* get the neighborhood of points that can contribute
		 * to the planarity estimate of the i'th point */
		
		/* because the points are assumed to be spaced equally
		 * angularly, then we only need to check points within
		 * this index range to see if they fall within dist of
		 * the current point. */
		m = (int) floor(atan(dist/r)/ang);
		j_min = i-m;
		if(j_min < 0)
			j_min = 0;
		j_max = i+m;
		if(j_max >= (int) this->num_points) 
			j_max = this->num_points-1;
		
		/* check points to see if they are within dist of i */
		neighs.clear();
		imp = this->map_list[i].get_scanpoint_mean_ptr();
		for(j = j_min; j <= j_max; j++)
		{
			/* get displacement bewtween points */
			jmp = this->map_list[j].get_scanpoint_mean_ptr();
			disp = (*imp) - (*jmp);
			if(disp.squaredNorm() <= d2)
				neighs.push_back(jmp);
		}

		/* fit a line to the neighborhood */
		line_model.fit(neighs);
	
		/* iterate over neighbors, compute normalized error
		 * of each neighbor point to line.  The normalized error
		 * is computed as the distance to the line divided by
		 * the isotropic std. dev. of the point */
		e = 0;
		for(j = j_min; j <= j_max; j++)
		{
			/* get isotropic std. dev of point */
			s = sqrt(this->map_list[j].get_scanpoint_var());

			/* compute normalized error */
			this->map_list[j].get_scanpoint_mean(neigh_pos);
			e += line_model.distance(neigh_pos) / s;
		}
		e /= (1 + j_max - j_min);

		/* store the probability in object structure
		 *
		 *
		 * if we treat e as a sample from a unit gaussian, then 
		 * we can express the probability of the line fit as the
		 * probability mass greater than e (that is, if the points
		 * are likely to be at least as far out as the line, then 
		 * the line is a good fit)
		 *
		 *                           |   |   |                   
		 *                           | --|-- |                      
		 *                           |/  |  \|                    
		 *                          _/   |   \_                     
		 *              _______-----#|   |   |#-----________   
		 *              #############|   |   |##############   
		 * --------------------------------------------------------
		 *                          -e       e   
		 *
		 * p = 2 * cdf(-e) = erf(-e)+1
		 */
		this->map_list[i].set_planar_prob(erf(-e)+1);
	}

	/* success */
	return 0;
}
