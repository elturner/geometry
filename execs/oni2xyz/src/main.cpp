#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <OpenNI.h>
#include <Eigen/Dense>
#include <util/error_codes.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/progress_bar.h>
#include "log_reader.h"

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This program converts from .oni format files to .xyz pointclouds
 *
 * @section DESCRIPTION
 *
 * This is the main file for the oni2xyz program.  This program will take
 * as input a .oni file, which is used to store PrimeSense depth scans.
 */

using namespace std;
using namespace openni;
using namespace Eigen;

/* command-line flags */

#define LOG_FILE_EXT "log" /* kinect path file */
#define ONI_FILE_EXT "oni" /* kinect depth frames */
#define XYZ_FILE_EXT "xyz" /* output pointcloud */

/* hard-coded constants */

#define MIN_VALID_DEPTH 50   /* units: millimeters */
#define MAX_VALID_DEPTH 3000 /* units: millimeters */
#define DOWNSAMPLE_RATE 100  /* units: pixels */
#define US2SECONDS(X)  ( (X) / 1000000.0 ) /* microseconds to seconds */
#define METERS2MM(X)   ( (X) * 1000.0 ) /* meters to millimeters */

/* helper function headers */

/**
 * Will initialize the command-line arguments structure
 *
 * @param args  The command-line arguments structure to initialize
 */
void init(cmd_args_t& args);

/**
 * Exports a ONI frame to the given xyz stream
 *
 * Given a single frame from the imported ONI file, will
 * export this frame to the specified XYZ file.
 *
 * @param frame    The frame to export
 * @param f        Index of frame
 * @param logread  The trajectory log associated with this frame
 * @param os       The output xyz stream to export to
 *
 * @return         Returns zero on success, non-zero on failure
 */
int export_frame(openni::VideoFrameRef& frame, size_t f,  
		const log_reader_t& logread, std::ostream& os);

/* implementation of main function */

int main(int argc, char** argv)
{
	cmd_args_t args;
	vector<string> logfiles;
	vector<string> onifiles;
	vector<string> xyzfiles;
	ofstream outfile;
	openni::Device dev;
	openni::VideoStream instream;
	openni::VideoFrameRef frame;
	openni::Status ret;
	log_reader_t logread;
	progress_bar_t progbar;
	uint64_t ts_curr, ts_prev;
	size_t i, n, f;
	tictoc_t clk;
	int r;

	/* initialize command-line arguments */
	tic(clk);
	init(args);
	r = args.parse(argc, argv);
	if(r)
	{
		/* unable to parse command-line */
		cerr << "[main]\tError " << r << ": "
		     << "Unable to parse command-line" << endl;
		return 1;
	}

	/* initialize drivers */
	ret = OpenNI::initialize();
	switch(ret)
	{
		/* check for allowable return values */
		case STATUS_OK:
		case STATUS_NO_DEVICE:
			break;

		/* check for errors */
		case STATUS_TIME_OUT:
		case STATUS_ERROR:
		case STATUS_NOT_IMPLEMENTED:
		case STATUS_NOT_SUPPORTED:
		case STATUS_BAD_PARAMETER:
		case STATUS_OUT_OF_FLOW:
			cerr << "[main]\tUnable to load OpenNI drivers.  "
			     << "Return value: " << ret << endl;
			return 2;
	}

	/* attempt to open the given files */
	args.files_of_type(LOG_FILE_EXT, logfiles);
	args.files_of_type(ONI_FILE_EXT, onifiles);
	args.files_of_type(XYZ_FILE_EXT, xyzfiles);
	n = onifiles.size();
	if(n != xyzfiles.size() || n != logfiles.size())
	{
		cerr << "[main]\tDifferent number of input and output "
		     << "files given. Please give same number of inputs "
		     << "and outputs." << endl;
		return 3;
	}
	toc(clk, "Initializing");
	
	/* iterate through files */
	for(i = 0; i < n; i++)
	{
		/* display status to user */
		tic(clk);
		cout << endl
		     << "Converting: " << onifiles[i] << endl
		     << "to:         " << xyzfiles[i] << endl; 
		
		/* open input data file */
		ret = dev.open(onifiles[i].c_str());
		if(ret != STATUS_OK)
		{
			cerr << "[main]\tUnable to open oni file" << endl;
			return 4;
		}

		/* open input log file */
		r = logread.parse(logfiles[i]);
		if(r)
		{
			cerr << "[main]\tUnable to parse log file" << endl;
			return 5;
		}
	
		/* open the video stream for this file */
		ret = instream.create(dev, SENSOR_DEPTH);
		if(ret != STATUS_OK)
		{
			cerr << "[main]\tCannot create stream" << endl;
			return 6;
		}
		ret = instream.start();
		if(ret != STATUS_OK)
		{
			cerr << "[main]\tCannot start stream" << endl;
			return 7;
		}

		/* open output file */
		outfile.open(xyzfiles[i].c_str());
		if(!(outfile.is_open()))
		{
			cerr << "[main]\tUnable to open xyz file" << endl;
			return 8;
		}

		/* iterate over the frames of this stream */
		ts_curr = ts_prev = 0;
		f = 0;
		progbar.set_name("Converting");
		while(true)
		{
			/* update progress bar */
			progbar.update();

			/* get next frame */
			ret = instream.readFrame(&frame);
			if(ret != STATUS_OK)
			{
				progbar.clear();
				cerr << "[main]\tUnable "
				     << "to read next frame!" << endl;
				break;
			}
		
			/* check if frame is valid */
			if(!(frame.isValid()))
			{
				progbar.clear();
				cerr << "[main]\tInvalid frame #" 
				     << f << endl;
				break;
			}

			/* check if we've reached the end of the file,
			 * by checking that the timestamps are monotonic */
			ts_curr = frame.getTimestamp();
			if(ts_curr < ts_prev)
				break;

			/* export this frame */
			r = export_frame(frame, f, logread, outfile);
			if(r)
			{
				progbar.clear();
				cerr << "[main]\tUnable to export Frame "
				     << "#" << f << endl;
				break;
			}

			/* we no longer need this frame */
			frame.release();
			ts_prev = ts_curr; /* keep track of timestamps */
			f++; /* increment frame counter */
		}

		/* cleanup */
		instream.stop();
		instream.destroy();
		dev.close();
		outfile.close();
		progbar.clear();
		toc(clk, "Converting files");
	}

	/* success */
	OpenNI::shutdown();
	return 0;
}

