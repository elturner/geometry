#include "octhist_2d.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <geometry/shapes/bounding_box.h>
#include <geometry/poly_intersect/poly2d.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
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
	bounding_box_t bbox;
	index_t min_i, max_i; /* index bounds */
	progress_bar_t progbar;
	tictoc_t clk;

	/* first, clear any existing info */
	tic(clk);
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

	/* use the root node of the octree as a bounding box
	 * for the geometry */
	bbox.init(octree);
	min_i = this->get_index(bbox.get_min(0), bbox.get_min(1));
	max_i = this->get_index(bbox.get_max(0), bbox.get_max(1));
	toc(clk, "Finding bounding box");

	/* populate the histogram based on the contents of the octree
	 *
	 * This is performed by iterating over the possible cells
	 * of the histogram, and checking what each cell intersects
	 * in the octree. */
	tic(clk);
	progbar.set_name("Histogram");
	for(this->current_index.first = min_i.first;
			this->current_index.first <= max_i.first;
				this->current_index.first++)
	{
		/* inform user of progress at each new column */
		progbar.update(this->current_index.first - min_i.first, 
				max_i.first - min_i.first + 1);

		/* do each column */
		for(this->current_index.second = min_i.second;
			this->current_index.second <= max_i.second;
				this->current_index.second++)
		{
			/* get the histogram values for this cell */
			octree.find(*this);
		}
	}	

	/* we have now successfully populated the histogram */
	progbar.clear();
	toc(clk, "Populating octhist");
	return 0;
}

void octhist_2d_t::clear()
{
	/* clear all info */
	this->cells.clear();
	this->resolution = -1; /* invalid */

	/* set current cell to default value */
	this->current_index.first = 0;
	this->current_index.second = 0;
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
		
bool octhist_2d_t::intersects(const Vector3d& c, double hw) const
{ 
	double argx[2];
	double argy[2];
	Vector3d mycenter;

	/* get this object's info */
	mycenter = this->get_vertex(0);

	/* compute the bounds of argument node */
	argx[0] = c(0) - hw; 
	argx[1] = c(0) + hw; 
	argy[0] = c(1) - hw; 
	argy[1] = c(1) + hw; 

	/* check for intersection */
	return poly2d::point_in_aabb(mycenter(0),mycenter(1),
			argx[0],argy[0],argx[1],argy[1]);
}

octdata_t* octhist_2d_t::apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d)
{
	pair<histmap_t::iterator, bool> ins;
	Vector3d myc;
	double w;
	MARK_USED(c);

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
	w = (2 * hw);
	
	/* add weight to sample at current index */
	ins = this->cells.insert(
			pair<index_t, double>(this->current_index, w));
	if(ins.second == false)
	{
		/* value already exists, so just add to its weight */
		ins.first->second += w;
	}
	
	/* don't modify the data value */
	return d;
}
