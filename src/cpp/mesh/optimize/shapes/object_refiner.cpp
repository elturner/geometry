#include "object_refiner.h"
#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <geometry/carve/random_carver.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
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
 *
 * This file requires the Eigen framework.
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
	int ret;

	/* store parameters */
	this->object_depth_increase = inc_depth;

	/* open input files */
	ret = this->chunk_map.init(chunklistfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	ret = this->wedges.open(wedgefile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	ret = this->carvemaps.open(cmfile);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* success */
	return 0;
}
		
int object_refiner_t::refine(octree_t& tree)
{
	chunk::chunk_reader_t chunk;
	set<string>::iterator it;
	set<chunk::point_index_t> inds;
	Vector3d chunkcenter;
	octnode_t* chunknode;
	unsigned int chunkdepth;
	progress_bar_t progbar;
	tictoc_t clk;
	size_t i, n;
	int ret;

	/* first, populate the list of object nodes detected in
	 * this tree by searching through the tree. */
	tic(clk);
	this->clear(); /* discard previous data */
	tree.find(*this); /* populates nodes vector */
	toc(clk, "Finding object chunks");

	/* refine this tree's max depth */
	tree.increase_depth(this->object_depth_increase);

	/* for each chunk that contains any of the object nodes,
	 * recarve this chunk at a finer resolution */
	tic(clk);
	progbar.set_name("  Refining octree");
	n = this->object_chunks.size();
	i = 0;
	for(it = this->object_chunks.begin();
			it != this->object_chunks.end(); it++)
	{
		/* inform user of progress */
		progbar.update(i++, n);

		/* open this chunk file for recarving */
		ret = chunk.open(*it);
		if(ret)
		{
			/* unable to open chunk file */
			progbar.clear();
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[object_refiner_t::refine]\tError " << ret
			     << ": Unable to open chunk file for refining: "
			     << *it << endl;
			return ret;
		}

		/* read all indices from this file */
		inds.clear();
		chunk.get_all(inds);

		/* prepare the node in the tree that holds this chunk */
		chunkcenter << chunk.center_x(),
		               chunk.center_y(),
		               chunk.center_z();
		chunknode = tree.expand(chunkcenter, chunk.halfwidth(),
		                              chunkdepth);
		if(chunknode == NULL)
		{
			/* tree not initialized */
			progbar.clear();
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[object_refiner_t::refine]\tError " << ret
			     << ": Unable to expand chunk for refining: "
			     << chunkcenter.transpose() << endl;
			return ret;
		}
		chunk.close();
		
		/* clear preexisting data from this node */
		chunknode->clear();

		/* replace this chunk in the tree */
		ret = random_carver_t::carve_node(chunknode, inds,
				this->carvemaps, this->wedges, 
				chunkdepth, true);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[object_refiner_t::refine]\tError " << ret
			     << ": Unable to carve chunk node for refining:"
			     << " " << chunkcenter.transpose() << endl;
			return ret;
		}
	}

	/* clean up */
	progbar.clear();
	toc(clk, "Refining octree");

	/* success */
	return 0;
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
	MARK_USED(hw);

	/* check if this leaf is an object leaf */
	if(d != NULL && d->is_object())
	{
		/* this leaf represents an object in the environment that
		 * was at least partially scanned, so record its chunk for 
		 * processing. */
		this->chunk_map.retrieve(c, this->object_chunks);
	}
	
	/* don't modify the data...for now */
	return d;
}
