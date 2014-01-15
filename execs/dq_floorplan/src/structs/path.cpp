#include "path.h"

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../util/error_codes.h"

using namespace std;

/* number of bytes used in file to define elements */
#define ZUPT_ELEMENT_SIZE 2
#define POSE_ROTATION_ELEMENT_SIZE 3

#ifndef M_PI
#define M_PI 3.14159265358979323846264
#endif
#define DEG2RAD(x) ( (x) * M_PI / 180.0 )

/****** PATH FUNCTIONS ********/

path_t::path_t() {}

path_t::~path_t() {}

int path_t::readmad(char* filename)
{
	FILE* infile;
	unsigned int i, num_zupts, num_poses;
	pose_t p;
	int ret;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading -- binary */
	infile = fopen(filename, "rb");
	if(!infile)
		return -2;
	if(feof(infile))
	{
		/* empty file */
		fclose(infile);
		return -3;
	}

	/* clear vector, so that poses can be added */
	pl.clear();

	/* read zupts header from file */
	ret = fread(&num_zupts, sizeof(unsigned int), 1, infile);
	if(ret != 1)
	{
		fclose(infile);
		return -4;
	}

	/* skip zupt info */
	ret = fseek(infile, num_zupts * ZUPT_ELEMENT_SIZE * sizeof(double), 
							SEEK_CUR);
	if(ret)
	{
		fclose(infile);
		return -5;
	}
	
	/* read number of poses */
	ret = fread(&num_poses, sizeof(unsigned int), 1, infile);
	if(ret != 1)
	{
		fclose(infile);
		return -7;
	}
	
	/* read all poses */
	pl.reserve(num_poses);
	for(i = 0; i < num_poses && !feof(infile); i++)
	{
		/* read pose */
		ret = fread(&(p.timestamp), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -8;
		}
		ret = fread(&(p.x), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -9;
		}
		ret = fread(&(p.y), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -10;
		}
		ret = fread(&(p.z), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -11;
		}
	
		/* read rotation information about the current pose */
		ret = fread(&(p.roll), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -12;
		}
		ret = fread(&(p.pitch), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -13;
		}
		ret = fread(&(p.yaw), sizeof(double), 1, infile);
		if(ret != 1)
		{
			fclose(infile);
			return -14;
		}

		/* convert the orientation information that was just
		 * read into radians in NED coordinates. */
		p.roll = DEG2RAD(p.roll);
		p.pitch = DEG2RAD(p.pitch);
		p.yaw = DEG2RAD(p.yaw);

		/* check that poses are given in order */
		if(i > 0 && pl[i-1].timestamp > p.timestamp)
		{
			fprintf(stderr, "[readmad]\tError:"
					" pose #%d out of order\n", i);
			fclose(infile);
			return -15;
		}
		
		/* store this info */
		pl.push_back(p);
	}

	/* cleanup */
	fclose(infile);
	return 0;
}

int path_t::closest_index(double t)
{
	unsigned int low, high, mid, last;
	double comp;

	/* check arguments */
	if(pl.empty())
		return -1;

	/* check outer bounds */
	if(t < pl[0].timestamp)
		return 0;
	last = pl.size() - 1;
	if(t > pl[last].timestamp)
		return last;

	/* assume poses are in order, and perform binary search */
	low = 0;
	high = pl.size();
	while(low <= high)
	{
		/* check middle of range */
		mid = (low + high)/2;
		comp = pl[mid].timestamp - t;

		/* subdivide */
		if(comp < 0)
			low = mid + 1;
		else if(comp > 0)
		{
			high = mid - 1;
			
			/* check if high is now less than t */
			if(mid != 0 && pl[high].timestamp < t)
			{
				low = high;
				break;
			}
		}
		else
			return mid; /* exactly the right time */
	}

	/* we know t falls between indices low and low+1 */
	if(t - pl[low].timestamp > pl[low+1].timestamp - t)
		return low+1;
	return low;
}

/**** POSE FUNCTIONS ***/
	
pose_t::pose_t() {}

pose_t::~pose_t() {}

double pose_t::dist_sq(pose_t& other)
{
	double x, y, z;

	/* compute distance */
	x = this->x - other.x;
	y = this->y - other.y;
	z = this->z - other.z;
	return x*x + y*y + z*z;
}
