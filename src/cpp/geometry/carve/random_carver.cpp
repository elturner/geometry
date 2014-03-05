#include "random_carver.h"
#include <geometry/probability/noise_model.h>
#include <geometry/carve/point_carver.h>
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
	/* set default values */
	this->num_samples = 0;
}

int random_carver_t::init(const string& madfile, const string& confile,
                          double res, unsigned int num_samps,
                          double clk_err)
{
	int ret;

	/* initialize the probabilistic structures */
	ret = this->model.set_path(madfile, confile);
	if(ret)
	{
		/* report error to user */
		cerr << "[random_carver_t::init]\tUnable to initialize"
		     << " noisy model of scanner, Error " << ret << endl;
		return PROPEGATE_ERROR(-1, ret); /* cannot read files */
	}

	/* initialize octree and algorithm parameters */
	this->tree.set_resolution(res);
	this->num_samples = num_samps;
	this->clock_uncertainty = clk_err;

	/* success */
	return 0;
}

int random_carver_t::carve(const string& fssfile)
{
	fss::reader_t infile;
	fss::frame_t frame;
	progress_bar_t progbar;
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
	ret = this->model.set_sensor(infile.scanner_name());
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
		model.set_timestamp(frame.timestamp,
		                    this->clock_uncertainty);

		/* iterate over points in frame */
		m = frame.points.size();
		for(j = 0; j < m; j++)
		{
			/* get statistics of j'th point */
			model.set_scan(frame.points[j]);

			// TODO planarity/edge info about scan?

			/* generate samples for this scan point */
			ret = this->carve_current_point();
			if(ret)
			{
				/* inform user of error */
				progbar.clear();
				ret = PROPEGATE_ERROR(-4, ret);
				cerr << "[random_carver_t::carve]\tUnable"
				     << " to carve point #" << j << " in"
				     << " frame #" << i << " of "
				     << infile.scanner_name() << ", Error "
				     << ret << endl;
				return ret;
			}
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

/* helper functions */

int random_carver_t::carve_current_point()
{
	point_carver_t volume;
	Eigen::Vector3d sensor_pos, scan_pos;
	unsigned int k;
	int ret;

	/* generate samples for this point's position */
	for(k = 0; k < this->num_samples; k++)
	{
		/* generate next sample */
		ret = this->model.generate_sample(sensor_pos, scan_pos);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
		
		/* add to distribution model for volume intersection */
		ret = volume.add_sample(sensor_pos, scan_pos, this->tree);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}
	
	/* incorporate volume model into full octree */
	ret = volume.update_tree();
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* success */
	return 0;
}
