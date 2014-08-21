#include "point_io.h"

#include <istream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "../structs/point.h"
#include "../structs/pose.h"
#include "../util/progress_bar.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

int readxyz(char* filename, vector<point_t>& pts)
{
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	point_t p;
	int ret, r, g, b, seriel, id;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading */
	infile.open(filename);
	if(!infile.is_open())
		return -2;

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
			&(p.x), &(p.y), &(p.z), &(r), &(g), &(b),
			&(id), &(p.timestamp), &(seriel));

		/* check format */
		if(ret != NUM_ELEMENTS_PER_LINE)
			continue; /* bad line */

		/* convert units to meters */
		p.x = MM2METERS(p.x);
		p.y = MM2METERS(p.y);
		p.z = MM2METERS(p.z);

		/* add point to list */
		pts.push_back(p);
	}

	/* clean up */
	infile.close();
	return 0;
}

int readxyz_to_pose(char* filename, vector<pose_t>& pl,
				boundingbox_t& bbox,
				point_t& laser_pos,
				int downsample_rate,
				double range_limit_sq)
{
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	point_t p;
	int ret, i, n, r, g, b, seriel, id, num_points_read;
	double d;

	/* check arguments */
	if(!filename || downsample_rate <= 0)
		return -1;

	/* open file for reading */
	infile.open(filename);
	if(!infile.is_open())
		return -2;

	/* add a new scan list to every pose */
	n = pl.size();
	for(i = 0; i < n; i++)
	{
		/* make room for a new scan at this pose */
		pl[i].scans.resize(pl[i].scans.size() + 1);

		/* record the laser's position at this pose */
		pose_transform_local_to_world_coords(pl[i], p, laser_pos);
		pl[i].laser_pos.push_back(p);
	}

	/* read lines of points from file */
	num_points_read = 0;
	while(!infile.eof())
	{
		/* read next line */
		infile.getline(buf, LINE_BUFFER_SIZE);

		/* parse line to make sure it is valid */
		if(strlen(buf) < 2*NUM_ELEMENTS_PER_LINE - 1)
			continue; /* bad line */

		/* assume this line defines a scan point */
		ret = sscanf(buf, XYZ_FORMAT_STRING,
			&(p.x), &(p.y), &(p.z), &(r), &(g), &(b),
			&(id), &(p.timestamp), &(seriel));

		/* check format */
		if(ret != NUM_ELEMENTS_PER_LINE)
			continue; /* bad line */

		/* check if we should store this point */
		num_points_read++;
		if(num_points_read % downsample_rate != 0)
			continue;

		/* convert units to meters */
		p.x = MM2METERS(p.x);
		p.y = MM2METERS(p.y);
		p.z = MM2METERS(p.z);

		/* determine which pose this point is associated with */
		i = poselist_closest_index(pl, p.timestamp);
		n = pl[i].scans.size() - 1;
		
		/* get distance of scan point from pose */
		d = pose_point_dist_sq(pl[i], p);
		if(d > range_limit_sq)
		{
			/* shorten length of scan to be max distance.
			 * This is valid only because voxels that contain
			 * scan points are considered to be 'empty' */
			d = sqrt( range_limit_sq / d ); 
			p.x = (p.x - pl[i].x)*d + pl[i].x;
			p.y = (p.y - pl[i].y)*d + pl[i].y;
			p.z = (p.z - pl[i].z)*d + pl[i].z;
		}	
	
		/* add point to pose scans */
		pl[i].scans[n].push_back(p);
		
		/* update bounding box */
		boundingbox_update(bbox, p);
	}

	/* clean up */
	infile.close();
	return 0;
}

