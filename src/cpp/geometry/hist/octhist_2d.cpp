#include "octhist_2d.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <iostream>
#include <map>

/**
 * @file     octhist_2d.cpp
 * @author   Eric Turner <elturner@eecs.berkeley.edu>
 * @brief    Performs a top-down 2D histogram of octree occupancy
 *
 * @section DESCRIPTION
 *
 * Will analyze an octree to determine the 2D histogram of occupancy
 * information in each node, projected onto the xy-axis.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int octhist_2d_t::init(octree_t& octree)
{
	int ret;

	/* apply resolution from the octree */
	ret = this->init(octree, octree.get_resolution());
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int octhist_2d_t::init(octree_t& octree, double res)
{
	/* first, clear any existing info */
	this->clear();

	/* initialize structure */
	if(res <= 0)
	{
		/* print error message */
		cerr << "[octhist_2d_t::init]\t"
		     << "Given invalid resolution" << endl;
		return -1; /* invalid resolution */
	}
	this->resolution = res;

	/* populate the histogram based on the contents of the octree */
	octree.find(*this);

	/* we have now successfully populated the histogram */
	return 0;
}

void octhist_2d_t::clear()
{
	/* clear all info */
	this->cells.clear();
	this->resolution = -1; /* invalid */
}
		
void octhist_2d_t::insert(double x, double y, double w)
{
	pair<histmap_t::iterator, bool> ins;
	index_t ind;

	/* compute the index of this position */
	ind = this->get_index(x, y);

	/* attempt to insert into map */
	ins = this->cells.insert(pair<index_t, double>(ind, w));
	if(ins.second == false)
	{
		/* value already exists, so just add to its weight */
		ins.first->second += w;
	}
}
		
void octhist_2d_t::writetxt(std::ostream& os) const
{
	histmap_t::const_iterator it;

	/* iterate through the map */
	for(it = this->cells.begin(); it != this->cells.end(); it++)
	{
		/* write out cell info */
		os << it->first.first  << " "
		   << it->first.second << " "
		   << it->second       << endl;
	}
}

octdata_t* octhist_2d_t::apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d)
{
	double w, x, y;

	/* ignore any nodes that don't have data */
	if(d == NULL)
		return d;

	/* also ignore nodes that have no count or no weight */
	if(d->get_count() == 0 || d->get_total_weight() <= 0)
		return d;

	/* ignore exterior nodes */
	if(!(d->is_interior()))
		return d;

	/* determine the weight of this node in this histogram,
	 * which is the vertical height of this node */
	w = 2 * hw;

	/* iterate over all histogram cells that vertically intersect
	 * this node */
	for( x = c(0) - hw + this->resolution/2 ; 
			x <= c(0) + hw ; x += this->resolution )
		for( y = c(1) - hw + this->resolution/2 ; 
			y <= c(1) + hw ; y += this->resolution )
		{
			/* add weight to this sample */
			this->insert(x, y, w);
		}
	
	/* don't modify the data value */
	return d;
}
