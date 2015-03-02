#include "process.h"
#include "oct2dq_run_settings.h"
#include <io/data/fss/fss_io.h>
#include <io/levels/building_levels_io.h>
#include <geometry/shapes/shape_wrapper.h>
#include <geometry/shapes/linesegment.h>
#include <geometry/shapes/linesegment_2d.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octtopo.h>
#include <geometry/quadtree/quadtree.h>
#include <geometry/quadtree/quaddata.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <mesh/refine/octree_padder.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <mesh/wall_sampling/wall_region_info.h>
#include <mesh/wall_sampling/horizontal_region_info.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/histogram.h>
#include <util/tictoc.h>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
#include <cmath>
#include <Eigen/Dense>

/**
 * @file   process.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This class contains all process data for the oct2dq program
 *
 * @section DESCRIPTION
 *
 * This class represents the processing pipeline for the oct2dq program.
 * It contains all necessary data products and has functions that
 * process these data products appropriately.
 */

using namespace std;
using namespace octtopo;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int process_t::init(oct2dq_run_settings_t& args)
{
	octtopo_t top;
	tictoc_t clk;
	int ret;

	/* import the octree */
	tic(clk);
	ret = this->tree.parse(args.octfile);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\t"
		     << "Unable to read in octree file: "
		     << args.octfile << endl;
		return PROPEGATE_ERROR(-3, ret);
	}
	octree_padder::pad(this->tree); /* just in case */

	/* check if the dq resolution needs to be modified based
	 * on the resolution of this octree */
	if(args.dq_resolution <= 0)
		args.dq_resolution = this->tree.get_resolution();
	toc(clk, "Importing octree");

	/* get octree topology */
	tic(clk);
	ret = top.init(tree);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to compute octree "
		     << "topology" << endl;
		return PROPEGATE_ERROR(-4, ret);
	}
	toc(clk, "Generating topology");

	/* use topology to form faces */
	ret = this->boundary.populate(top);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to compute boundary "
		     << "faces" << endl;
		return PROPEGATE_ERROR(-5, ret);
	}

	/* use faces to form regions */
	tic(clk);
	ret = this->region_graph.populate(this->boundary);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to populate regions "
		     << "from faces" << endl;
		return PROPEGATE_ERROR(-6, ret);
	}
	toc(clk, "Populating regions");

	/* coalesce initial regions into larger regions */
	tic(clk);
	this->region_graph.init(args.coalesce_planethresh,
			args.coalesce_distthresh, 
			args.use_isosurface_pos);
	ret = this->region_graph.coalesce_regions();
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to coalesce regions"
		     << endl;
		return PROPEGATE_ERROR(-7, ret);
	}
	toc(clk, "Coalescing regions");

	/* success */
	return 0;
}
		