void init(cmd_args_t& args)
{
	args.set_program_description("This program will convert from "
			".oni files to .xyz files.  The ONI file format "
			"is used to represent PrimeSense Depth Scans, and "
			"was developed for the OpenNI library.  The XYZ "
			"format is a basic point-cloud representation.");
	args.add_required_file_type(LOG_FILE_EXT, 1, "Represents the path "
			"of the input sensor.  For formatting details, see:"
			"\n\nhttp://web.stanford.edu/~qianyizh/projects/"
			"scenedata.html");
	args.add_required_file_type(ONI_FILE_EXT, 1, "Represents the input "
			"file to convert.");
	args.add_required_file_type(XYZ_FILE_EXT, 1, "Represents the "
			"output file to write.  Must be equal number of "
			"input and output files.");
}

int export_frame(openni::VideoFrameRef& frame, size_t f,
		const log_reader_t& logread, std::ostream& os)
{
	size_t i, num_pixels, u, v, w;
	const unsigned short* buf;
	Vector3d p;
	double ts;
	int ret;

	/* check that pixel format is PIXEL_FORMAT_DEPTH_1_MM */
	if(frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM)
	{
		cerr << "[export_frame]\tFrame #" << f << ": "
		     << "Invalid pixel format: "
		     << frame.getVideoMode().getPixelFormat() << endl;
		return -1;
	}

	/* iterate over pixels of this depth frame. */
	w = frame.getWidth();
	num_pixels = w * frame.getHeight();
	buf = (const unsigned short*) frame.getData();
	for(i = 0; i < num_pixels; i++)
	{
		/* downsample each frame spatially */
		if(i % DOWNSAMPLE_RATE != 0)
			continue;

		/* check if depth is valid */
		if(buf[i] < MIN_VALID_DEPTH || buf[i] > MAX_VALID_DEPTH)
			continue; /* ignore this one */

		/* determine (u,v) coordinates of this pixel, assuming
		 * row-major order */
		u = i % w;
		v = i / w; 

		/* compute the 3D point */
		ret = logread.compute_point(f, u, v, (double) buf[i], p);
		if(ret)
		{
			cerr << "[export_frame]\tFrame #" << f << ": "
			     << "Cannot compute point #" << i << endl;
			return PROPEGATE_ERROR(-2, ret);
		}

		/* convert timestamp to seconds from microseconds */
		ts = US2SECONDS(frame.getTimestamp());

		/* store the point in the output stream
		 *
		 * Format:
		 *
		 *  x y z r g b id timestamp serial
		 *
		 * Where:
		 *
		 *  Distances are in millimeters,
		 *  -z is the direction of gravity
		 */
		os << METERS2MM( p(2)) << " " 
		   << METERS2MM(-p(0)) << " " 
		   << METERS2MM(-p(1)) << " "
		   << "255 255 255 " << f << " " << ts << " 0" << endl;
	}

	/* success */
	return 0;
}
