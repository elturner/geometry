#include "wall_sampling.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>

/**
 * @file   wall_sample.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains wall sampling classes
 *
 * @section DESCRIPTION
 *
 * This file contains classes that are used to represent
 * a 2D wall sampling of a scanned environment.  Wall samples
 * are a set of 2D points that dictate estimates of the positions
 * of strong vertical surfaces in the environment (which should
 * represent walls).
 */

using namespace std;

/* the following definitions are used for these classes */
#define DEFAULT_WALL_SAMPLE_RESOLUTION 0.05 /* units: meters */

/*----------------------------------------*/
/* wall sampling function implementations */
/*----------------------------------------*/

wall_sampling_t::wall_sampling_t()
{
	this->init(DEFAULT_WALL_SAMPLE_RESOLUTION, 0, 0, 0);
}
		
wall_sampling_t::wall_sampling_t(double res)
{
	this->init(res, 0, 0, res);
}

wall_sampling_t::wall_sampling_t(double res, double x, double y)
{
	this->init(res, x, y, res);
}

wall_sampling_t::wall_sampling_t(double res, double x, double y, double hw)
{
	this->init(res, x, y, hw);
}

void wall_sampling_t::init(double res, double x, double y, double hw)
{
	/* clear any existing samples, since we're about to overwrite
	 * the parameters of this sampling map */
	this->clear();

	/* set the parameters of this class */
	this->resolution = res;
	this->center_x   = x;
	this->center_y   = y;
	this->halfwidth  = res;
	this->set_halfwidth(hw);
}
		
void wall_sampling_t::set_halfwidth(double hw)
{
	/* initialize the halfwidth if it is non-positive */
	if(this->halfwidth <= 0)
		this->halfwidth = this->resolution;
	
	/* the halfwidth must be a power of two of the resolution,
	 * so it can only ever grow by factors of two */
	while(this->halfwidth < hw)
		this->halfwidth *= 2;
}
		
wall_sample_t wall_sampling_t::add(double x, double y,
				double z_min, double z_max, double w)
{
	pair<wall_sample_map_t::iterator, bool> ins;
	wall_sample_map_t::iterator it;
	wall_sample_t key;

	/* find the sample in the map.  This may require
	 * creating a new map entry */
	key.init(x, y, this->resolution, this->center_x, this->center_y);
	it = this->samples.find(key);
	if(it == this->samples.end())
		/* create a new entry for this sample */
		it = this->samples.insert(pair<wall_sample_t, 
				wall_sample_info_t>(key, 
					wall_sample_info_t())).first;
	
	/* update the info with the provided sample */
	it->second.add(x, y, w);
	it->second.add_zs(z_min, z_max);

	/* update the halfwidth */
	this->set_halfwidth(max(abs(x-this->center_x),
				abs(y-this->center_y)));

	/* return the wall sample */
	return key;
}
		 
wall_sample_t wall_sampling_t::add(double x, double y, size_t ind)
{	
	wall_sample_t key;

	/* find the sample in the map.  This may require
	 * creating a new map entry */
	key.init(x, y, this->resolution, this->center_x, this->center_y);
	this->add(key, ind);

	/* return the wall sample */
	return key;
}
		 
void wall_sampling_t::add(const wall_sample_t& ws, size_t ind)
{
	wall_sample_map_t::iterator it;
	
	/* find in map */
	it = this->samples.find(ws);
	if(it == this->samples.end())
		/* create a new entry for this sample */
		it = this->samples.insert(pair<wall_sample_t, 
				wall_sample_info_t>(ws, 
					wall_sample_info_t())).first;
	
	/* update the info with the provided sample */
	it->second.add_pose(ind);
}
		
wall_sample_map_t::const_iterator
			wall_sampling_t::find(double x, double y) const
{ 
	return this->find(wall_sample_t(x, y,
			this->resolution, this->center_x, this->center_y)); 
}
		
int wall_sampling_t::writedq(const string& filename) const
{
	wall_sample_map_t::const_iterator it;
	ofstream outfile;

	/* prepare to write to the given file */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write out the file header */
	outfile << this->get_max_depth()                   << endl
	        << this->halfwidth                         << endl
	        << this->center_x << " " << this->center_y << endl;

	/* iterate through the wall samples */
	for(it = this->samples.begin(); it != this->samples.end(); it++)
		it->second.writedq(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
		
size_t wall_sampling_t::get_max_depth() const
{
	return (size_t) ceil(log(  (2.0*this->halfwidth) 
					/ (this->resolution)      )
				/ log(2.0) );
}

/*--------------------------------------*/
/* wall sample function implementations */
/*--------------------------------------*/

wall_sample_t::wall_sample_t()
{
	this->init(0,0);
}
		
wall_sample_t::wall_sample_t(int xxi, int yyi)
{
	this->init(xxi,yyi);
}
		
wall_sample_t::wall_sample_t(double xx, double yy, 
		double res, double cx, double cy)
{
	this->init(xx,yy,res,cx,cy);
}

void wall_sample_t::init(double xx, double yy,
		double res, double cx, double cy)
{
	/* convert from continuous position to gridcell index.
	 *
	 * Note that grid cells just discretized representations
	 * of the continous values, so we perform the following:
	 */
	this->xi = (int) floor((xx-cx) / res);
	this->yi = (int) floor((yy-cy) / res);
}

/*-------------------------------------------*/
/* wall sample info function implementations */
/*-------------------------------------------*/
		
wall_sample_info_t::wall_sample_info_t()
{
	/* initialize to be empty */
	this->clear();
}

void wall_sample_info_t::clear()
{
	/* reset the total weight to be zero */
	this->total_weight = 0;

	/* reset the z ranges to be invalid */
	this->z_min = 1;
	this->z_max = 0; /* invalid when min > max */

	/* clear all pose info */
	this->poses.clear();
}
		
void wall_sample_info_t::add(double x, double y, double w)
{
	/* perform a weighted average between the existing
	 * samples and the new sample */
	this->x_avg = ( (w*x) + (this->total_weight*this->x_avg) )
			/ (w + this->total_weight);
	this->y_avg = ( (w*y) + (this->total_weight*this->y_avg) )
			/ (w + this->total_weight);
	this->total_weight += w;
}
		
void wall_sample_info_t::add_zs(double z0, double z1)
{
	/* check if input is valid */
	if(z0 > z1)
		return; /* ignore invalid range */

	/* check if existing range is valid */
	if(this->z_min > this->z_max)
	{
		/* since we didn't have any valid ranges to begin
		 * with, just copy over this range */
		this->z_min = z0;
		this->z_max = z1;
		return;
	}

	/* merge the two valid ranges */
	this->z_min = min(z0, this->z_min);
	this->z_max = max(z1, this->z_max);
}

void wall_sample_info_t::writedq(ostream& os) const
{
	set<size_t>::const_iterator it;
	size_t w;

	/* get the weight of this sample as an integer */
	w = (int) this->total_weight;

	/* if weight is less than one, don't write out this sample,
	 * since it's clearly not well represented */
	if(w <= 0)
		return;

	/* export this info to a single line in the
	 * given dq file stream */
	os << this->x_avg << " " /* center position (x) */
	   << this->y_avg << " " /* center position (y) */
	   << this->z_min << " " /* minimum z value */
	   << this->z_max << " " /* maximum z value */
	   << w << " " /* "num_points" field */
	   << this->poses.size(); /* number of pose indices */

	/* write out each pose index */
	for(it = this->poses.begin(); it != this->poses.end(); it++)
		os << " " << (*it);

	/* export a new line, indicating end of sample */
	os << endl;
}
