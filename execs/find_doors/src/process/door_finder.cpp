#include "door_finder.h"
#include <io/levels/building_levels_io.h>
#include <geometry/system_path.h>
#include <geometry/quadtree/quadtree.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <geometry/shapes/point_2d.h>
#include <geometry/shapes/shape_wrapper.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
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
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int door_finder_t::analyze(octree_t& tree, const system_path_t& path,
				const std::string& levelsfile)
{
	Vector3d doorpos;
	pose_t* prev_p, *curr_p;
	octtopo::octtopo_t top;
	octnode_t* root, *prev_node, *curr_node;
	tictoc_t clk;
	size_t i, n;
	int ret, prev_room, curr_room;

	/* get root information for tree */
	tic(clk);
	root = tree.get_root();
	if(root == NULL)
		return -1;
	
	/* determine the topology of the octree */
	ret = top.init(tree);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);
	toc(clk, "Initializing topology");

	/* iterate over the poses of the path, and find which segments
	 * of the path cross between rooms */
	tic(clk);
	n = path.num_poses();
	for(i = 1; i < n; i++)
	{
		/* get the segment between this and the previous pose */
		prev_p = path.get_pose(i-1);
		curr_p = path.get_pose(i  );

		/* find the nodes that intersect these poses */
		prev_node = root->retrieve(prev_p->T);
		curr_node = root->retrieve(curr_p->T);

		/* check that both leaf nodes have valid rooms defined */
		if(prev_node == NULL || curr_node == NULL
				|| prev_node->data == NULL
				|| curr_node->data == NULL)
			continue; /* don't bother with these */

		/* get the room info */
		prev_room = prev_node->data->get_fp_room();
		curr_room = curr_node->data->get_fp_room();
		if(prev_room == curr_room)
			continue; /* not an interesting section of path */

		/* if got here, it means that this section of path moved
		 * from one room to another, so it is likely to be a
		 * location of a door */

		/* refine our estimate of the position of the door */
		ret = this->find_door_intersection(doorpos, tree,
					prev_p->T, curr_p->T);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);

		/* record this position as a door */
		this->door_positions.push_back(doorpos);
	}
	toc(clk, "Finding door locations");

	/* Remove duplicated door positions
	 *
	 * Duplicates can happen if the operator walks through
	 * the same door multiple times.
	 */
	tic(clk);
	ret = this->remove_duplicates(tree, levelsfile);
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Removing duplicate doors");

	/* success */
	return 0;
}
		
int door_finder_t::writetxt(const std::string& txtfile) const
{
	ofstream outfile;
	tictoc_t clk;
	size_t i, n;

	/* attempt to open file */
	tic(clk);
	outfile.open(txtfile.c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[door_finder_t::writetxt]\tUnable to open "
		     << "file for writing: " << txtfile << endl;
		return -1;
	}	

	/* write each door position to the file */
	n = this->door_positions.size();
	for(i = 0; i < n; i++)
		outfile << this->door_positions[i].transpose() << endl;

	/* clean up */
	outfile.close();
	toc(clk, "Writing txt file");
	return 0;
}
		
