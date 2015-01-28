#include "point_carver.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <fstream>
#include <vector>
#include <map>

/**
 * @file point_carver.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the functions for the point_carver_t class,
 * which is used to model the probability distribution of the occupancy
 * volume intersected by a single scanned point with noisy positioning.
 */

using namespace std;
using namespace Eigen;

/* function implementations */
		
point_carver_t::point_carver_t()
{
	/* initialize a cleared object */
	this->clear();
}

void point_carver_t::clear()
{
	/* empty the structure */
	this->volume_map.clear();
	this->num_samples = 0;
}

int point_carver_t::add_sample(const Vector3d& sensor_pos,
                               const Vector3d& scan_pos,
                               octree_t& tree)
{
	vector<octnode_t*> leafs;
	vector<octnode_t*>::iterator it;
	pair<map<octnode_t*, unsigned int>::iterator, bool> mit;
	int ret;

	/* carve this line segment into the tree, find all intersected
	 * leaf nodes.  This process may create some leaf nodes in the
	 * tree. */
	ret = tree.raycarve(leafs, sensor_pos, scan_pos);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* take all intersected leafs, and insert into volume map */
	for(it = leafs.begin(); it != leafs.end(); it++)
	{
		/* attempt to insert leaf into map */
		mit = this->volume_map.insert(
			make_pair<octnode_t*, unsigned int>(*it, 1));

		/* check if node already in map */
		if(!mit.second)
		{
			/* already in map, so update count */
			mit.first->second++;
		}
	}

	/* update total number of contributing samples */
	this->num_samples++;

	/* success */
	return 0;
}
		
int point_carver_t::update_tree() const
{
	map<octnode_t*, unsigned int>::const_iterator it;
	double prob;

	/* for each stored node, update its data object, or if there
	 * is no data object, add one */
	for(it=this->volume_map.begin(); it!=this->volume_map.end(); it++)
	{
		/* get the probability observation for this node,
		 * which is the ratio of samples that intersected it
		 * to the total number of samples */
		prob = ((double) it->second) / this->num_samples;
		// LEFT UNDONE planarity and corner coefficient estimates

		/* check if this node already has data */
		if(it->first->data != NULL)
		{
			/* add observation to this data object */
			it->first->data->add_sample(prob);
		}
		else
		{
			/* this is the first data value for this
			 * node, so make a new one */
			it->first->data = new octdata_t(prob);
		}
	}

	// LEFT UNDONE tree simplification?
	
	/* success */
	return 0;
}
		
int point_carver_t::write_to_file(const string& filename) const
{
	map<octnode_t*, unsigned int>::const_iterator it;
	ofstream outfile;
	
	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write contents to file */
	for(it=this->volume_map.begin(); it!=this->volume_map.end(); it++)
		outfile << it->first->center(0) 
		        << " " << it->first->center(1)
		        << " " << it->first->center(2)
		        << " " << it->first->halfwidth
		        << " " << it->second << endl;

	/* clean up */
	outfile.close();
	return 0;
}
