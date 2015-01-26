#include "process_scan.h"
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <float.h>
#include <timestamp/sync_xml.h>
#include <io/data/urg/urg_data_reader.h>
#include <io/data/fss/fss_io.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>
#include <util/error_codes.h>

/**
 * @file process_scan.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains processing functions for filtering
 * urg scans and exporting the results to file.
 */

using namespace std;

/* The following macros describe the statistics of a UTM-30LX.
 *
 * These values were taken from a study on error distributions in scans
 * for various laser range finders:
 *
 * Pomerleau, F., Breitenmoser, A., Liu M., Colas, F., and Siegwart, R.,
 * "Noise Characterization of Depth SEnsors for Surface Inspections",
 * 2012 2nd International Conference on Applied Robotics for the Power
 * Industry (CARPI), ETH Zurich, Switerland, September 11-13, 2012
 *
 * Table II, Anisotropic model for sensor: UTM-30LX
 */
#define UTM_30LX_BIAS_MM       0.0   /* units: millimeters */
#define UTM_30LX_STDDEV_MM     18.0  /* units: millimeters */
#define UTM_30LX_WIDTH_MM(d)   ( (0.0006*(d) + 1.48) )
                                     /* units: mm for input and output */
#define UTM_30LX_MIN_RANGE_MM  500   /* units: millimeters */
#define UTM_30LX_MAX_RANGE_MM  30000 /* units: millimeters */

/* function implementations */

int process_scan(SyncXml& timesync, 
                 const string& infilename,
                 const string& outfilename)
{
	fss::writer_t outfile;
	fss::frame_t fss_frame;
	urg_reader_t infile;
	urg_frame_t urg_frame;
	FitParams timeparams;
	string curr_activity;
	progress_bar_t progbar;
	unsigned int i, j, m, n;
	double x, y, d;
	bool goodpoint;
	tictoc_t clk;
	int ret;

	/* parse the input file */
	ret = infile.open(infilename);
	if(ret)
	{
		cerr << "[process_scan]\tUnable to open file: "
		     << infilename << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* Use header info from file to find time sync params */
	if(!(timesync.isMember(infile.serialNum())))
	{
		/* time sync doesn't reference this scanner */
		cerr << "[process_scan]\tUnable to get time sync for "
		     << infile.serialNum() << ".  Has the scanner name "
		     << "been changed to something other than the serial "
		     << "number?" << endl;
		infile.close();
		return PROPEGATE_ERROR(-2, ret);
	}
	timeparams = timesync.get(infile.serialNum());

	/* output to filtered scan file format (.fss) */
	outfile.init(infile.serialNum(), "laser", infile.numScans(),
	                 infile.pointsPerScan(), fss::UNITS_MILLIMETERS);
	ret = outfile.open(outfilename);
	if(ret)
	{
		cerr << "[process_scan]\tUnable to open output file"
		     << ": " << outfilename << "   Error " << ret
		     << endl;
		infile.close();
		return PROPEGATE_ERROR(-3, ret);
	}

	/* iterate over scans to process */
	n = infile.numScans();
	tic(clk);
	curr_activity = "Processing " + infile.serialNum();
	progbar.set_name(curr_activity);
	for(i = 0; i < n; i++)
	{
		/* get next frame to process */
		ret = infile.next(urg_frame);
		if(ret)
		{
			/* difficulty reading infile */
			infile.close();
			outfile.close();
			return PROPEGATE_ERROR(-4, ret);
		}

		/* update user */
		progbar.update(i, n);
		
		/* synchronize the timestamp of this frame */
		fss_frame.timestamp = timeparams.convert(
		                                 urg_frame.timestamp);
		
		/* transfer urg frame to fss frame */
		m = infile.pointsPerScan();
		fss_frame.points.resize(m);
		for(j = 0; j < m; j++)
		{
			/* process this scan statistics */
			d = urg_frame.range_values[j];
				/* depth or range value */
			x = d * cos(infile.angleMap()[j]);
				/* cartesian x-value */
			y = d * sin(infile.angleMap()[j]);
				/* cartesian y-value */

			/* check that point range is valid */
			goodpoint = (d >= UTM_30LX_MIN_RANGE_MM
					&& d <= UTM_30LX_MAX_RANGE_MM);

			/* copy over the fields to the fss
			 * frame object about point position */
			fss_frame.points[j].x = x;
			fss_frame.points[j].y = y;
			fss_frame.points[j].z = 0.0; /* scan plane flat */
			fss_frame.points[j].intensity = 0; /* no color */

			/* compute statistical info */
			if(goodpoint)
			{
				/* record statistics */
				fss_frame.points[j].bias = 
					UTM_30LX_BIAS_MM;
				fss_frame.points[j].stddev = 
					UTM_30LX_STDDEV_MM;
				fss_frame.points[j].width = 
					UTM_30LX_WIDTH_MM(d);
			}
			else
			{
				/* this is a bad point, so we
				 * should put no confidence in it */
				fss_frame.points[j].bias   = 0;
				fss_frame.points[j].stddev = DBL_MAX;
				fss_frame.points[j].width  = DBL_MAX;
			}
		}

		/* export scans to filtered scan formatted file */
		ret = outfile.write(fss_frame);
		if(ret)
		{
			infile.close();
			outfile.close();
			return PROPEGATE_ERROR(-5, ret);
		}
	}
	progbar.clear();
	toc(clk, curr_activity.c_str());

	/* clean up */
	infile.close();
	outfile.close();
	
	/* success */
	return 0;
}