int process_t::identify_surfaces(const oct2dq_run_settings_t& args)
{
	map<node_face_t, size_t> wall_regions;
	map<node_face_t, size_t> floor_regions;
	map<node_face_t, size_t> ceiling_regions;
	map<node_face_t, size_t>::iterator wit, w1it, w2it;
	map<node_face_t, size_t>::iterator floor_it, ceil_it;
	horizontal_region_info_t hori_info;
	wall_region_info_t wall_info;
	regionmap_t::const_iterator it;
	faceset_t::const_iterator fit, n1it, n2it;
	progress_bar_t progbar;
	tictoc_t clk;
	double height;
	size_t i;
	int best_floor_ind, best_ceil_ind;

	/* initialize */
	tic(clk);
	progbar.set_name("Finding walls");
	i = 0;

	/* iterate over regions, computing strength for wall samples */
	for(it = this->region_graph.begin();
			it != this->region_graph.end(); it++)
	{
		/* show status to user */
		progbar.update(i++, this->region_graph.size());

		/* get strength for this region.  stronger means more
		 * wall-like */
		wall_info.strength 
			= this->compute_region_strength(it, args);
	
		/* only proceed if strength is good enough */
		if(wall_info.strength <= 0)
			continue;

		/* initialize stored values in wall_info struct */
		wall_info.init(wall_info.strength, 
				it->second.get_region());

		/* compare bounding box to wall height threshold,
		 * just to make sure we want to use this region */
		height = wall_info.b_max - wall_info.b_min;
		if(height < args.wallheightthresh)
			continue; /* don't use region */

		/* add this wall region to our list */
		this->walls.push_back(wall_info);

		/* we want to use this region, so keep it */
		wall_regions.insert(pair<node_face_t, size_t>(
					it->first, this->walls.size()-1));
	}

	/* iterate over the regions again.  check if a reject
	 * region has two neighbors that:
	 * 	- are both wall regions
	 * 	- AND are facing opposing directions
	 */
	progbar.set_name("Finding small walls");
	i = 0;
	for(it = this->region_graph.begin();
			it != this->region_graph.end(); it++)
	{
		/* update the progress bar */
		progbar.update(i++, this->region_graph.size());

		/* check if this is already a wall region */
		if(wall_regions.count(it->first) > 0)
			continue; /* don't need to do anything */

		/* check that this region still satisfies the verticality
		 * threshold to be a wall region. */
		if(abs(it->second.get_region().get_plane().normal(2))
				>= args.verticalitythresh)
			continue; /* not vertically aligned */

		/* compute strength of this region, such as it is */
		wall_info.strength = process_t::compute_region_strength(
					it, args) + 1;

		/* iterate over neighboring regions to this region
		 *
		 * We want to compare every pair of neighboring regions,
		 * so this requires a double-iterator (how fun for us) */
		for(n1it = it->second.begin_neighs();
				n1it != it->second.end_neighs(); n1it++)
		{
			/* we only care about this region if it 
			 * represents a wall */
			w1it = wall_regions.find(*n1it);
			if(w1it == wall_regions.end())
				continue; /* ignore it */

			/* iterate over the remainder of the neighbors */
			n2it = n1it;
			n2it++;
			for( ; n2it != it->second.end_neighs(); n2it++)
			{
				/* check that this neighbor is also a 
				 * wall region */
				w2it = wall_regions.find(*n2it);
				if(w2it == wall_regions.end())
					continue; /* ignore it */

				/* both of these neighbors are wall
				 * regions.  We should add this region
				 * if the two neighbors face opposite
				 * directions, which indicates
				 * that it is a very small wall that
				 * joins two other walls (e.g. a doorway
				 * frame). */
				if(this->walls[w1it->second]
						.vertical.normal.dot(
					this->walls[w2it->second]
						.vertical.normal) > 0)
					continue; /* not opposing */
			
				/* the neighbors have opposing normals,
				 * so we should add this region */
				wall_info.init(wall_info.strength,
						it->second.get_region());
				this->walls.push_back(wall_info);
				wall_regions.insert(pair<node_face_t, 
						size_t>(it->first, 
						this->walls.size()-1));
			}
		}
	}

	/* now that we have the set of regions that are estimated to
	 * be walls, we want to get better estimates for their vertical
	 * extent.
	 *
	 * To do this, we look for nearly-horizontal neighboring regions
	 * to each wall, and choose the largest ones as the neighboring
	 * floor and ceiling, which define the min and max elevation of
	 * the wall.
	 *
	 * We also want to keep track of these horizontal regions for
	 * the purposes of determine multi-level splits
	 */
	progbar.set_name("Finding floors/ceils");
	i = 0;
	for(it = this->region_graph.begin();
			it != this->region_graph.end(); it++)
	{
		/* update progress bar */
		progbar.update(i++, this->region_graph.size());

		/* check if this is already a wall region */
		if(wall_regions.count(it->first) > 0)
			continue; /* don't need to do anything */

		/* get info about this region, to see
		 * if it would be a good fit for horizontal
		 * regions. */
		if(!(hori_info.init(it->second.get_region(), 
				args.verticalitythresh,
				args.floorceilsurfareathresh)))
			continue; /* not a good fit */

		/* insert this info into appropriate structures,
		 * based on whether it is a floor or ceiling. */
		if(hori_info.upnormal)
		{
			/* floor */
			this->floors.push_back(hori_info);
			floor_regions.insert(pair<node_face_t, size_t>
				(it->first, this->floors.size()-1));
		}
		else
		{
			/* ceiling */
			this->ceilings.push_back(hori_info);
			ceiling_regions.insert(pair<node_face_t, size_t>
				(it->first, this->ceilings.size()-1));
		}
	}

	/* use the floor and ceiling positions to adjust the neighboring
	 * wall heights */
	progbar.set_name("Refining wall heights");
	i = 0;
	for(wit = wall_regions.begin(); wit != wall_regions.end(); wit++)
	{
		/* update progress bar */
		progbar.update(i++, wall_regions.size());
	
		/* get the original planar regions for each wall */
		it = this->region_graph.lookup_face(wit->first);
		if(it == this->region_graph.end())
		{
			progbar.clear();
			cerr << "[identify_surfaces]\tError! Inconsistent "
			     << "wall region iterators (this means the "
			     << "code is probably broken)" << endl;
			return -1;
		}

		/* Go through the neighboring regions of this planar 
		 * region, and check if any of the neighbors are floors
		 * or ceilings.  If so, then update the heights of the
		 * wall based on the largest neighboring floor and ceiling.
		 */
		best_floor_ind = best_ceil_ind = -1;
		for(n1it = it->second.begin_neighs();
				n1it != it->second.end_neighs(); n1it++)
		{
			/* is this neighboring region (represented by
			 * the seed face) a floor? */
			floor_it = floor_regions.find(*n1it);
			if(floor_it != floor_regions.end())
			{
				/* update the floor height of this wall */
				if(best_floor_ind < 0 
					|| this->floors[
					best_floor_ind].surface_area
					< this->floors[
					floor_it->second].surface_area)
				{
					/* n1it points to a better, larger
					 * floor for this wall, update the
					 * bounds */
					best_floor_ind = floor_it->second;
					this->walls[wit->second
						].update_zmin(
						this->floors[
						best_floor_ind].z);
				}
			}

			/* is this neighboring region (represented by
			 * the seed face) a ceiling? */
			ceil_it = ceiling_regions.find(*n1it);
			if(ceil_it != ceiling_regions.end())
			{
				/* update the ceiling height of 
				 * this wall */
				if(best_ceil_ind < 0
					|| this->ceilings[
					best_ceil_ind].surface_area
					< this->ceilings[
					ceil_it->second].surface_area)
				{
					/* nit points to a better, larger 
					 * ceiling for this wall, update 
					 * the bounds */
					best_ceil_ind = ceil_it->second;
					this->walls[wit->second
						].update_zmax(
						this->ceilings[
						best_ceil_ind].z);
				}
			}
		}
	}

	/* success */
	progbar.clear();
	toc(clk, "Finding surfaces");
	return 0;
}
		
