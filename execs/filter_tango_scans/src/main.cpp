#include "filter_tango_scans_settings.h"
#include <io/data/tango/tango_io.h>
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>

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
	toc(clk, "Filtering scans");
	return 0;
}

int export_fss(const filter_tango_scans_run_settings_t& args,
				tango_reader_t& infile)
{
	tango_frame_t frame;
	fss::writer_t outfile;
	int ret;

	/* first, check if an output file is specified */
	if(args.fss_outfile.empty())
		return 0;

	// TODO
	
	/* success */
	infile.close();
	return 0;
}

int export_mad(const filter_tango_scans_run_settings_t& args,
				tango_reader_t& infile)
{
	tango_frame_t frame;
	system_path_t path;
	pose_t p;
	size_t i, i_start, i_end;
	int ret;

	/* first, check if an output file is specified */
	if(args.mad_outfile.empty())
		return 0;

	/* resize the path to have correct number of poses */
	path.resize(infile.num_frames());
	i_start = (args.begin_idx < 0 ? 0 : args.begin_idx);
	i_end   = (args.end_idx < 0 
			|| args.end_idx >= (int)infile.num_frames()
			? infile.num_frames() : args.end_idx);

	/* populate the frames */
	for(i = i_start; i < i_end; i++)
	{
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
		p.timestamp = frame.timestamp;
		p.T(0)  = frame.position[0];
		p.T(1)  = frame.position[1];
		p.T(2)  = frame.position[2];
		p.R.x() = frame.quaternion[0];
		p.R.y() = frame.quaternion[1];
		p.R.z() = frame.quaternion[2];
		p.R.w() = frame.quaternion[3];

		/* insert pose into path */
		path.set(i, p);
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
	infile.close();
	return 0;
}
