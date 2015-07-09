#include "door_finder.h"
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <geometry/shapes/linesegment.h>
#include <geometry/shapes/shapewrapper.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <set>

/**
 * @file   door_finder.cpp
 * @author Eric Turner <elturner@indoorreality.com>
 * @brief  Class used to find doors in octree models
 *
 * @section DESCRIPTION
 *
 * Will take an octree and a path, and will estimate the positions
 * of doors in the model.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int door_finder_t::analyze(octree_t& tree, const system_path_t& path)
{
	set<int> rooms_seen;
	pose_t* prev_p, *curr_p;
	octtopo::octtopo_t top;
	linesegment_t line;
	shape_wrapper_t linewrap;
	tictoc_t clk;
	size_t i, n, j, m;
	int ret;

	/* determine the topology of the octree */
	tic(clk);
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	toc(clk, "Initializing topology");

	/* iterate over the poses of the path, and find which segments
	 * of the path cross between rooms */
	n = path.num_poses();
	for(i = 1; i < n; i++)
	{
		/* get the segment between this and the previous pose */
		prev_p = path.get_pose(i-1);
		curr_p = path.get_pose(i  );
		line.init(prev_p->T, curr_p->T);

		/* find all nodes in the that intersect this segment */
		linewrap.find_in_tree(line, tree);

		/* now that we have a set of leaves that were intersected,
		 * check if there are multiple rooms represented */
		m = linewrap.data.size();
		rooms_seen.clear();
		for(j = 0; j < m; j++)
			rooms_seen.insert(linewrap.data[j].fp_room);
		rooms_seen.erase(-1); /* don't count out-of-bounds */

		/* do we see multiple rooms? */
		if(rooms_seen.size() > 1)
		{
			// TODO
		}

		// TODO LEFT OFF HERE
	}

	/* success */
	return 0;
}