int process_t::compute_level_splits(const oct2dq_run_settings_t& args)
{
	histogram_t floor_hist, ceil_hist;
	vector<double> floor_peaks, ceil_peaks;
	vector<double> floor_counts, ceil_counts;
	vector<double> floor_heights, ceil_heights;
	building_levels::file_t levelsfile;
	size_t i, fi, fi_next, ci, ci_next, fn, cn;
	tictoc_t clk;
	int ret;

	/* start timer */
	tic(clk);

	/* prepare histograms for analysis */
	floor_hist.set_resolution(this->tree.get_resolution());
	ceil_hist.set_resolution(this->tree.get_resolution());

	/* iterate over the floor regions that have been discovered,
	 * and record their elevation + surface area */
	fn = this->floors.size();
	for(fi = 0; fi < fn; fi++)
		floor_hist.insert(this->floors[fi].z, 
				this->floors[fi].surface_area);

	/* do the same thing for the ceiling regions */
	cn = this->ceilings.size();
	for(ci = 0; ci < cn; ci++)
		ceil_hist.insert(this->ceilings[ci].z, 
				this->ceilings[ci].surface_area);

	/* find locations peaks in the histograms */
	floor_hist.find_peaks(floor_peaks, floor_counts, 
				args.minlevelheight); 
	ceil_hist.find_peaks(ceil_peaks, ceil_counts, 
				args.minlevelheight); 

	/* clear output */
	floor_heights.clear();
	ceil_heights.clear();
	this->level_splits.clear();

	/* start at the first floor, and find the corresponding first
	 * ceiling */
	fn = floor_peaks.size();
	cn = ceil_peaks.size();
	fi = ci = 0;
	while(ci < cn && ceil_peaks[ci] <= floor_peaks[fi])
		ci++; /* get to a ceiling that's above the first floor */

	/* find matching floor/ceiling pairs until run out of surfaces */
	while(fi < fn && ci < cn)
	{
		/* find the floor with the highest count that is
		 * still below the current ceiling */
		for(i = fi+1; i<fn && floor_peaks[i] < ceil_peaks[ci]; i++)
			if(floor_counts[i] > floor_counts[fi])
				fi = i;

		/* figure out what the next floor above the current ceiling
		 * is */
		fi_next = fi+1;
		while(fi_next < fn 
				&& floor_peaks[fi_next] < ceil_peaks[ci])
			fi_next++;

		/* find the ceiling with the highest count that is below
		 * the next floor position */
		for(i = ci+1; i<cn && (fi_next >= fn 
				|| ceil_peaks[i] < floor_peaks[fi_next]); 
				i++)
			if(ceil_counts[i] > ceil_counts[ci])
				ci = i;
	
		/* we know have the optimum floor and ceiling positions
		 * for this level, so export those to the output */
		floor_heights.push_back(floor_peaks[fi]);
		ceil_heights.push_back(ceil_peaks[ci]);

		/* find the next ceiling */
		ci_next = ci+1;
		while(ci_next < cn && ceil_peaks[ci_next] 
					< floor_peaks[fi_next])
			ci_next++;

		/* move to next floor */
		fi = fi_next;
		ci = ci_next;
	}
	
	/* verify that each level has a floor and a ceiling */
	fn = floor_heights.size();
	cn = ceil_heights.size();
	if(fn != cn)
	{
		cerr << "[process_t::compute_level_splits]\tError! "
		     << "Computed " << fn << " floor heights and "
		     << cn << " ceiling heights." << endl;
		return -1;
	}
	if(fn == 0)
	{
		cerr << "[process_t::compute_level_splits]\tError! "
		     << "No floors or ceilings found!" << endl;
		return -2;
	}

	/* populate the level partition heights, which are the
	 * elevations where one level is partitioned from its
	 * neighboring levels 
	 *
	 * The length of this list is (N-1), where N is the number
	 * of discovered levels */
	level_splits.resize(fn-1);
	for(i = 1; i < fn; i++)
	{
		/* split halfway between the lower ceiling and the
		 * upper floor */
		this->level_splits[i-1] = 0.5 * (ceil_heights[i-1] 
				+ floor_heights[i]);
	}

	/* optionally export level partitioning to disk */
	if(!(args.levelsfile.empty()))
	{
		/* populate levels file */
		for(i = 0; i < fn; i++)
		{
			/* add current level */
			ret = levelsfile.insert(building_levels::level_t(
				i, floor_heights[i], ceil_heights[i]));
			if(ret)
			{
				cerr << "[process_t::compute_level_splits]"
				     << "\tError!  Unable to export level"
				     << " #" << i << endl;
				return PROPEGATE_ERROR(-3, ret);
			}
		}

		/* export .levels to disk */
		ret = levelsfile.write(args.levelsfile);
		if(ret)
		{
			cerr << "[process_t::compute_level_splits]\t"
			     << "ERROR! Unable to export .levels file "
			     << "to: " << args.levelsfile << endl;
			return PROPEGATE_ERROR(-4, ret);
		}
	}

	/* success */
	toc(clk, "Computing level ranges");
	return 0;
}

