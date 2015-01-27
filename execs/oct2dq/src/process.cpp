#include "process.h"
#include "oct2dq_run_settings.h"
#include "wall_region_info.h"
#include "horizontal_region_info.h"
#include <io/data/fss/fss_io.h>
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
#include <util/progress_bar.h>
#include <util/error_codes.h>
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
		cerr << "[process_t::init]\tUnable to read in octree file: "
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
	horizontal_region_info_t hori_info;
	wall_region_info_t wall_info;
	regionmap_t::const_iterator it;
	faceset_t::const_iterator fit, n1it, n2it;
	progress_bar_t progbar;
	tictoc_t clk;
	double height;
	size_t i;

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
		wall_info.strength = this->compute_region_strength(it,args);
	
		/* only proceed if strength is good enough */
		if(wall_info.strength <= 0)
			continue;

		/* initialize stored values in wall_info struct */
		wall_info.init(wall_info.strength, it->second.get_region());

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
	progbar.set_name("Finding floors/ceilings");
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
		if(!(hori_info.init(it->second.get_region(), args)))
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
	// TODO

	/* success */
	progbar.clear();
	toc(clk, "Finding surfaces");
	return 0;
}
		
int process_t::compute_level_splits(const oct2dq_run_settings_t& args)
{
	// TODO implement me
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
	size_t wall_index, num_walls;

	/* init */
	tic(clk);
	progbar.set_name("Wall sampling");

	/* initialize the quadtree wall sampling */
	p2d << this->tree.get_root()->center(0),
	       this->tree.get_root()->center(1);
	this->sampling.set(args.dq_resolution, p2d,
			this->tree.get_root()->halfwidth);
	
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
				data = this->sampling.insert(p2d, n2d,
					p(2), p(2), 
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
				     << "Error!  Can't compute fss pose at "
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
					     << "_inds]\tUnable to process "
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
		/* get the ratio for the pose counts for this wall sample */
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
	ofstream outfile;
	tictoc_t clk;

	/* prepare dq file to write to */
	outfile.open(args.dqfile.c_str());
	if(!(outfile.is_open()))
	{
		/* unable to write to disk */
		cerr << "[process_t::export_data]\tUnable to open file "
		     << "for writing: " << args.dqfile << endl;
		return -1;
	}

	/* write the wall samples to the specified dq file */
	tic(clk);
	this->sampling.print(outfile);
	toc(clk, "Exporting wall samples");

	/* success */
	outfile.close();
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
	size_t node_ind, num_nodes, best_ind;
	
	/* extend the line segment by some distance,
	 * to try to intersect any walls behind 
	 * objects */
	dir = (point_pos_orig - pose.T);
	dir.normalize();
	
	/* units: meters */
	point_pos = point_pos_orig + dir*(args.minroomsize); 
	dir2d << dir(0), dir(1); /* projection of 
				normal into R^2 */

	/* prepare the line segment */
	lineseg.init(pose.T, point_pos);

	/* find the nodes that it intersects 
	 * in the 2D representation of the 
	 * environment */
	xings.clear();
	this->sampling.raytrace(xings, lineseg);

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
