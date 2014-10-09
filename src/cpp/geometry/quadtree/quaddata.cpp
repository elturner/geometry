#include "quaddata.h"
#include <Eigen/Dense>
#include <iostream>
#include <set>

/**
 * @file   quaddata.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to define data in quadtrees
 *
 * @section DESCRIPTION
 *
 * This file defines the quaddata_t structure.  These are the
 * data elements stored at the leafs of quadtrees.
 */

using namespace std;
using namespace Eigen;

/*--------------------*/
/* QUADDATA FUNCTIONS */
/*--------------------*/

quaddata_t::quaddata_t()
{
	/* set all counters to zero */
	this->average << 0,0,0;
	this->total_weight = 0;
	this->normal << 0,0,0;

	/* set heights to invalid */
	this->min_z = 1;
	this->max_z = 0;
}

quaddata_t::~quaddata_t() { /* no work necessary */ }

void quaddata_t::add(const Eigen::Vector2d& p, 
			const Eigen::Vector2d& n, double w)
{
	double tw;
	int i;

	/* compute the new total weight */
	tw = this->total_weight + w;

	/* add to average and normals */
	this->average = ( (this->total_weight * this->average)
					+ (p*w) ) / tw;
	this->normal = ( (this->normal*this->total_weight) 
					+ (n*w) ) / tw;
	this->total_weight = tw;
}
		
void quaddata_t::add(double miz, double maz)
{
	/* ignore input if invalid */
	if(miz > maz)
		return;

	/* check if we currently have invalid ranges */
	if(this->min_z > this->max_z)
	{
		/* just copy values */
		this->min_z = miz;
		this->max_z = maz;
		return;
	}

	/* compute union */
	this->min_z = (miz < this->min_z) ? miz : this->min_z;
	this->max_z = (maz > this->max_z) ? maz : this->max_z;
}
	
quaddata_t* quaddata_t::clone() const
{
	quaddata_t* c;

	/* allocate new memory */
	c               = new quaddata_t();

	/* copy values */
	c->average      = this->average;
	c->normal       = this->normal;
	c->total_weight = this->total_weight;
	c->pose_inds.insert(this->pose_inds.begin(), this->pose_inds.end());
	c->min_z        = this->min_z;
	c->max_z        = this->max_z;

	/* return clone */
	return c;
}
	
void quaddata_t::print(ostream& os) const
{
	set<unsigned int>::iterator it;
	size_t num;

	/* convert continuous weight to integer */
	num = (size_t) (this->total_weight);

	/* print static info */
	os << this->average(0)   << " "
	   << this->average(1)   << " "
	   << this->min_z        << " "
	   << this->max_z        << " "
	   << num                << " "
	   << this->pose_inds.size();

	/* write poses */
	for(it = this->pose_inds.begin(); it != this->pose_inds.end(); it++)
		os << " " << (*it);
}
