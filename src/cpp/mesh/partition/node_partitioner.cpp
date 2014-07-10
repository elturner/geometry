#include "node_partitioner.h"
#include "node_set.h"
#include <geometry/octree/octtopo.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/union_find.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <sstream>
#include <string>
#include <vector>
#include <map>

/**
 * @file   node_partitioner.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Partitions octree volume into separate objects
 *
 * @section DESCRIPTION
 *
 * The classes declared in this file are used to analyze the nodes of
 * an existing octree.  This set of nodes will be partitioned into
 * individual objects, as well as nodes that represent the permenant
 * building features.
 *
 * Objects in the environment will be divided based on connectivity, in
 * attempt to separate each object from the others.
 *
 * Created July 8th, 2014
 */

using namespace std;
using namespace Eigen;
using namespace octtopo;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int node_partitioner_t::partition(const octtopo_t& topo)
{
	map<octnode_t*, octneighbors_t>::const_iterator it;
	map<octnode_t*, size_t> indices;
	vector<octnode_t*> rev_indices;
	union_find_t us;
	vector<octnode_t*> neighs;
	vector<vector<size_t> > final_unions;
	size_t fi, i, j, num_nodes, myind, num_unions;
	int ret;

	/* index the nodes of the octree referenced in the topology */
	for(it = topo.begin(); it != topo.end(); it++)
		if((indices.insert(pair<octnode_t*, size_t>(it->first,
						indices.size()))).second)
			rev_indices.push_back(it->first); /* newly added */

	/* populate the union-find structure with edges */
	us.init(indices.size());
	for(it = topo.begin(); it != topo.end(); it++)
	{
		/* iterate over faces of current node,
		 * getting all neighbors */
		neighs.clear();
		for(fi = 0; fi < NUM_FACES_PER_CUBE; fi++)
			it->second.get(all_cube_faces[fi], neighs);
	
		/* iterate over neighbors, adding each edge to the
		 * union-find structure */
		myind = indices[it->first];
		num_nodes = neighs.size();
		for(i = 0; i < num_nodes; i++)
		{
			/* only counts as an edge if both are labeled
			 * the same */
			if(it->first->data->is_interior() 
					!= neighs[i]->data->is_interior())
				continue;
			if(it->first->data->get_fp_room()
					!= neighs[i]->data->get_fp_room())
				continue;

			/* add this edge to graph */
			ret = us.add_edge(myind, indices[neighs[i]]);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
		}
	}

	/* populate list of node sets */
	us.get_unions(final_unions);
	num_unions = final_unions.size();
	this->partitions.resize(num_unions);
	for(i = 0; i < num_unions; i++)
	{
		/* copy the nodes from this union to the current
		 * partition */
		this->partitions[i].clear();
		num_nodes = final_unions[i].size();
		for(j = 0; j < num_nodes; j++)
			this->partitions[i].add(
				rev_indices[final_unions[i][j]]
					);
	}

	/* success */
	return 0;
}
	
int node_partitioner_t::writeobjs(const std::string& prefix) const
{
	progress_bar_t progbar;
	stringstream ss;
	tictoc_t clk;
	size_t i, n;
	int ret;

	/* iterate over the objects to write */
	tic(clk);
	progbar.set_name("Writing objects");
	n = this->partitions.size();
	for(i = 0; i < n; i++)
	{
		/* show status to user */
		progbar.update(i, n);

		/* prepare file for this object */
		ss.str("");
		ss << prefix << "_" << i << ".obj";
		ret = this->partitions[i].writeobj(ss.str());
		if(ret)
			return PROPEGATE_ERROR(-(i+1), ret);
	}

	/* success */
	progbar.clear();
	toc(clk, "Writing objects");
	return 0;
}
