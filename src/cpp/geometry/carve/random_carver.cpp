#include "random_carver.h"
#include <geometry/carve/gaussian/noisy_scanpoint.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/octree/octree.h>
#include <io/data/fss/fss_io.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <iostream>
#include <string>
#include <map>

/**
 * @file random_carver.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the random_carver_t class, which
 * is used to populate an octree with imported range scans,
 * in a probalistic fashion.
 *
 * This file requires the Eigen framework to be linked.
 */

using namespace std;

/* function implementations */
		
random_carver_t::random_carver_t()
{
	/* default parameter values */
	this->clock_uncertainty = 0;
}

int random_carver_t::init(const string& madfile, const string& confile,
                          double res, double clk_err)
{
	int ret;

	/* initialize the structures */
	ret = this->path.readmad(madfile);
	if(ret)
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to initialize"
		     << " path of system, Error " << ret << endl;
		return PROPEGATE_ERROR(-1, ret); /* cannot read file */
	}
	ret = this->path.parse_hardware_config(confile);
	if(ret)
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to initialize"
		     << " system hardware config, Error " << ret << endl;
		return PROPEGATE_ERROR(-2, ret); /* cannot read file */
	}

	/* initialize octree and algorithm parameters */
	this->tree.set_resolution(res);
	this->clock_uncertainty = clk_err;

	/* success */
	return 0;
}

int random_carver_t::carve(const string& fssfile)
{
	fss::reader_t infile;
	fss::frame_t frame;
	ofstream outfile;
	progress_bar_t progbar;
	scan_model_t model;
	noisy_scanpoint_t point;
	string label;
	tictoc_t clk;
	unsigned int i, n, j, m;
	int ret;

	/* read in scan file */
	tic(clk);
	infile.set_correct_for_bias(true); /* correct statistical bias */
	infile.set_convert_to_meters(true); /* we want units of meters */
	ret = infile.open(fssfile);
	if(ret)
	{
		/* unable to parse, report to user */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::carve]\tError " << ret
		     << ": Unable to parse input scan file "
		     << fssfile << endl;
		return ret;
	}

	/* prepare noisy model for this scanner */
	ret = model.set_sensor(infile.scanner_name(), 
	                       this->clock_uncertainty, this->path);
	if(ret)
	{
		/* not a recognized sensor */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[random_carver_t::carve]\tError " << ret
		     << ": Unable to recognize sensor \""
		     << infile.scanner_name() << "\"" << endl;
		return ret;
	}
	toc(clk, "Parsing scan file");

	/* iterate through scans, incorporating them into the octree */
	tic(clk);
	progbar.set_name(infile.scanner_name());
	n = infile.num_frames();
	for(i = 0; i < n; i++)
	{
		/* parse the current frame and update user on progress */
		progbar.update(i, n);
		ret = infile.get(frame, i);
		if(ret)
		{
			/* error occurred reading frame, inform user */
			progbar.clear();
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "parse frame #" << i << ", Error "
			     << ret << endl;
			return ret;
		}

		/* prepare model for this frame */
		ret = model.set_frame(frame.timestamp, this->path);
		if(ret)
		{
			/* error occurred computing frame parameters */
			progbar.clear();
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "compute frame #" << i << ", Error "
			     << ret << endl;
			return ret;
		}

		/* iterate over points in frame */
		m = frame.points.size();
		for(j = 0; j < m; j ++)
		{
			/* get statistics of j'th point */
			point.set(frame.points[j].x,
			          frame.points[j].y,
				  frame.points[j].z,
				  frame.points[j].stddev,
				  frame.points[j].width);
			if(!point.has_finite_noise())
				continue;

			/* model the combined statistics of this point */
			model.set_point(point);

			// TODO planarity/edge info about scan?
		
			// TODO add to octree
		}
	}
	
	/* inform user that processing is finished */
	progbar.clear();
	label = "Random carving of " + infile.scanner_name();
	toc(clk, label.c_str());

	/* success */
	return 0;
}

int random_carver_t::serialize(const string& octfile) const
{
	int ret;

	/* serialize the octree */
	ret = this->tree.serialize(octfile);
	if(ret)
	{
		/* unable to write to file, inform user */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[random_carver_t::serialize]\tError " << ret
		     << ": Unable to write to output file "
		     << octfile << endl;
		return -1;
	}

	/* success */
	return 0;
}