int process_t::compute_wall_samples(const oct2dq_run_settings_t& args)
{
	pair<nodefacemap_t::const_iterator, 
			nodefacemap_t::const_iterator> range;
	Vector2d p2d, n2d;
	octnode_t* leaf;
	quaddata_t* data;
	Vector3d p;
	progress_bar_t progbar;
	tictoc_t clk;
	double coord_a, coord_b;
	size_t level_index, wall_index, num_walls, num_levels;

	/* init */
	tic(clk);
	progbar.set_name("Wall sampling");

	/* initialize the quadtree wall sampling for each level */
	p2d << this->tree.get_root()->center(0),
	       this->tree.get_root()->center(1);
	num_levels = this->level_splits.size() + 1;
	this->sampling.resize(num_levels);
	for(level_index = 0; level_index < num_levels; level_index++)
	{
		/* initialize this level */
		this->sampling[level_index].set(args.dq_resolution, p2d,
			this->tree.get_root()->halfwidth);
	}

	/* iterate over all the regions we determined to be walls,
	 * and actually compute the wall samples to export */
	num_walls = this->walls.size();
	for(wall_index = 0; wall_index < num_walls; wall_index++)
	{
		/* update the progress bar */
		progbar.update(wall_index, num_walls);

		/* iterate over the plane of the region, adding
		 * samples taken uniformly.
		 *
		 * Doing it this way rather than just using the face
		 * centers as sample points allows for a couple advantages:
		 *
		 * 	- face centers may not be uniform, since
		 * 	  each face has its own halfwidth
		 * 	- if the wall is occluded by something, then
		 * 	  the face centers will not reflect that geometry,
		 * 	  whereas the bounding box will
		 *
		 * To ensure we don't over-sample regions not actually
		 * part of the wall, we check each point to make sure
		 * that it's part of the exterior volume of the model.
		 */
		for(coord_a = this->walls[wall_index].a_min; 
				coord_a <= this->walls[wall_index].a_max; 
					coord_a += args.dq_resolution)
			for(coord_b = this->walls[wall_index].b_min; 
				coord_b <= this->walls[wall_index].b_max;
					coord_b += args.dq_resolution)
			{
				/* reconstruct 3D point in world
				 * coordinates */
				p = this->walls[wall_index].vertical.point
					+ (this->walls[wall_index].a 
							* coord_a) 
					+ (this->walls[wall_index].b 
							* coord_b);

				/* get the octnode that contains this
				 * point. */
				leaf = this->tree.get_root()->retrieve(p);
				
				/* only export if the node is either a
				 * boundary or exterior */
				if(leaf == NULL || leaf->data == NULL)
					continue; /* don't use point */
				if(leaf->data->is_interior())
				{
					/* check if leaf is a boundary
					 * node, in which case we'll allow
					 * the use of the point */
					range = this->boundary.find_node(
							leaf);
					if(range.first == range.second)
						continue; /* not a boundary
							* node, don't use
							* it */
				}

				/* now that we want to insert this
				 * point as a wall sample, we should
				 * snap it to the vertically aligned
				 * plane that is the adjustment of the
				 * wall. */
				this->walls[wall_index]
					.vertical.project_onto(p);
		
				/* get the appropriate level index for
				 * this point */
				level_index 
					= this->level_of_elevation(p(2));

				/* get the 2D projection of this value,
				 * so we are able to insert it into the
				 * 2D structure of the wall samples */
				p2d << p(0), p(1);
				n2d << this->walls[wall_index]
						.vertical.normal(0),
				       this->walls[wall_index]
					       .vertical.normal(1);

				/* we can now use this point to
				 * contribute to wall samples */
				data = this->sampling[level_index]
					.insert(p2d, n2d, p(2), p(2), 
					this->walls[wall_index].strength);
				if(data == NULL)
				{
					/* error occurred */
					progbar.clear();
					cerr << "[process_t::"
					     << "compute_wall_samples]\t"
					     << "Unable to insert point "
					     << "into wall samples: " 
					     << p2d.transpose()
					     << endl;
					return -1;
				}

				/* keep track of which data came from
				 * which walls */
				this->ws_to_walls[data].insert(wall_index);
			}
	}
	
	/* success */
	progbar.clear();
	toc(clk, "Wall sampling");
	return 0;
}
		