int door_finder_t::find_door_intersection(Eigen::Vector3d& doorpos,
				octree_t& tree,
				const Eigen::Vector3d& A,
				const Eigen::Vector3d& B) const
{
	Vector2d line_start, line_center, line_end, dir, binpoint;
	shape_wrapper_t finder;
	point_2d_t point;
	vector<double> bins;
	double stepsize, z_mean;
	size_t i, n, j, m, i_min;

	/* A and B are the positions of the adjacent poses that
	 * cross the door threshold.  These form a line when projected
	 * onto 2D */
	line_start << A(0), A(1);
	line_end   << B(0), B(1);
	dir = line_end - line_start;
	dir.normalize();
	
	/* Next, extend the line AB to make sure it covers the exact
	 * position of the door threshold
	 *
	 * The door could swing either inwards or outwards, so we know
	 * the area must be clear by at least door_min_width on each
	 * side. */
	z_mean      = ( A(2) + B(2) ) / 2;
	line_center = (line_start + line_end) / 2;
	line_start  = line_center - (this->door_min_width * dir);
	line_end    = line_center + (this->door_min_width * dir);

	/* iterate over the line, checking the values at each bin,
	 * and determine the amount of interior volume intersected */
	stepsize = tree.get_resolution() / 2; /* small enough step */
	n = ceil( dir.norm() / stepsize ) + 1; /* fencepost */
	bins.resize(n);
	i_min = 0; /* to start, assume the first bin is smallest */
	for(i = 0; i < n; i++)
	{
		/* get the position of this bin */
		binpoint = line_start + i*stepsize*dir;
		point.init(binpoint, 
			z_mean - this->door_max_height/2,
			z_mean + this->door_max_height/2);

		/* find all nodes that intersect this point */
		finder.find_in_tree(point, tree);

		/* now we have all leaf nodes in the tree
		 * that intersect this point.  Count up the
		 * interior volume. */
		m = finder.data.size();
		bins[i] = 0;
		for(j = 0; j < m; j++)
		{
			/* check if values exist that this point */
			if(finder.data[j] == NULL)
				continue;

			/* update this bin value 
			 *
			 * The bin values are in units of 
			 * 		<probability> * <height>
			 *
			 * Since they are the sum of node probabilities
			 * across the height of the point in question.
			 *
			 * This determines if a vertically-oriented
			 * solid object is at this point, since it 
			 * averages the solid-ness across the heights
			 * at this point. */
			bins[i] += finder.data[j]->get_probability() 
					* 2 * finder.halfwidths[j];
		}
	
		/* check if this is the bin with the smallest value.  
		 * The smallest bin value indicates that it has 
		 * the "most solid" volume */
		if(bins[i_min] > bins[i])
		{
			/* record that this bin is lowest so far */
			i_min = i;

			/* save position of bin in 3D */
			doorpos(0) = binpoint(0);
			doorpos(1) = binpoint(1);
			doorpos(2) = z_mean;
		}
	}
			
	/* success */
	return 0;
}
		
int door_finder_t::remove_duplicates(const octree_t& octree,
		const std::string& levelsfile)
{
	vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> >
				keepers;
	vector<quaddata_t*> neighs;
	building_levels::file_t levels;
	building_levels::level_t curr_level;
	quadtree_t quad;
	octnode_t* octroot;
	Vector2d quadcenter, doorpos2d, some_norm;
	double door_height;
	size_t level_index, num_levels, door_index, num_doors;
	int ret;

	/* if provided, read in the levels file */
	if(!(levelsfile.empty()))
	{
		/* attempt to read */
		ret = levels.parse(levelsfile);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* record number of levels */
		num_levels = levels.num_levels();
	}
	else
		num_levels = 1; /* assume single-level */

	/* iterate over the levels of the model */
	num_doors = this->door_positions.size();
	keepers.clear();
	some_norm << 1,0; /* arbitrary values */
	for(level_index = 0; level_index < num_levels; level_index++)
	{
		/* get the current level (if it exists) */
		if(!(levelsfile.empty()))
			curr_level = levels.get_level(level_index);

		/* initialize the quadtree to overlap with the octree */
		octroot = octree.get_root();
		quadcenter << octroot->center(0), octroot->center(1);
		quad.set(octree.get_resolution(), 
				quadcenter, octroot->halfwidth);
	
		/* iterate over the doors found */
		for(door_index = 0; door_index < num_doors; door_index++)
		{
			/* get the elevation of this door position */
			door_height = this->door_positions[door_index](2);
			doorpos2d << this->door_positions[door_index](0),
				  this->door_positions[door_index](1);

			/* check if this door is in this level */
			if(num_levels > 1
				&& ( curr_level.floor_height > door_height
				|| curr_level.ceiling_height < door_height))
				continue; /* not in level */

			/* check to see if a door is already near
			 * this position */
			neighs.clear();
			ret = quad.neighbors_in_range(doorpos2d, 
					this->door_max_width,
					neighs);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);
			if(!(neighs.empty()))	
				continue; /* door is already here */

			/* add this door to the map, for future checks */
			quad.insert(doorpos2d, some_norm);

			/* we want to keep this door */
			keepers.push_back(this->door_positions[door_index]);
		}
	}

	/* repopulate the door positions with only the keepers */
	this->door_positions.clear();
	this->door_positions.insert(this->door_positions.begin(),
			keepers.begin(), keepers.end());
	
	/* success */
	return 0;
}
