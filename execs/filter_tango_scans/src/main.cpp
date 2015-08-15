#include "filter_tango_scans_settings.h"
#include <io/data/tango/tango_io.h>
#include <io/data/fss/fss_io.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>
#include <float.h>
#include <cmath>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@indoorreality.com>
 * @brief  The main file for the filter_tango_scans program
 *
 * @section DESCRIPTION
 *
 * This program is used to parse the tango .dat files, and store
 * them in a more scanner-agnostic format of .fss files.  This
 * program also enables the user to specify an output .mad file
 * to store the tango path.
 */

using namespace std;
using namespace tango_io;

/* the following constants are used to define the tango depth sensor's
 * noise characteristics 
 *
 * These values were acquired from experimental testing and
 * from the source:
 *
 * https://developers.google.com/project-tango/overview/depth-perception
 *
 * The standard deviation of the points is reported to be "a few 
 * centimeters".  Experimental measurements put it at less than a centimeter
 * average, so this program will model a linear increase in stddev based
 * on distance, adding one centimeter of stddev for each meter the
 * point is away from the optimal distance.
 */
#define TANGO_MIN_CUTOFF_DISTANCE 0.1   /* units: meters */
#define TANGO_MIN_GOOD_DISTANCE   0.5   /* units: meters */
#define TANGO_BEST_DISTANCE       1.0   /* units: meters */
#define TANGO_MAX_GOOD_DISTANCE   4.0   /* units: meters */
#define TANGO_MAX_CUTOFF_DISTANCE 6.0   /* units: meters */

/* these offsets define a base level std. dev. given to every point.
 *
 * This base changes with respect to which operating zone a point is in,
 * whether it is in the optimal operating distance (i.e. "good") or
 * not (i.e. "bad") */
#define TANGO_MIN_STD_GOOD        0.001 /* units: meters */
#define TANGO_MIN_STD_BAD         0.05  /* units: meters */

/* these slopes indicate how much increase a point's estimated
 * standard deviation receives based on how it deviates from the
 * tango's optimum operating distance */
#define TANGO_STD_SLOPE_GOOD      0.01  /* units: meters */
#define TANGO_STD_SLOPE_BAD       0.02  /* units: meters */

/* the std. dev. to use when a point is in the "good distance" range */
#define TANGO_STD_FOR_GOOD_DIST(d) ( TANGO_MIN_STD_GOOD + \
		(abs((d) - TANGO_BEST_DISTANCE) * TANGO_STD_SLOPE_GOOD ) )

/* the std. dev. to use when a point is in the "bad distance" range */
#define TANGO_STD_FOR_BAD_DIST(d)  ( TANGO_MIN_STD_BAD + \
		(abs((d) - TANGO_BEST_DISTANCE) * TANGO_STD_SLOPE_BAD ) )


/* helper functions */
int export_fss(const filter_tango_scans_run_settings_t& args,
			tango_reader_t& infile);
int export_mad(const filter_tango_scans_run_settings_t& args,
			tango_reader_t& infile); 

/**
 * The main function for the filter_tango_scans program
 */
int main(int argc, char** argv)
{
	filter_tango_scans_run_settings_t args;
	tango_reader_t infile;
	tictoc_t clk;
	int ret;

	/* read command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse */
		cerr << "[main]\tCan't parse command-line, Error " 
		     << ret << endl;
		return 1;
	}
	
	/* open input file */
	tic(clk);
	ret = infile.open(args.tangofile);
	if(ret)
	{
		cerr << "[export_fss]\tcan't read infile." << endl;
		return -1;
	}
	toc(clk, "Parsing file");
	
	/* export to fss outfile, if specified */
	ret = export_fss(args, infile);
	if(ret)
	{
		cerr << "[main]\tUnable to export fss file, Error "
		     << ret << endl;
		return 2;
	}

	/* export to mad outfile, if specified */
	ret = export_mad(args, infile);
	if(ret)
	{
		cerr << "[main]\tUnable to export mad file, Error "
		     << ret << endl;
		return 3;
	}

	/* success */
	infile.close();
	return 0;
}

