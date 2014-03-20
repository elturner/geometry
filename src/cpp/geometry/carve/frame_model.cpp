#include "frame_model.h"
#include <io/carve/chunk_io.h>
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/shapes/carve_wedge.h>
#include <geometry/shapes/chunk_exporter.h>
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <stdlib.h>
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
		
int frame_model_t::init(const fss::frame_t& frame,
                        scan_model_t& model, const system_path_t& path)
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
		this->is_valid.resize(this->num_points, true);
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
		if(!point.has_finite_noise())
		{
			/* this point is not valid */
			this->is_valid[i] = false;
			continue;
		}

		/* model the combined statistics of this point */
		model.set_point(point);

		/* generate a carve map from statistical model */
		model.populate(this->map_list[i]);
	}

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
                          double buf, unsigned int si, unsigned int fi,
                          chunk_exporter_t& chunker) const
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

		/* prepare point index vals */
		vals.resize(4);
		vals[0].set(si, fi, ta);
		vals[1].set(si, fi, tb);
		vals[2].set(si, fi, na);
		vals[3].set(si, fi, nb);

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
	carve_wedge_t wedge;
	unsigned int i, ta, tb, na, nb, n;
	int ret;

	/* iterate over every edge in this scan */
	n = this->num_points - 1;
	for(i = 0; i < n; i++)
	{
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
	}

	/* success */
	return 0;
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

	/* get the scanpoints to use from each frame, checking
	 * for and avoiding bad scan points */
	ta = na = i;
	tb = nb = i+1;
	while(!this->is_valid[ta] && ta > 0)
		ta--;
	while(!this->is_valid[tb] && tb < tn)
		tb++;

	/* do the same thing for the next frame, checking for
	 * bad scan points */
	while(!next.is_valid[na] && na > 0)
		na--;
	while(!next.is_valid[nb] && nb < nn)
		nb++;
}
