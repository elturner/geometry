#include "bounding_box.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <util/error_codes.h>
#include <Eigen/Dense>

/**
 * @file   bounding_box.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Computes the bounding box of an octree
 *
 * @section DESCRIPTION
 *
 * The bounding_box_t class extends shape_t, and iterates through
 * the nodes of a given octree to compute the bounding box of the
 * data stored in the octree.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void bounding_box_t::init(octree_t& tree)
{
	/* clear any existing data */
	this->clear();

	/* find all data in tree */
	tree.find(*this);
}
		
octdata_t* bounding_box_t::apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d)
{
	size_t i;
	MARK_USED(hw);

	/* ignore empty or null data objects */
	if(d == NULL)
		return d;
	if(d->get_count() == 0)
		return d;
	if(d->get_total_weight() <= 0)
		return d;

	/* update bounding box */
	if(this->is_valid())
	{
		/* update each dimension */
		for(i = 0; i < 3; i++)
		{
			if(this->min_corner(i) > c(i))
				this->min_corner(i) = c(i);
			if(this->max_corner(i) < c(i))
				this->max_corner(i) = c(i);
		}
	}
	else
	{
		/* this is the first datapoint, so just
		 * set the bounding box to this center position */
		this->min_corner = c;
		this->max_corner = c;
	}

	/* return the unmodified data */
	return d;
}
