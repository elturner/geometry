#include "voxel_io.h"
#include <fstream>
#include <map>
#include <stdio.h>
#include "../structs/dgrid.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

int readvox(char* filename, dgrid_t& g)
{
	ifstream infile;
	char buf[LINE_BUFFER_SIZE];
	voxel_t v;
	voxel_state_t s;
	int si, ret;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading */
	infile.open(filename);
	if(!infile.is_open())
		return -2;

	/* clear current grid values */
	g.voxels.clear();

	/* get voxel size */
	infile.getline(buf, LINE_BUFFER_SIZE);
	ret = sscanf(buf, "%lf", &(g.vs));
	if(ret != 1)
	{
		infile.close();
		return -3;
	}

	/* read file and populate g */
	while(!infile.eof())
	{
		/* the next line specifies a voxel */
		infile.getline(buf, LINE_BUFFER_SIZE);
		

		ret = sscanf(buf, "%d %d %d %d",
				&(v.x_ind), &(v.y_ind), &(v.z_ind), &si);
		if(ret != 4)
		{
			/* check if at end of file */
			if(infile.eof())
				break;

			/* report error */
			LOGI("[readvox]\tCould not parse: %s\n", buf);
			LOGI("\t\tgot %d values\n", ret);
			infile.close();
			return -4;
		}

		/* store voxel in grid */
		s = (voxel_state_t) si;
		g.voxels.insert(pair<voxel_t, voxel_state_t>(v, s));
	}
	
	/* clean up */
	infile.close();
	return 0;
}

int writevox(char* filename, dgrid_t& g)
{
	ofstream outfile;
	map<voxel_t, voxel_state_t>::iterator it;
	
	/* check arguments */
	if(!filename)
		return -1;

	/* open file for writing */
	outfile.open(filename);
	if(!outfile.is_open())
		return -2;

	/* Write g to file */
	outfile << g.vs << endl; /* size of the voxels */
	for(it = g.voxels.begin(); it != g.voxels.end(); it++)
	{
		/* write the current voxel to file */
		outfile << (*it).first.x_ind << " "
			<< (*it).first.y_ind << " "
			<< (*it).first.z_ind << " "
			<< ((int) (*it).second) << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}
