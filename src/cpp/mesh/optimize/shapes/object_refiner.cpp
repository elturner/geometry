#include "object_refiner.h"
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <map>

/**
 * @file object_refiner.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief This class is used to selectively refine data in an octree
 *
 * @section DESCRIPTION
 *
 * This class is used to refine data elements in an octree that correspond
 * to objects.  This requires the octree to have already imported floorplan
 * information.  The motivation for removing these data elements is so
 * those locations can be recarved at a finer resolution.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

/*----------------------*/
/* overloaded functions */
/*----------------------*/

Vector3d object_refiner_t::get_vertex(unsigned int i) const
{
	Vector3d v;
	MARK_USED(i);

	/* just return a default value */
	return v;
}
		
bool object_refiner_t::intersects(const Vector3d& c, double hw) const
{
	MARK_USED(c);
	MARK_USED(hw);

	/* for any node, this object will intersect it */
	return true;
}

octdata_t* object_refiner_t::apply_to_leaf(const Vector3d& c,
                                           double hw, octdata_t* d)
{
	/* check if this leaf is an object leaf */
	if(d != NULL && d->is_object())
	{
		/* this leaf represents an object in the environment that
		 * was at least partially scanned, so record it for 
		 * processing. */
		this->nodes.push_back(
			node_location_t(d->get_fp_room(), c, hw));
	}
	
	/* don't modify the data...for now */
	return d;
}

/*---------------------*/
/* debugging functions */
/*---------------------*/

void object_refiner_t::writeobj(ostream& os) const
{
	vector<node_location_t>::const_iterator it;
	Vector3d c;

	/* iterate over room id's */
	for(it = this->nodes.begin(); it != this->nodes.end(); it++)
	{
		/* print points */
		c = it->get_center();
		os << "v " << c(0)
		   <<  " " << c(1)
		   <<  " " << c(2) << endl;
	}
}