int readxyz_subset_to_pose(char* filename, streampos start, streampos end,
				vector<pose_t>& pl, boundingbox_t& bbox,
				point_t& laser_pos,
				int downsample_rate,
				double range_limit_sq)
{
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	point_t p;
	int ret, s, i, n, r, g, b, seriel, id, num_points_read;
	double d;

	/* check arguments */
	if(!filename || downsample_rate <= 0)
		return -1;

	/* open file for reading */
	infile.open(filename, ios::binary);
	if(!infile.is_open())
		return -2;

	/* seek to starting position */
	infile.seekg(start);

	/* verify this is a valid position */
	if(!(infile.good()))
		return -3;

	/* show user the progress */
	reserve_progress_bar();
	s = (int) ( end - start );

	/* add a new scan list to every pose */
	n = pl.size();
	for(i = 0; i < n; i++)
	{
		/* make room for a new scan at this pose */
		pl[i].scans.resize(pl[i].scans.size() + 1);

		/* record the laser's position at this pose */
		pose_transform_local_to_world_coords(pl[i], p, laser_pos);
		pl[i].laser_pos.push_back(p);
	}

	/* read lines of points from file */
	num_points_read = 0;
	while(!infile.eof() && ((long int)(end - infile.tellg()) > 0))
	{
		/* user is impatient, wants to know status */
		if(num_points_read % 100000 == 0)
			progress_bar("parsing file", 
				((double) (infile.tellg() - start)) / s);

		/* read next line */
		infile.getline(buf, LINE_BUFFER_SIZE);

		/* parse line to make sure it is valid */
		if(strlen(buf) < 2*NUM_ELEMENTS_PER_LINE - 1)
			continue; /* bad line */

		/* assume this line defines a scan point */
		ret = sscanf(buf, XYZ_FORMAT_STRING,
			&(p.x), &(p.y), &(p.z), &(r), &(g), &(b),
			&(id), &(p.timestamp), &(seriel));

		/* check format */
		if(ret != NUM_ELEMENTS_PER_LINE)
			continue; /* bad line */

		/* check if we should store this point */
		num_points_read++;
		if(num_points_read % downsample_rate != 0)
			continue;

		/* convert units to meters */
		p.x = MM2METERS(p.x);
		p.y = MM2METERS(p.y);
		p.z = MM2METERS(p.z);

		/* determine which pose this point is associated with */
		i = poselist_closest_index(pl, p.timestamp);
		n = pl[i].scans.size() - 1;
		
		/* get distance of scan point from pose */
		d = pose_point_dist_sq(pl[i], p);
		if(d > range_limit_sq)
		{
			/* shorten length of scan to be max distance.
			 * This is valid only because voxels that contain
			 * scan points are considered to be 'empty' */
			d = sqrt( range_limit_sq / d ); 
			p.x = (p.x - pl[i].x)*d + pl[i].x;
			p.y = (p.y - pl[i].y)*d + pl[i].y;
			p.z = (p.z - pl[i].z)*d + pl[i].z;
		}	
	
		/* add point to pose scans */
		pl[i].scans[n].push_back(p);
		
		/* update bounding box */
		boundingbox_update(bbox, p);
	}

	/* clean up */
	delete_progress_bar();
	
	infile.close();
	return 0;
}

int readxyz_index_scans(char* filename, vector<streampos>& sssp)
{	
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	int ret, r, g, b, seriel, id, prev_id;
	double x, y, z, timestamp, prev_timestamp;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading */
	infile.open(filename, ios::binary);
	if(!infile.is_open())
		return -2;

	/* clear output */
	sssp.clear();

	/* add start of file to list */
	sssp.push_back(infile.tellg());

	/* read lines of points from file */
	prev_id = -1;
	prev_timestamp = -DBL_MAX;
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
		{
			fprintf(stderr, "[readxyz_index_scans]"
				"\tbad line: %s\n", buf);
			continue; /* bad line */
		}

		/* check that order is preserved */
		if(id < prev_id || timestamp < prev_timestamp)
			return -3; /* out of order */

		/* if this line represents a new scan id,
		 * then record in vector */
		if(id > prev_id)
		{
			sssp.push_back(infile.tellg());
			prev_id = id;
			prev_timestamp = timestamp;
		}
	}

	/* add end-of-file to list */
	sssp.push_back(infile.tellg());

	/* clean up */
	infile.close();
	return 0;
}
