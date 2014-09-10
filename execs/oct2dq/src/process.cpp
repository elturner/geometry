#include "process.h"
#include "oct2dq_run_settings.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octtopo.h>
#include <geometry/system_path.h>
#include <mesh/refine/octree_padder.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region_graph.h>
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

	/* read in the path information */
	tic(clk);
	ret = path.readnoisypath(args.pathfile);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to read in path file: "
		     << args.pathfile << endl;
		return PROPEGATE_ERROR(-1, ret);
	}
	ret = path.parse_hardware_config(args.configfile);
	if(ret)
	{
		/* report error */
		cerr << "[process_t::init]\tUnable to read in hardware "
		     << "config xml file: "
		     << args.configfile << endl;
		return PROPEGATE_ERROR(-2, ret);
	}
	toc(clk, "Importing path");

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
	regionmap_t::const_iterator it;
	faceset_t::const_iterator fit;
	octnode_t* leaf;
	pair<nodefacemap_t::const_iterator, 
			nodefacemap_t::const_iterator> range;
	Vector3d a, b, p;
	double strength, a_min, a_max, b_min, b_max, coord_a, coord_b;
	tictoc_t clk;

	/* initialize */
	tic(clk);
	this->sampling.init(args.dq_resolution,
			this->tree.get_root()->center(0),
			this->tree.get_root()->center(1),
			this->tree.get_root()->halfwidth);

	/* iterate over regions, computing strength for wall samples */
	for(it = this->region_graph.begin();
			it != this->region_graph.end(); it++)
	{
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

				/* we can now use this point to
				 * contribute to wall samples */
				this->sampling.add(p(0), p(1), 
						p(2), p(2), strength);
			}
	}
	
	/* success */
	toc(clk, "Computing wall samples");
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