int process_t::compute_pose_inds(const oct2dq_run_settings_t& args)
{
	map<quaddata_t*, pair<size_t, size_t> >::iterator pccit;
	system_path_t path;
	fss::reader_t infile;
	fss::frame_t frame;
	transform_t pose;
	Vector3d point_pos;
	progress_bar_t progbar;
	tictoc_t clk;
	double score;
	size_t pt_ind, num_pts;
	size_t file_ind, num_files, frame_ind, num_frames;
	size_t pose_ind;
	int ret;
	
	/* when assigning pose indices to wall samples, we want to keep
	 * track of which samples get a lot of poses, and which don't.
	 *
	 * The following structure keeps track of the ratio of times
	 * a wall sample was chosen, over the total number of times
	 * it was considered to hold a pose.
	 *
	 * Thanks goes to Nick for suggesting this idea.
	 */
	map<quaddata_t*, pair<size_t, size_t> > pose_choice_counts;

	/* read in the path information */
	tic(clk);
	ret = path.readnoisypath(args.pathfile);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::compute_pose_inds]"
		     << "\tUnable to read in path file: "
		     << args.pathfile << endl;
		return PROPEGATE_ERROR(-1, ret);
	}
	ret = path.parse_hardware_config(args.configfile);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::compute_pose_inds]"
		     << "\tUnable to read in hardware "
		     << "config xml file: "
		     << args.configfile << endl;
		return PROPEGATE_ERROR(-2, ret);
	}
	toc(clk, "Importing path");

	/* iterate over input fss files */
	num_files = args.fssfiles.size();
	for(file_ind = 0; file_ind < num_files; file_ind++)
	{
		/* open input fss data file */
		infile.set_correct_for_bias(true);
		ret = infile.open(args.fssfiles[file_ind]);
		if(ret)
		{
			/* report error */
			cerr << "[process_t::compute_pose_inds]"
			     << "\tUnable to read in fss data file: "
			     << args.fssfiles[file_ind] << endl;
			return PROPEGATE_ERROR(-3, ret);
		}

		/* prepare progress bar */
		tic(clk);
		progbar.set_name(infile.scanner_name());

		/* iterate through frames in file */
		num_frames = infile.num_frames();
		for(frame_ind = 0; frame_ind < num_frames; frame_ind++)
		{
			/* update status for user */
			progbar.update(frame_ind, num_frames);

			/* get the next scan in the file */
			ret = infile.get(frame, frame_ind);
			if(ret)
			{
				/* report error */
				progbar.clear();
				cerr << "[process_t::compute_pose_inds]\t"
				     << "Error!  Difficulty parsing "
				     << "fss scan #" << frame_ind << endl;
				return PROPEGATE_ERROR(-4, ret);
			}

			/* check if valid timestamp */
			if(path.is_blacklisted(frame.timestamp))
				continue;

			/* get the pose of the system at this time */
			ret = path.compute_transform_for(pose,
					frame.timestamp,
					infile.scanner_name());
			if(ret)
			{
				/* report error */
				progbar.clear();
				cerr << "[process_t::compute_pose_inds]\t"
				     << "Error! Can't compute fss pose at "
				     << "time " << frame.timestamp
				     << " for " << infile.scanner_name()
				     << endl;
				return PROPEGATE_ERROR(-5, ret);
			}

			/* get the index of this pose */
			pose_ind = (size_t) path.closest_index(
						frame.timestamp);

			/* iterate over the points in this frame */
			num_pts = frame.points.size();
			for(pt_ind = 0; pt_ind < num_pts; pt_ind++)
			{
				/* get the world coordinate for this
				 * point, represented as a line segment
				 * in 3D space */
				point_pos(0) = frame.points[pt_ind].x;
				point_pos(1) = frame.points[pt_ind].y;
				point_pos(2) = frame.points[pt_ind].z;
				pose.apply(point_pos);

				/* analyze this scan point */
				ret = this->analyze_scan(pose, pose_ind,
					point_pos, pose_choice_counts,
					args);
				if(ret)
				{
					/* report error */
					progbar.clear();
					cerr << "[process_t::compute_pose"
					     << "_inds]\t"
					     << "Unable to process "
					     << "scan." << endl;
					return PROPEGATE_ERROR(-6, ret);
				}
			}
		}

		/* clean up this file */
		progbar.clear();
		infile.close();
		toc(clk, "Computing pose indices");
	}

	/* remove any wall samples that have low pose information */
	for(pccit = pose_choice_counts.begin(); 
			pccit != pose_choice_counts.end(); pccit++)
	{
		/* get the ratio for the pose counts for 
		 * this wall sample */
		score = ((double) pccit->second.first) 
			/ ((double) pccit->second.second);

		/* check if this wall sample has good pose counts */
		if(score < args.choiceratiothresh)
		{
			/* bad pose count, so we want to throw
			 * away this wall sample */
			pccit->first->total_weight = 0.0;
		}
	}

	/* success */
	return 0;
}
		
