#include "process.h"
#include "oct2dq_run_settings.h"
#include <io/data/fss/fss_io.h>
#include <geometry/shapes/shape_wrapper.h>
#include <geometry/shapes/linesegment.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octtopo.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <mesh/refine/octree_padder.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <map>
#include <iostream>
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
		
int process_t::compute_wall_samples(const oct2dq_run_settings_t& args)
{
	progress_bar_t progbar;
	regionmap_t::const_iterator it;
	faceset_t::const_iterator fit;
	octnode_t* leaf;
	pair<nodefacemap_t::const_iterator, 
			nodefacemap_t::const_iterator> range;
	Vector3d a, b, p;
	wall_sample_t ws;
	plane_t vertical;
	double strength, a_min, a_max, b_min, b_max, coord_a, coord_b;
	tictoc_t clk;
	size_t i;

	/* initialize */
	tic(clk);
	this->sampling.init(args.dq_resolution,
			this->tree.get_root()->center(0),
			this->tree.get_root()->center(1),
			this->tree.get_root()->halfwidth);
	this->node_ws_map.clear();
	progbar.set_name("Wall Sampling");
	i = 0;

	/* iterate over regions, computing strength for wall samples */
	for(it = this->region_graph.begin();
			it != this->region_graph.end(); it++)
	{
		/* show status to user */
		progbar.update(i++, this->region_graph.size());

		/* populate this->region_strengths for each region */
		strength = this->compute_region_strength(it, args);
		
		/* only proceed if strength is good enough */
		if(strength <= 0)
			continue;

		/* get coordinate frame along this planar region */
		b << 0,0,1;
		const Vector3d& n 
			= it->second.get_region().get_plane().normal;
		a = b.cross(n).normalized(); /* most-horizontal coord */
		b = n.cross(a); /* most-vertical coord */

		/* get a version of the region plane that's perfectly
		 * vertical. */
		vertical = it->second.get_region().get_plane();
		vertical.normal(2) = 0; /* normal must be horizontal */
		vertical.normal.normalize();

		/* get bounding box of the planar region */
		it->second.get_region().compute_bounding_box(a, b,
				a_min, a_max, b_min, b_max);

		/* compare bounding box to wall height threshold,
		 * just to make sure we want to use this region */
		if(b_max - b_min < args.wallheightthresh)
			continue; /* don't use region */

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
		for(coord_a = a_min; coord_a <= a_max; 
					coord_a += args.dq_resolution)
			for(coord_b = b_min; coord_b <= b_max;
					coord_b += args.dq_resolution)
			{
				/* reconstruct 3D point in world
				 * coordinates */
				p = it->second.get_region()
						.get_plane().point
					+ (a*coord_a) + (b*coord_b);

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
				vertical.project_onto(p);

				/* we can now use this point to
				 * contribute to wall samples */
				ws = this->sampling.add(p(0), p(1), 
						p(2), p(2), strength);
		
				/* store the pairing between this
				 * wall sample and the given leaf node.
				 *
				 * Since the data value in a leaf will
				 * never be null (because we padded the
				 * tree), then saving the octdata is
				 * equivalent to saving the node itself
				 */
				if(leaf->data != NULL)
					this->node_ws_map.insert(
						pair<octdata_t*,
						set<wall_sample_t> >(
						leaf->data, 
						set<wall_sample_t>())
						).first->second.insert(ws);
			}
	}
	
	/* success */
	progbar.clear();
	toc(clk, "Computing wall samples");
	return 0;
}
		
int process_t::compute_pose_inds(const oct2dq_run_settings_t& args)
{
	system_path_t path;
	fss::reader_t infile;
	fss::frame_t frame;
	transform_t pose;
	Vector3d point_pos;
	linesegment_t lineseg;
	shape_wrapper_t shapewrap;
	nodewsmap_t::iterator wsit;
	set<wall_sample_t>::iterator sit;
	progress_bar_t progbar;
	tictoc_t clk;
	size_t file_ind, num_files, frame_ind, num_frames;
	size_t pt_ind, num_pts, node_ind, num_nodes;
	size_t pose_ind;
	int ret;
	
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

				/* prepare the line segment */
				lineseg.init(pose.T, point_pos);

				/* find the nodes that it intersects */
				shapewrap.find_in_tree(lineseg, this->tree);

				/* iterate through the intersected
				 * nodes, and record this pose with any
				 * associated wall samples */
				num_nodes = shapewrap.data.size();
				for(node_ind = 0; node_ind < num_nodes;
							node_ind++)
				{
					/* ignore nodes with null data */
					if(shapewrap.data[node_ind] == NULL)
						continue;

					/* retrieve any wall samples
					 * for this node data */
					wsit = this->node_ws_map.find(
						shapewrap.data[node_ind]);
					if(wsit == this->node_ws_map.end())
						continue; /* no samples 
						           * here */

					/* iterate over samples at this
					 * node */
					for(sit = wsit->second.begin();
						sit != wsit->second.end();
							sit++)
						/* add pose to sample */
						this->sampling.add(*sit,
								pose_ind);
				}
			}
		}

		/* clean up this file */
		progbar.clear();
		infile.close();
		toc(clk, "Computing pose indices");
	}

	/* success */
	return 0;
}
		
int process_t::export_data(const oct2dq_run_settings_t& args) const
{
	tictoc_t clk;
	int ret;

	/* write the wall samples to the specified dq file */
	tic(clk);
	ret = this->sampling.writedq(args.dqfile);
	if(ret)
	{
		cerr << "[process_t::export_data]\tUnable to write "
		     << "wall samples to dq file: " << args.dqfile
		     << endl;
		return PROPEGATE_ERROR(-1, ret);
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