int export_fss(const filter_tango_scans_run_settings_t& args,
				tango_reader_t& infile)
{
	SyncXml syncfile;
	FitParams timesync;
	fss::writer_t outfile;
	fss::frame_t  outframe;
	tango_frame_t inframe;
	progress_bar_t progbar;
	tictoc_t clk;
	double dist;
	size_t i, i_start, i_end, j, n;
	int ret;

	/* first, check if an output file is specified */
	if(args.fss_outfile.empty())
		return 0;
	
	/* determine the number of frames we're exporting */
	tic(clk);
	progbar.set_name("Writing fss");
	i_start = (args.begin_idx < 0 ? 0 : args.begin_idx);
	i_end   = (args.end_idx < 0 
			|| args.end_idx >= (int)infile.num_frames()
			? infile.num_frames() : args.end_idx);	
	n = i_end - i_start;

	/* get the time synchronization for this sensor */
	if(!(syncfile.read(args.timefile)))
	{
		/* unable to read file */
		cerr << "[export_mad]\tUnable to import time sync file: "
		     << args.timefile << endl << endl;
		return -1;
	}
	if(!(syncfile.isMember(args.sensor_name)))
	{
		/* not a valid sensor */
		cerr << "[export_mad]\tCould not find timesync for sensor: "
		     << args.sensor_name << endl << endl;
		return -2;
	}
	timesync = syncfile.get(args.sensor_name);

	/* attempt to open the output file for writing */
	outfile.init(args.sensor_name, /* unique name of sensor on system */
		"Google_Tango", /* type of sensor */
		n,  /* number of scan frames */
		-1, /* variable number of points per frame */
		fss::UNITS_METERS); /* scans given in meters */
	ret = outfile.open(args.fss_outfile);
	if(ret)
	{
		cerr << "[export_fss]\tUnable to open output .fss file: "
		     << args.fss_outfile << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* iterate through scan frames */
	for(i = i_start; i < i_end; i++)
	{
		/* update user on status */
		progbar.update(i - i_start, i_end - i_start);

		/* read in the tango frame */
		ret = infile.get(i, inframe);
		if(ret)
		{
			cerr << "[export_fss]\tUnable to import frame "
			     << " #" << i << " of tango data file."
			     << endl;
			return PROPEGATE_ERROR(-2, ret);
		}

		/* copy info to fss frame */
		outframe.timestamp = timesync.convert(inframe.timestamp);
		outframe.points.resize(inframe.points.size());
		for(j = 0; j < inframe.points.size(); j++)
		{
			/* put the tango points into more reasonable
			 * coordinate system */
			outframe.points[j].x =  inframe.points[j].x;
			outframe.points[j].y =  inframe.points[j].y;
			outframe.points[j].z =  inframe.points[j].z;
			
			/* assume no intensity */
			outframe.points[j].intensity = 0;
		
			/* compute the distance from the scan point
			 * to the device.  This value is used to
			 * determine the accuracy of the scans */
			dist = sqrt(
				 (inframe.points[j].x * inframe.points[j].x)
				+(inframe.points[j].y * inframe.points[j].y)
				+(inframe.points[j].z * inframe.points[j].z)
				);

			/* check that this distance is within reasonable
			 * range */
			if(dist < TANGO_MIN_CUTOFF_DISTANCE 
					|| dist > TANGO_MAX_CUTOFF_DISTANCE)
			{
				/* way outside of valid range,
				 * give it infinite error */
				outframe.points[j].stddev = DBL_MAX;
			}
			else if(dist < TANGO_MIN_GOOD_DISTANCE
					|| dist > TANGO_MAX_GOOD_DISTANCE)
			{
				/* outside of optimum range, give
				 * it more penalized std. dev. */
				outframe.points[j].stddev
					= TANGO_STD_FOR_BAD_DIST(dist);
			}
			else
			{
				/* inside optimum operating range, give it
				 * an optimistic std. dev. */
				outframe.points[j].stddev 
					= TANGO_STD_FOR_GOOD_DIST(dist);
			}

			/* the following values are based on the
			 * statistics of the sensor */
			outframe.points[j].bias = 0; /* no bias */
			outframe.points[j].width = 0; /* no width for now */
		}

		/* export fss frame */
		ret = outfile.write(outframe);
		if(ret)
		{
			cerr << "[export_fss]\tUnable to export frame "
			     << " #" << i << " to fss output file: \""
			     << args.fss_outfile << "\"" << endl;
			return PROPEGATE_ERROR(-3, ret);
		}
	}

	/* success */
	outfile.close();
	progbar.clear();
	toc(clk, "Exporing fss file");
	/* the infile should remain open since it was an argument */
	return 0;
}

int export_mad(const filter_tango_scans_run_settings_t& args,
				tango_reader_t& infile)
{
	SyncXml syncfile;
	FitParams timesync;
	tango_frame_t frame;
	system_path_t path;
	pose_t p;
	progress_bar_t progbar;
	tictoc_t clk;
	size_t i, i_start, i_end;
	int ret;

	/* first, check if an output file is specified */
	if(args.mad_outfile.empty())
		return 0;

	/* resize the path to have correct number of poses */
	tic(clk);
	progbar.set_name("Writing mad");
	i_start = (args.begin_idx < 0 ? 0 : args.begin_idx);
	i_end   = (args.end_idx < 0 
			|| args.end_idx >= (int)infile.num_frames()
			? infile.num_frames() : args.end_idx);
	path.resize(i_end - i_start);
	
	/* get the time synchronization for this sensor */
	if(!(syncfile.read(args.timefile)))
	{
		/* unable to read file */
		cerr << "[export_mad]\tUnable to import time sync file: "
		     << args.timefile << endl << endl;
		return -1;
	}
	if(!(syncfile.isMember(args.sensor_name)))
	{
		/* not a valid sensor */
		cerr << "[export_mad]\tCould not find timesync for sensor: "
		     << args.sensor_name << endl << endl;
		return -2;
	}
	timesync = syncfile.get(args.sensor_name);

	/* populate the frames */
	for(i = i_start; i < i_end; i++)
	{
		/* update user */
		progbar.update(i - i_start, i_end - i_start);

		/* get the next frame */
		ret = infile.get(i, frame);
		if(ret)
		{
			/* i/o error */
			cerr << "[export_mad]\tUnable to get frame #"
			     << i << " of " << infile.num_frames() << endl;
			return PROPEGATE_ERROR(-1, ret);
		}
	
		/* populate a pose based on this information */
		p.timestamp = timesync.convert(frame.timestamp);
		p.T(0)  = frame.position[0];
		p.T(1)  = frame.position[1];
		p.T(2)  = frame.position[2];

		p.R = Eigen::Quaternion<double>(
				frame.quaternion[3],  /* w */
				frame.quaternion[0],  /* x */
				frame.quaternion[1],  /* y */
				frame.quaternion[2]); /* z */

		/* insert pose into path */
		path.set(i - i_start, p);
	}

	/* export path */
	ret = path.writemad(args.mad_outfile);
	if(ret)
	{
		/* export error */
		cerr << "[export_mad]\tUnable to write output mad file \""
		     << args.mad_outfile << "\".  Error " << ret << endl;
		return PROPEGATE_ERROR(-2, ret);
	}
	
	/* success */
	progbar.clear();
	toc(clk, "Exporting mad file");
	return 0;
}
