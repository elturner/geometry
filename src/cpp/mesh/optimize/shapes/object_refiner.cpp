#include "object_refiner.h"
#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <string>
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
/* processing functions */
/*----------------------*/

int object_refiner_t::init(unsigned int inc_depth,
                           const string& chunklistfile,
                           const string& wedgefile,
                           const string& cmfile)
{
	chunk::chunk_reader_t chunk;
	string chunkfile;
	size_t i, n;
	int ret;

	/* store parameters */
	this->object_depth_increase = inc_depth;

	/* open input files */
	ret = this->chunklist.open(chunklistfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	ret = this->wedges.open(wedgefile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	ret = this->carvemaps.open(cmfile);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* iterate over chunks */
	n = this->chunklist.num_chunks();
	for(i = 0; i < n; i++)
	{
		/* verify that the chunk file is valid */
		ret = this->chunklist.next(chunkfile);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);

		/* open file to verify */
		ret = chunk.open(chunkfile);
		if(ret)
			return PROPEGATE_ERROR(-5, ret);
		// TODO cache position of chunk
		chunk.close();
	}

	/* success */
	return 0;
}
		
int object_refiner_t::refine(octree_t& tree)
{
	/* first, populate the list of object nodes detected in
	 * this tree by searching through the tree. */
	this->nodes.clear(); /* discard previous data */
	tree.find(*this); /* populates nodes vector */

	/* refine this tree's max depth */
	tree.increase_depth(this->object_depth_increase);

	/* now that we have a list of nodes to refine, we need
	 * to identify the chunks that contain these nodes */
	// TODO

	/* for each chunk that contains any of the object nodes,
	 * recarve this chunk at a finer resolution */
	// TODO

	return -1; /* this function is incomplete */
}

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
