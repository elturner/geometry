#include "populate_histogram.h"
#include "../io/config.h"
#include "../structs/point.h"
#include "../structs/color.h"
#include "../structs/pointcloud.h"
#include "../structs/histogram.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"
#include "../util/tictoc.h"
#include "../util/progress_bar.h"

using namespace std;

int populate_histogram(histogram_t& floors, histogram_t& ceilings, 
						config_t& conf)
{
	progress_bar_t prog_bar;
	path_t path;
	tictoc_t clk;
	scanner_t scanner;
	scan_t scan;
	vector<point_t>::iterator it;
	pointcloud_reader_t pcr;
	point_t p;
	color_t c;
	int sn, ser;
	double timestamp;
	int i, num_xyz, num_msd, pi, ret;

	/* clear any stored info from the histogram */
	floors.set_resolution(conf.res);
	ceilings.set_resolution(conf.res);

	/* load the path */
	ret = path.readmad(conf.mad_infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* iterate through input files */
	tic(clk);
	prog_bar.set_name("Reading scans");

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
				return PROPEGATE_ERROR(-2, ret);
			}

			/* get corresponding pose for this point */
			pi = path.closest_index(timestamp);

			/* determine if point should be counted as a
			 * "floor" or a "ceiling" point */
			if(path.pl[pi].z < p.get(2))
			{
				/* point is above pose, so assume ceiling */
				ceilings.insert(p.get(2));
			}
			else
			{
				/* point is below pose, so assume floor */
				floors.insert(p.get(2));
			}
		}
	}

	/* iterate through msd files */
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
				return PROPEGATE_ERROR(-3, ret);
			}

			/* get corresponding pose for this point */
			pi = path.closest_index(scan.get_timestamp());
			scan.transform_from_pose(path.pl[pi]);

			/* iterate through points in scan */
			for(it=scan.pts.begin(); it!=scan.pts.end(); it++)
			{
				/* determine if point should be counted as
				 * a "floor" or a "ceiling" point */
				if(path.pl[pi].z < it->get(2))
				{
					/* point is above pose, 
					 * so assume ceiling */
					ceilings.insert(it->get(2));
				}
				else
				{
					/* point is below pose, 
					 * so assume floor */
					floors.insert(it->get(2));
				}
			}
		}
		scanner.close();
	}

	/* report status */
	prog_bar.clear();
	toc(clk, "Reading scans");

	/* success */
	return 0;
}
