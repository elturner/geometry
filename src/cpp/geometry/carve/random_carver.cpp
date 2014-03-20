#include "random_carver.h"
#include <mesh/floorplan/floorplan.h>
#include <geometry/shapes/extruded_poly.h>
#include <geometry/shapes/chunk_exporter.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <geometry/carve/frame_model.h>
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
	this->num_rooms = 0;
}

int random_carver_t::init(const string& madfile, const string& confile,
                          double res, double clk_err, double carvebuf)
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
	this->carving_buffer = carvebuf;
	this->num_rooms = 0;

	/* success */
	return 0;
}
		
int random_carver_t::export_chunks(const vector<string>& fss_files,
                                   const string& chunklist,
                                   const string& chunk_dir)
{
	chunk_exporter_t chunker;
	vector<string> sensor_names;
	fss::reader_t infile;
	fss::frame_t inframe;
	progress_bar_t progbar;
	scan_model_t model;
	frame_model_t curr_frame, prev_frame;
	string label;
	tictoc_t clk;
	unsigned int i, si, n, num_sensors;
	int ret;

	/* open chunk exporter */
	chunker.open(chunklist, chunk_dir);

	/* iterate over scan files */
	num_sensors = fss_files.size();
	sensor_names.resize(num_sensors);
	for(si = 0; si < num_sensors; si++)
	{
		/* read in scan file */
		tic(clk);
		infile.set_correct_for_bias(true); /* correct bias */
		infile.set_convert_to_meters(true); /* units: meters */
		ret = infile.open(fss_files[si]);
		if(ret)
		{
			/* unable to parse, report to user */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Error " << ret
			     << ": Unable to parse input scan file "
			     << fss_files[si] << endl;
			return ret;
		}

		/* record name of sensor */
		sensor_names[si] = infile.scanner_name();

		/* prepare noisy model for this scanner */
		ret = model.set_sensor(infile.scanner_name(), 
	                       this->clock_uncertainty, this->path);
		if(ret)
		{
			/* not a recognized sensor */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[random_carver_t::export_chunks]\t"
			     << "Error " << ret
			     << ": Unable to recognize sensor \""
			     << infile.scanner_name() << "\"" << endl;
			return ret;
		}
		label = "Parsing " + infile.scanner_name();
		toc(clk, label.c_str());

		/* iterate through scans */
		tic(clk);
		progbar.set_name(infile.scanner_name());
		n = infile.num_frames();
		for(i = 0; i < n; i++)
		{
			/* inform user of progress */
			progbar.update(i, n);
			
			/* parse current frame, update user on progress */
			ret = infile.get(inframe, i);
			if(ret)
			{
				/* error occurred reading frame, 
				 * inform user */
				progbar.clear();
				ret = PROPEGATE_ERROR(-3, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to parse frame #"
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}
		
			/* compute carving model for this frame */
			ret = curr_frame.init(inframe, model, this->path);
			if(ret)
			{
				/* error occurred computing 
				 * frame parameters */
				progbar.clear();
				ret = PROPEGATE_ERROR(-4, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to compute frame #" 
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}
		
			/* only proceed from here if we have two frame's 
			 * worth of data, so we can interpolate the 
			 * distributions between them and carve the 
			 * corresponding volume. */
			if(i == 0)
			{
				/* prepare for the next frame */
				curr_frame.swap(prev_frame);
				continue;
			}

			/* add frame information to octree */
			ret = prev_frame.export_chunks(
				this->tree, curr_frame,
				this->carving_buffer, si, i-1,
				chunker);
			if(ret)
			{
				/* error occurred carving this frame */
				progbar.clear();
				ret = PROPEGATE_ERROR(-5, ret);
				cerr << "[random_carver_t::export_chunks]\t"
				     << "Unable to chunk frame #" << i
				     << ", Error " << ret << endl;
				return ret;
			}

			/* prepare for the next frame */
			curr_frame.swap(prev_frame);
		}

		/* inform user that processing is finished */
		progbar.clear();
		label = "Chunking of " + infile.scanner_name();
		toc(clk, label.c_str());
	}

	/* close this chunker, which will finish exporting all files */
	chunker.close(this->tree, sensor_names);

	/* success */
	return 0;
}

int random_carver_t::carve(const string& fssfile)
{
	fss::reader_t infile;
	fss::frame_t inframe;
	progress_bar_t progbar;
	scan_model_t model;
	frame_model_t curr_frame, prev_frame;
	string label;
	tictoc_t clk;
	unsigned int i, n;
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
		/* inform user of progress */
		progbar.update(i, n);
		
		/* parse the current frame and update user on progress */
		ret = infile.get(inframe, i);
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
		
		// TODO planarity/edge info about scan?

		/* compute carving model for this frame */
		ret = curr_frame.init(inframe, model, this->path);
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
		
		/* only proceed from here if we have two frame's worth
		 * of data, so we can interpolate the distributions
		 * between them and carve the corresponding volume. */
		if(i == 0)
		{
			/* prepare for the next frame */
			curr_frame.swap(prev_frame);
			continue;
		}

		/* add frame information to octree */
		ret = prev_frame.carve(this->tree, curr_frame,
		                       this->carving_buffer);
		if(ret)
		{
			/* error occurred carving this frame */
			progbar.clear();
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[random_carver_t::carve]\tUnable to "
			     << "carve frame #" << i << " into tree, "
			     << "Error " << ret << endl;
			return ret;
		}

		/* prepare for the next frame */
		curr_frame.swap(prev_frame);
	}

	/* inform user that processing is finished */
	progbar.clear();
	label = "Random carving of " + infile.scanner_name();
	toc(clk, label.c_str());

	/* success */
	return 0;
}
		
int random_carver_t::import_fp(const std::string& fpfile)
{
	fp::floorplan_t f;
	extruded_poly_t poly;
	tictoc_t clk;
	unsigned int i, n;
	int ret;

	/* read in floor plan */
	tic(clk);
	ret = f.import_from_fp(fpfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* iterate over the rooms of this floorplan, and generate
	 * a shape object for each room */
	n = f.rooms.size();
	for(i = 0; i < n; i++)
	{
		/* create shape */
		poly.init(f, this->num_rooms + i, i);
	
		/* import into tree */
		this->tree.find(poly);
	}

	/* update number of rooms in building */
	this->num_rooms += n;
	toc(clk, "Importing floor plan");

	/* success */
	return 0;
}

int random_carver_t::serialize(const string& octfile) const
{
	tictoc_t clk;
	int ret;

	/* serialize the octree */
	tic(clk);
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
	toc(clk, "Exporting octfile");

	/* success */
	return 0;
}