int process_t::export_data(const oct2dq_run_settings_t& args) const
{
	stringstream filename;
	ofstream outfile;
	tictoc_t clk;
	size_t i, n;

	/* export each level */
	tic(clk);
	n = this->sampling.size();
	for(i = 0; i < n; i++)
	{
		/* determine name of output file */
		filename.clear();
		filename.str("");
		filename << args.dqfile_prefix << i << ".dq";

		/* prepare dq file to write to */
		outfile.open(filename.str().c_str());
		if(!(outfile.is_open()))
		{
			/* unable to write to disk */
			cerr << "[process_t::export_data]\t"
			     << "Unable to open file "
			     << "for writing: " << filename.str() << endl;
			return -1;
		}

		/* write the wall samples to the specified dq file */
		this->sampling[i].print(outfile);

		/* close the stream */
		outfile.close();
	}

	/* success */
	toc(clk, "Exporting wall samples");
	return 0;
}

/*------------------*/
/* helper functions */
/*------------------*/

double process_t::compute_region_strength(regionmap_t::const_iterator it,
				const oct2dq_run_settings_t& args) const
{
	double planarity, area, verticality;

	/* check surface area against threshold */
	area = it->second.get_region().surface_area();
	if(area < args.surfaceareathresh)
		return 0; /* don't give this region any strength,
		           * it's not big enough. */

	/* check verticality against input threshold */
	verticality = abs(it->second.get_region().get_plane().normal(2));
	if(verticality >= args.verticalitythresh)
		return 0; /* not aligned enough to be a wall */
	
	/* retrieve relevant values */
	planarity = it->second.compute_planarity();

	/* compute strength for this region */
	return area * planarity * (1 - verticality);
}
		
