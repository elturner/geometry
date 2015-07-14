#include "door_finder.h"
#include "../structs/door.h"
#include <geometry/system_path.h>
#include <geometry/quadtree/quadtree.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/shapes/point_2d.h>
#include <geometry/shapes/shape_wrapper.h>
#include <geometry/hist/hia_analyzer.h>
#include <geometry/hist/hia_cell_index.h>
#include <geometry/hist/hia_cell_info.h>
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

int door_finder_t::analyze(octree_t& tree, 
		const hia_analyzer_t& hia, const system_path_t& path)
{
	door_t curr_door;
	pose_t* prev_p, *curr_p;
	octnode_t* root, *prev_node, *curr_node;
	tictoc_t clk;
	size_t i, n;
	int ret, prev_room, curr_room;

	/* get root information for tree */
	root = tree.get_root();
	if(root == NULL)
		return -1;

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
		ret = this->find_door_intersection(curr_door.center, tree,
					prev_p->T, curr_p->T);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);

		/* record this position as a door */
		this->doors.push_back(curr_door);
	}
	toc(clk, "Finding door locations");

	/* Remove duplicated door positions
	 *
	 * Duplicates can happen if the operator walks through
	 * the same door multiple times.
	 */
	tic(clk);
	ret = this->remove_duplicates(tree, 
			hia.get_bounds().get_min(2), 
			hia.get_bounds().get_max(2));
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Removing duplicate doors");

	/* estimate the door geometry */
	tic(clk);
	n = this->doors.size();
	for(i = 0; i < n; i++)
	{
		/* estimate the shape of current door */
		ret = this->estimate_door_geom(hia, i);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);
	}
	toc(clk, "Estimating door geometry");

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
	n = this->doors.size();
	for(i = 0; i < n; i++)
		this->doors[i].writeobj(outfile);

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
				double floor_height, double ceil_height)
{
	vector<door_t> keepers;
	vector<quaddata_t*> neighs;
	quadtree_t quad;
	octnode_t* octroot;
	Vector2d quadcenter, doorpos2d, some_norm;
	double door_height;
	size_t door_index, num_doors;
	int ret;

	/* iterate over the levels of the model */
	num_doors = this->doors.size();
	keepers.clear();
	some_norm << 1,0; /* arbitrary values */

	/* initialize the quadtree to overlap with the octree */
	octroot = octree.get_root();
	quadcenter << octroot->center(0), octroot->center(1);
	quad.set(octree.get_resolution(), 
			quadcenter, octroot->halfwidth);
	
	/* iterate over the doors found */
	for(door_index = 0; door_index < num_doors; door_index++)
	{
		/* get the elevation of this door position */
		door_height = this->doors[door_index].center(2);
		doorpos2d << this->doors[door_index].center(0),
			  this->doors[door_index].center(1);

		if(floor_height > door_height || ceil_height < door_height)
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
		keepers.push_back(this->doors[door_index]);
	}

	/* repopulate the door positions with only the keepers */
	this->doors.clear();
	this->doors.insert(this->doors.begin(),
			keepers.begin(), keepers.end());
	
	/* success */
	return 0;
}
		
int door_finder_t::estimate_door_geom(const hia_analyzer_t& hia, 
					size_t door_ind)
{
	vector<double> angles, sums;
	Vector2d center2d, dir, normal, start_pos, end_pos, curr_pos;
	hia_cell_index_t centerind;
	hia_analyzer_t::cellmap_t::const_iterator it;
	size_t i, n, i_min, j, m;
	double res, curr_val, avg_val;

	/* get some basics about the model */
	res = hia.get_resolution();

	/* get info about the door position */
	center2d << this->doors[door_ind].center(0),
		    this->doors[door_ind].center(1);
	centerind = hia.get_index_of(center2d);
	it = hia.get_info_for(centerind);
	if(it == hia.end())
		return -1; /* invalid index */

	/* we want to find a line segment that goes through the center 
	 * point that has the smallest sum of open height.  This segment
	 * will denote the orientation of the door. */

	/* prepare the list of possible angles to test */
	n = 1 + ceil(2*M_PI / this->angle_stepsize); /* fencepost */
	m = 1 + ceil(this->door_max_width / res); /* fencepost */
	angles.resize(n);
	sums.resize(n);
	i_min = 0;
	for(i = 0; i < n; i++)
	{
		/* initalize value */
		angles[i] = i * this->angle_stepsize;
		sums[i]   = 0;

		/* for this angle, determine the start/end points
		 * of the line segment representing the door */
		dir(0)    = cos(angles[i]);
		dir(1)    = sin(angles[i]);
		normal << dir(1), -dir(0); /* 90 degree rotation */
		start_pos = center2d - (dir * this->door_max_width/2);
		end_pos   = center2d + (dir * this->door_max_width/2);

		/* iterate along this line, testing what the values are */
		for(j = 0; j < m; j++)
		{
			/* get the j'th position along the line */
			curr_pos = start_pos + (res * dir);

			/* get value at this position, and add to sum */
			curr_val = hia.get_open_height_at(curr_pos);
			if(curr_val >= 0)
				sums[i] += curr_val;

			/* also add one gridcell in each normal
			 * direction, to make sure we don't miss something
			 * adjacent */
			curr_val = hia.get_open_height_at(curr_pos 
						+ (res * normal));
			if(curr_val >= 0)
				sums[i] += curr_val;
			curr_val = hia.get_open_height_at(curr_pos 
						- (res * normal));
			if(curr_val >= 0)
				sums[i] += curr_val;
		}

		/* check if the integral over this line is smaller
		 * than the current min */
		if(sums[i] <= sums[i_min])
		{
			/* update min */
			i_min = i;

			/* since this is the best so far, save the
			 * end points */
			this->doors[door_ind].endpoints[0] = start_pos;
			this->doors[door_ind].endpoints[1] = end_pos;
		}
	}
	
	/* Now that we have the best angle, we can estimate the
	 * height of the door by taking the average value from the
	 * sum at the min angle. */
	avg_val = sums[i_min] / (3*m);
	this->doors[door_ind].z_min = it->second.min_z;
	this->doors[door_ind].z_max = it->second.min_z 
		+ max(min(avg_val, this->door_max_height),
					this->door_min_height);

	/* now that we know the best angle, we need to determine the
	 * width of the door.  Note also that the 'center' position
	 * currently chosen for the door may not actually be the true
	 * center */
	
	// TODO LEFT OFF HERE

	/* success */
	return 0;
}
