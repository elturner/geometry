#include "pose_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../structs/pose.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

/* number of bytes used in file to define elements */
#define ZUPT_ELEMENT_SIZE 2
#define POSE_ROTATION_ELEMENT_SIZE 3

int readmad(char* filename, vector<pose_t>& pl)
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

		/* calculate trig functions of these angles */
		p.cr = cos(p.roll);
		p.sr = sin(p.roll);
		p.cp = cos(p.pitch);
		p.sp = sin(p.pitch);
		p.cy = cos(p.yaw);
		p.sy = sin(p.yaw);

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