int process_t::analyze_scan(const transform_t& pose, size_t pose_ind,
				const Eigen::Vector3d& point_pos_orig,
				std::map<quaddata_t*, 
				std::pair<size_t, size_t> >& 
				pose_choice_counts,
				const oct2dq_run_settings_t& args)
{
	map<quaddata_t*, pair<size_t, size_t> >::iterator pccit;
	Vector3d dir, point_pos;
	Vector2d dir2d;
	linesegment_2d_t lineseg;
	vector<quaddata_t*> xings;
	double score, best_score;
	size_t node_ind, num_nodes, best_ind, level_index;
	
	/* get the building level that contains the current pose */
	level_index = this->level_of_elevation(pose.T(2));

	/* extend the line segment by some distance,
	 * to try to intersect any walls behind 
	 * objects */
	dir = (point_pos_orig - pose.T);
	dir.normalize();
	
	/* units: meters */
	point_pos = point_pos_orig + dir*(args.minroomsize); 
	dir2d << dir(0), dir(1); /* projection of 
				normal into R^2 */
	dir2d.normalize();

	/* prepare the line segment */
	lineseg.init(pose.T, point_pos);

	/* find the nodes that it intersects 
	 * in the 2D representation of the 
	 * environment.
	 *
	 * Only check for occlusions in the current building
	 * level, since we don't care about horizontal intersections
	 * on a completely different vertical level. */
	xings.clear();
	this->sampling[level_index].raytrace(xings, lineseg);

	/* iterate through the intersected
	 * nodes, and record this pose with the
	 * associated wall sample that had the
	 * best score out of any that were
	 * intersected */
	best_score = 0;
	num_nodes = xings.size();
	best_ind = num_nodes;
	for(node_ind = 0; node_ind < num_nodes; node_ind++)
	{
		/* ignore nodes with null data */
		if(xings[node_ind] == NULL)
			continue;
	
		/* augment score by normal analysis,
		 * We only want to count walls
		 * that have their normals facing
		 * the scanner (where the below
		 * dot-product would be negative) */
		if(xings[node_ind]->normal.dot(dir2d) >= 0)
			continue;

		/* check against best */
		score = xings[node_ind]->total_weight;
		if(score > best_score)
		{
			/* we found a new best */
			best_score = score;
			best_ind = node_ind;
		}
	}

	/* check if we found anything */
	if(best_ind >= num_nodes)
		return 0; /* no good nodes, don't bother with this scan */

	/* apply this pose index to wall sample 
	 * with best score */
	xings[best_ind]->pose_inds.insert(pose_ind);
			
	/* iterate through each wall sample
	 * that was considered for this pose,
	 * and update its choice counts.
	 *
	 * These counts record how many times
	 * a sample was chosen over how many
	 * times it was considered. */
	for(node_ind = 0; node_ind < num_nodes; node_ind++)
	{
		/* ignore wall samples that
		 * were facing away from the
		 * scanner */
		if(xings[node_ind]->normal.dot(dir2d) >= 0)
			continue;

		/* get the pair that represents
		 * this ratio.
		 *
		 * The following call will get
		 * the existing ratio if it exists,
		 * or insert a new ratio in the
		 * map if it doesn't. */
		pccit = pose_choice_counts.insert(
				pair<quaddata_t*, pair<size_t,size_t> >(
				xings[node_ind], pair<size_t,size_t>(0,0)))
				.first;

		/* update this ratio */
		if(xings[node_ind]->normal.dot(dir2d) >= 0)
			continue;
		pccit->second.second ++;
		
		/* we want to count this sample as used if it is part
		 * of the same wall that was actually chosen */
		if(this->shares_a_wall(xings[node_ind], xings[best_ind]))
			pccit->second.first++;
	}

	/* success */
	return 0;
}
		
