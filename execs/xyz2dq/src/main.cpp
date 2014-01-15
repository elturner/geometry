/* main.cpp:
 *
 *	Stores wall samples of the input Point-cloud files
 *	in the specified DQ-file.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

#include "structs/pose.h"
#include "structs/quadtree.h"
#include "structs/point.h"
#include "structs/normal.h"
#include "io/config.h"
#include "io/pose_io.h"
#include "util/error_codes.h"
#include "util/parameters.h"

using namespace std;

int main(int argc, char** argv)
{
	vector<pose_t> path;
	quadtree_t dq;
	config_t conf;
	unsigned int i;
	int ret;
	ifstream infile;
	ofstream outfile;
	char buf[LINE_BUFFER_SIZE];
	point_t p;
	double x, y, z, timestamp;
	int r, g, b, seriel, id, pose_index;

	/* read command-line args */
	if(parseargs(argc, argv, conf))
	{
		print_usage_short(argv[0]);
		return 1;
	}

	/* initialize quadtree */
	dq.set_resolution(conf.resolution);

	/* import path */
	ret = readmad(conf.mad_infile, path);
	if(ret)
	{
		PRINT_ERROR("unable to read madfile:");
		PRINT_ERROR(conf.mad_infile);
		return 1;
	}

	/* iterate through all input pointcloud files */
	for(i = 0; i < conf.num_pc_files; i++)
	{
		/* open file for reading */
		infile.open(conf.pc_infile[i]);
		if(!infile.is_open())
		{
			PRINT_ERROR("unable to read point-cloud:");
			PRINT_ERROR(conf.pc_infile[i]);
			continue;
		}

		/* read lines of points from file */
		while(!infile.eof())
		{
			/* read next line */
			infile.getline(buf, LINE_BUFFER_SIZE);

			/* parse line to make sure it is valid */
			if(strlen(buf) < 2*NUM_ELEMENTS_PER_LINE - 1)
				continue; /* bad line */

			/* assume this line defines a scan point */
			ret = sscanf(buf, XYZ_FORMAT_STRING,
				&(x), &(y), &(z), &(r), &(g), &(b),
				&(id), &(timestamp), &(seriel));
	
			/* check format */
			if(ret != NUM_ELEMENTS_PER_LINE)
				continue; /* bad line */

			/* convert units to meters */
			x = MM2METERS(x);
			y = MM2METERS(y);
			z = MM2METERS(z);
	
			/* get pose index corresponding to this point */
			pose_index = poselist_closest_index(path, 
							timestamp);
			if(pose_index < 0)
			{
				/* bad timestamp */
				PRINT_ERROR("bad timestamp");
				continue;
			}

			/* insert point into tree */
			p.set(0, x);
			p.set(1, y);
			dq.insert(p, pose_index, z);
		}

		/* clean up */
		infile.close();
	}

	/* export dq tree */
	outfile.open(conf.outfile);
	if(!(outfile.is_open()))
	{
		PRINT_ERROR("unable to write to outfile");
		return 1;
	}
	dq.print(outfile, conf.min_wall_num_points, conf.min_wall_height);
	outfile.close();

	/* success */
	return 0;
}
