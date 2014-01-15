#include "export_data.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "../io/config.h"
#include "../structs/histogram.h"
#include "../structs/point.h"
#include "../structs/color.h"
#include "../structs/pointcloud.h"
#include "../util/error_codes.h"
#include "../util/progress_bar.h"
#include "../util/tictoc.h"
#include "../util/parameters.h"

using namespace std;

int export_data(vector<double>& floor_heights, vector<double>& ceil_heights,
		histogram_t& floor_hist, histogram_t& ceil_hist,
		config_t& conf)
{
	tictoc_t clk;
	int ret;

	/* begin clock */
	tic(clk);

	/* check if we should write a matlab script contain our results */
	if(conf.matlab_outfile)
	{
		ret = export_matlab_script(floor_heights, ceil_heights,
					floor_hist, ceil_hist, conf);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* check if we should export points to new xyz files */
	if(conf.outfile)
	{
		ret = partition_scans(floor_heights, ceil_heights, conf);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	toc(clk, "Exporting data");
	return 0;
}

int export_matlab_script(vector<double>& floor_heights, 
			vector<double>& ceil_heights,
			histogram_t& floor_hist, histogram_t& ceil_hist,
			config_t& conf)
{
	histogram_t joint_hist;
	ofstream outfile;
	int i, n, ret;

	/* check valid levels */
	if(floor_heights.size() != ceil_heights.size())
		return -1;

	/* check if matlab file is valid */
	if(conf.matlab_outfile == NULL)
		return -2;

	/* open matlab script for writing */
	outfile.open(conf.matlab_outfile);
	if(!(outfile.is_open()))
		return -3;

	/* add header stuff */
	outfile << "close all;" << endl
	        << "clear all;" << endl
		<< "clc;" << endl << endl;

	/* create a joint histogram of floor and ceiling points */
	joint_hist.set_resolution(conf.res);
	joint_hist.insert(floor_hist);
	joint_hist.insert(ceil_hist);

	/* prepare figure */
	outfile << ((char) 0x25)
	        << " Prepare figure" << endl
		<< "figure(1);" << endl
		<< "hold all;" << endl
		<< endl;

	/* export max bin count */
	outfile << ((char) 0x25)
	        << " The following is the largest bin count"
		<< endl
		<< "m = " << joint_hist.count(joint_hist.max()) << ";"
		<< endl << endl;

	/* export the level ranges to matlab */
	outfile << ((char) 0x25) 
	        << " The following are level ranges"
	        << endl;
	n = floor_heights.size();
	for(i = 0; i < n; i++)
	{
		/* write range for this level */
		outfile << "L" << i << " = [" 
		        << floor_heights[i] << ", " << ceil_heights[i]
			<< "];" << endl
			<< "patch("
			<< "m * [1 1 0 0], "
			<< "[L" << i << ", fliplr(L" << i << ")], "
			<< "[" << ((rand()%15)*0.01 + 0.8)
			<< " " << ((rand()%15)*0.01 + 0.8)
			<< " " << ((rand()%15)*0.01 + 0.8) << "], "
			<< "'EdgeColor', 'none');" << endl;
	}
	outfile << endl;

	/* write histogram to file */
	ret = joint_hist.export_to_matlab(outfile, true);
	if(ret)
	{
		outfile.close();
		return PROPEGATE_ERROR(-4, ret);
	}

	/* annotate plot */
	outfile << ((char) 0x25) << " Annotate plot" << endl
	        << "title('Height histogram of building scans', "
		<< "'Fontsize', 18);" << endl
		<< "xlabel('Point count', 'FontSize', 14);" << endl
		<< "ylabel('Height (m), bin size of " << conf.res 
		<< " meters', 'Fontsize', 14);" << endl
		<< "legend(";
	for(i = 0; i < n; i++)
		outfile << "'Level " << i << "', ";
	outfile << "'Point histogram');" << endl << endl;

	/* success */
	return 0;
}

int partition_scans(vector<double>& floor_heights, 
			vector<double>& ceil_heights,
			config_t& conf)
{
	progress_bar_t prog_bar;
	pointcloud_writer_t* outfiles;
	stringstream ss;
	path_t path;
	scanner_t scanner;
	scan_t scan;
	vector<point_t>::iterator it;
	pointcloud_reader_t pcr;
	point_t p;
	color_t c;
	int sn, ser;
	double timestamp;
	int i, level, num_levels, num_xyz, num_msd, pi, ret;

	/* check validity of levels */
	if(floor_heights.size() != ceil_heights.size())
		return -1;

	/* load the path */
	ret = path.readmad(conf.mad_infile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* open output pointcloud xyz files for each level */
	num_levels = floor_heights.size();
	outfiles = new pointcloud_writer_t[num_levels]; 
	for(i = 0; i < num_levels; i++)
	{
		/* form name of file to write for this level */
		ss.str("");
		ss << conf.outfile << i << ".xyz";

		/* open file for writing */
		ret = outfiles[i].open(ss.str().c_str(), 
				XYZ_DEFAULT_UNITS);
		if(ret)
			return PROPEGATE_ERROR(-3, 
				PROPEGATE_ERROR(-i, ret));
	}

	/* iterate through input files */
	prog_bar.set_name("Partitioning scans");

	/* first read xyz files */
	num_xyz = conf.xyz_infiles.size();
	num_msd = conf.msd_infiles.size();
	for(i = 0; i < num_xyz; i++)
	{
		/* update progress bar */
		prog_bar.update(i, num_xyz + num_msd);

		/* iterate through current file */
		pcr.open(conf.xyz_infiles[i], XYZ_DEFAULT_UNITS); 
		while(!pcr.eof())
		{
			/* get next point */
			ret = pcr.next_point(p, c, sn, timestamp, ser);
			if(ret)
			{
				/* check nature of error */
				if(pcr.eof())
					break;
				return PROPEGATE_ERROR(-4, ret);
			}

			/* get corresponding pose for this point */
			pi = path.closest_index(timestamp);

			/* partition point */
			level = level_of_point(p, floor_heights, 
						ceil_heights);
			ret = outfiles[level].write_point(p, c, sn, 
						timestamp, ser);
			if(ret)
				return PROPEGATE_ERROR(-5, ret);
		}
	}

	/* iterate through msd files */
	c.set(255, 255, 255);
	for(i = 0; i < num_msd; i++)
	{
		/* iterate through file */
		scanner.open_msd(conf.msd_infiles[i]);
		while(!scanner.eof())
		{
			/* update progress bar */
			prog_bar.update(
				(num_xyz + i + scanner.amount_read())
				/ (1.0 * (num_xyz + num_msd)));
	
			/* get next point */
			ret = scanner.next_scan(scan);
			if(ret)
			{
				/* check nature of error */
				if(scanner.eof())
					break;
				return PROPEGATE_ERROR(-6, ret);
			}

			/* get corresponding pose for this point */
			pi = path.closest_index(scan.get_timestamp());
			scan.transform_from_pose(path.pl[pi]);

			/* iterate through points in scan */
			for(it=scan.pts.begin(); it!=scan.pts.end(); it++)
			{
				/* partition point */
				level = level_of_point(*it, floor_heights, 
							ceil_heights);
				ret = outfiles[level].write_point(*it, 
					c, scan.get_scan_num(),
					scan.get_timestamp(), 
					scan.get_serial_number());
				if(ret)
					return PROPEGATE_ERROR(-7, ret);
			}
		}
		scanner.close();
	}

	/* success */
	prog_bar.clear();
	delete[] outfiles;
	return 0;
}

int level_of_point(point_t& p, vector<double>& floor_heights, 
				vector<double>& ceil_heights)
{
	int i, n;
	double z, thresh;

	/* get number of levels */
	n = floor_heights.size();
	if(n <= 0)
		return -1;
	
	/* check first level */
	z = p.get(2);
	if(z < ceil_heights[0])
		return 0; /* at or below first floor */

	/* iterate over middle levels */
	for(i = 1; i < n; i++)
	{
		/* compute cut-off threshold for level i-1 */
		thresh = 0.5 * (ceil_heights[i-1] + floor_heights[i]);

		/* check point */
		if(z < thresh)
			return i-1; /* closest to this level */
	}

	/* must be at top level */
	return (n-1);
}