bool process_t::shares_a_wall(quaddata_t* a, quaddata_t* b) const
{
	map<quaddata_t*, std::set<size_t> >::const_iterator ita, itb;
	vector<size_t> intersection;
	vector<size_t>::iterator xit;
	size_t s;

	/* check trivial case */
	if(a == b)
		return true;

	/* find these wall samples in the map */
	s = 0;
	ita = this->ws_to_walls.find(a);
	if(ita == this->ws_to_walls.end())
		return false;
	s += ita->second.size();
	itb = this->ws_to_walls.find(b);
	if(itb == this->ws_to_walls.end())
		return false;
	s += itb->second.size();
	intersection.resize(s);

	/* find the set intersection of the two sets of originating
	 * walls for these samples */
	xit = set_intersection(ita->second.begin(), ita->second.end(),
			itb->second.begin(), itb->second.end(),
			intersection.begin());
	return (xit - intersection.begin() > 0);
}
		
size_t process_t::level_of_elevation(double z) const
{
	size_t i, n;
	
	/* check base case (no splits) */
	n = this->level_splits.size();
	if(n == 0)
		return 0; /* entire building is 'first level' */

	/* check the elevation against the given set of splits */
	for(i = 0; i < n; i++)
		if(z < this->level_splits[i])
			return i; /* below threshold for i'th level */

	/* must be at the last (top) level */
	return n; /* there are N-1 splits for N levels */
}
