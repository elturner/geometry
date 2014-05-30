#include "wedge_generator.h"
#include <io/data/fss/fss_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <geometry/carve/frame_model.h>
#include <geometry/carve/gaussian/scan_model.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file wedge_generator.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Computes all wedges and writes them to file
 *
 * @section DESCRIPTION
 *
 * This file contains a class that will perform all probabilistic
 * computations for a set of scanners, in order to compute all
 * wedges from these scanners.  These wedges can then we exported
 * to a binary file.
 */

using namespace std;

/* function implementations */

int wedge_generator_t::init(const string& pathfile,
                            const string& confile,
                            const string& tsfile,
                            double dcu, double carvebuf, double lf_dist)
{
	int ret;

	/* initialize the structures */
	ret = this->path.readnoisypath(pathfile);
	if(ret)
	{
		/* report error to user */
		cerr << "[wedge_generator_t::init]\tUnable to initialize"
		     << " path of system, Error " << ret << endl;
		return PROPEGATE_ERROR(-1, ret); /* cannot read file */
	}
	ret = this->path.parse_hardware_config(confile);
	if(ret)
	{
		/* report error to user */
		cerr << "[wedge_generator_t::init]\tUnable to initialize"
		     << " system hardware config, Error " << ret << endl;
		return PROPEGATE_ERROR(-2, ret); /* cannot read file */
	}
	if(!this->timesync.read(tsfile))
	{
		/* report error to user */
		cerr << "[wedge_generator_t::init]\tUnable to parse the "
		     << "time synchronization output xml file: "
		     << tsfile << endl;
		return PROPEGATE_ERROR(-3, ret);
	}

	/* initialize octree and algorithm parameters */
	this->carving_buffer = carvebuf;
	this->default_clock_uncertainty = dcu;
	this->linefit_dist = lf_dist;

	/* success */
	return 0;
}
		
int wedge_generator_t::process(const vector<string>& fssfiles,
                               const string& cmfile,
                               const string& wedgefile) const
{
	progress_bar_t progbar;
	wedge::writer_t wedge_outfile;
	cm_io::writer_t cm_outfile;
	fss::reader_t infile;
	fss::frame_t inframe;
	scan_model_t model;
	frame_model_t curr_frame, prev_frame;
	string label;
	tictoc_t clk;
	size_t i, si, n, num_sensors, total_num_frames;
	int ret;

	/* prepare output files */

	/* prepare carvemap file */
	ret = cm_outfile.open(cmfile);
	if(ret)
	{
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[wedge_generator_t::process]\tError " << ret
		     << ": Could not open output carvemap file for writing:"
		     << " " << cmfile << endl << endl;
		return ret;
	}
	
	/* prepare wedge file */
	ret = wedge_outfile.open(wedgefile, this->carving_buffer);
	if(ret)
	{
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[wedge_generator_t::process]\tERROR " << ret
		     << ": Could not open output file for writing:"
		     << wedgefile << endl << endl;
		return ret;
	}

	/* iterate over scan files */
	num_sensors = fssfiles.size();
	total_num_frames = 0;
	for(si = 0; si < num_sensors; si++)
	{
		/* read in scan file */
		tic(clk);
		infile.set_correct_for_bias(true); /* correct bias */
		infile.set_convert_to_meters(true); /* units: meters */
		ret = infile.open(fssfiles[si]);
		if(ret)
		{
			/* unable to parse, report to user */
			ret = PROPEGATE_ERROR(-3, ret);
			infile.close();
			cm_outfile.close();
			wedge_outfile.close();
			cerr << "[wedge_generator_t::process]\t"
			     << "Error " << ret
			     << ": Unable to parse input scan file "
			     << fssfiles[si] << endl;
			return ret;
		}

		/* prepare noisy model for this scanner */
		ret = model.set_sensor(infile.scanner_name(), 
	        	this->get_clock_uncertainty_for_sensor(
				infile.scanner_name()), this->path);
		if(ret)
		{
			/* not a recognized sensor */
			ret = PROPEGATE_ERROR(-4, ret);
			infile.close();
			cm_outfile.close();
			wedge_outfile.close();
			cerr << "[wedge_generator_t::process]\t"
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
		for(i = 0; i < n; i++, total_num_frames++)
		{
			/* inform user of progress */
			progbar.update(i, n);
		
			/* parse current frame */
			ret = infile.get(inframe, i);
			if(ret)
			{
				/* error occurred reading frame, 
				 * inform user */
				progbar.clear();
				infile.close();
				cm_outfile.close();
				wedge_outfile.close();
				ret = PROPEGATE_ERROR(-5, ret);
				cerr << "[wedge_generator_t::process]\t"
				     << "Unable to parse frame #"
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}
		
			/* compute carving model for this frame */
			ret = curr_frame.init(inframe, infile.angle(),
			                      this->linefit_dist,
			                      model, this->path);
			if(ret)
			{
				/* error occurred computing 
				 * frame parameters */
				progbar.clear();
				infile.close();
				cm_outfile.close();
				wedge_outfile.close();
				ret = PROPEGATE_ERROR(-6, ret);
				cerr << "[wedge_generator_t::process]\t"
				     << "Unable to compute frame #" 
				     << i << ", Error "
				     << ret << endl;
				return ret;
			}

			/* export the carve maps for this frame */
			ret = curr_frame.serialize_carvemaps(cm_outfile);
			if(ret)
			{
				/* error occurred */
				progbar.clear();
				infile.close();
				cm_outfile.close();
				wedge_outfile.close();
				ret = PROPEGATE_ERROR(-7, ret);
				cerr << "[wedge_generator_t::process]\t"
				     << "Error " << ret << ": Unable to "
				     << "export carve maps for frame "
				     << i << endl << endl;
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

			/* export all this frame's wedges to file */
			ret = prev_frame.serialize_wedges(wedge_outfile,
					total_num_frames-1, curr_frame);
			if(ret <= 0)
			{
				/* an error occurred */
				progbar.clear();
				infile.close();
				cm_outfile.close();
				wedge_outfile.close();
				ret = PROPEGATE_ERROR(-8, ret);
				cerr << "[wedge_generator_t::process]\t"
				     << "Error " << ret << ": Unable to "
				     << "serialize frame #" << (i-1)
				     << endl << endl;
				return ret;
			}

			/* prepare for the next frame */
			curr_frame.swap(prev_frame);
		}

		/* inform user that processing is finished */
		progbar.clear();
		infile.close();
		label = "Generating wedges for " + infile.scanner_name();
		toc(clk, label.c_str());
	}

	/* clean up */
	cm_outfile.close();
	wedge_outfile.close();

	/* success */
	return 0;
}

double wedge_generator_t::get_clock_uncertainty_for_sensor(
                        const string& sensor_name) const
{
	FitParams clk;

	/* get the timesync parameters for this sensor */
	clk = this->timesync.get(sensor_name);

	/* check if std. dev. is valid */
	if(clk.stddev < 0)
		return this->default_clock_uncertainty;

	/* return this sensor's specific uncertainty */
	return clk.stddev;
}
