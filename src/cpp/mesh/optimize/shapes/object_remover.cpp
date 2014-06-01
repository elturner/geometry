#include "object_remover.h"
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <vector>
#include <cmath>
#include <map>

/**
 * @file object_remover.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief This class is used to selectively delete data from an octree
 *
 * @section DESCRIPTION
 *
 * This class is used to remove data elements in an octree that correspond
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

Vector3d object_remover_t::get_vertex(unsigned int i) const
{
	Vector3d v;
	MARK_USED(i);

	/* just return a default value */
	return v;
}
		
bool object_remover_t::intersects(const Vector3d& c, double hw) const
{
	MARK_USED(c);
	MARK_USED(hw);

	/* for any node, this object will intersect it */
	return true;
}

octdata_t* object_remover_t::apply_to_leaf(const Vector3d& c,
                                           double hw, octdata_t* d)
{
	MARK_USED(hw);

	/* check if this leaf is an object leaf */
	if(d == NULL || !d->is_object() || d->get_count() == 0)
		return d; /* don't do anything */

	/* this leaf represents an object in the environment that was
	 * at least partially scanned, so record
	 * it and then free the data */
	this->objects[d->get_fp_room()].push_back(c);
// TODO	delete d;
//	return NULL;
	return d;
}

/*---------------------*/
/* debugging functions */
/*---------------------*/

void object_remover_t::writeobj(ostream& os) const
{
	map<int, vector<Vector3d, 
		aligned_allocator<Vector3d> > >::const_iterator it;
	size_t i;
	unsigned int r,g,b;

	/* iterate over room id's */
	for(it = this->objects.begin(); it != this->objects.end(); it++)
	{
		/* make random color for this room */
		r = 64 + (rand() % 128);
		g = 64 + (rand() % 128);
		b = 64 + (rand() % 128);

		/* print points */
		for(i = 0; i < it->second.size(); i++)
			os << "v " << it->second[i](0)
			   <<  " " << it->second[i](1)
			   <<  " " << it->second[i](2)
			   <<  " " << r
			   <<  " " << g
			   <<  " " << b << endl;
	}
}
