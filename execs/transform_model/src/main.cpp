#include "transmodel_run_settings.h"
#include <util/tictoc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Converts units and offset of models
 *
 * @section DESCRIPTION
 *
 * This program is used to modify existing model files, by converting
 * from one set of units to another or by adding an offset.
 */

using namespace std;

/*------------------*/
/* helper functions */
/*------------------*/

/**
 * These functions are used to convert various file formats.
 */
int convert_ply(const transmodel_run_settings_t& args);
int convert_obj(const transmodel_run_settings_t& args);
int convert_xyz(const transmodel_run_settings_t& args);

/**
 * Converts a point based on the transform described in input args
 *
 * @param args   The parsed input arguments
 * @param x      The x-coordinate of point (will be modified in-place)
 * @param y      The y-coordinate of point (will be modified in-place)
 * @param z      The z-coordinate of point (will be modified in-place)
 */
inline void convert_point(const transmodel_run_settings_t& args,
			double& x, double& y, double& z)
{
	/* line is a point, convert it */
	x *= args.scale;
	y *= args.scale;
	z *= args.scale;
	x += args.translate;
	y += args.translate;
	z += args.translate;
}

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	transmodel_run_settings_t args;
	tictoc_t clk;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* convert ply files, if they exist */
	if(!(args.plyfiles.empty()))
	{
		/* convert it */
		tic(clk);
		ret = convert_ply(args);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to convert ply files." << endl;
			return 2;
		}
		toc(clk, "Converting ply files");
	}

	/* convert obj files, if they exist */
	if(!(args.objfiles.empty()))
	{
		/* convert it */
		tic(clk);
		ret = convert_obj(args);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to convert obj files." << endl;
			return 2;
		}
		toc(clk, "Converting obj files");
	}

	/* convert xyz files, if they exist */
	if(!(args.xyzfiles.empty()))
	{
		/* convert it */
		tic(clk);
		ret = convert_xyz(args);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to convert xyz files." << endl;
			return 2;
		}
		toc(clk, "Converting xyz files");
	}

	/* success */
	return 0;
}

int convert_ply(const transmodel_run_settings_t& args)
{
	ifstream infile;
	ofstream outfile;
	string tline;
	int num_found;
	double x, y, z, w;

	/* prepare input for reading */
	infile.open(args.plyfiles[0].c_str());
	if(!(infile.is_open()))
	{
		cerr << "[convert_ply]\tUnable to open input: "
		     << args.plyfiles[0] << endl;
		return -1;
	}

	/* prepare output for writing */
	outfile.open(args.plyfiles[1].c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[convert_ply]\tUnable to open output: "
		     << args.plyfiles[1] << endl;
		return -2;
	}

	/* iterate through file */
	while(!(infile.eof()))
	{
		/* get the next line */
		std::getline(infile, tline);

		/* check if this line is a point */
		num_found = sscanf(tline.c_str(),
				"%lf %lf %lf %lf", &x, &y, &z, &w);
		if(num_found != 3)
		{
			/* line is not a point, just pipe it */
			outfile << tline << endl;
		}
		else
		{
			/* convert and output it */
			convert_point(args, x, y, z);
			outfile << x << " " << y << " " << z << endl;
		}
	}

	/* clean up */
	infile.close();
	outfile.close();
	return 0;
}

int convert_obj(const transmodel_run_settings_t& args)
{
	ifstream infile;
	ofstream outfile;
	string tline;
	int num_found;
	double x, y, z;

	/* prepare input for reading */
	infile.open(args.objfiles[0].c_str());
	if(!(infile.is_open()))
	{
		cerr << "[convert_obj]\tUnable to open input: "
		     << args.objfiles[0] << endl;
		return -1;
	}

	/* prepare output for writing */
	outfile.open(args.objfiles[1].c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[convert_obj]\tUnable to open output: "
		     << args.objfiles[1] << endl;
		return -2;
	}

	/* iterate through file */
	while(!(infile.eof()))
	{
		/* get the next line */
		std::getline(infile, tline);

		/* check if this line is a point */
		num_found = sscanf(tline.c_str(),
				"v %lf %lf %lf", &x, &y, &z);
		if(num_found != 3)
		{
			/* line is not a point, just pipe it */
			outfile << tline << endl;
		}
		else
		{
			/* convert and output it */
			convert_point(args, x, y, z);
			outfile << "v " 
			        << x << " " << y << " " << z << endl;
		}
	}

	/* clean up */
	infile.close();
	outfile.close();
	return 0;
}

int convert_xyz(const transmodel_run_settings_t& args)
{
	ifstream infile;
	ofstream outfile;
	string tline;
	int num_found;
	double x, y, z, timestamp;
	int r, g, b, index, serial;

	/* prepare input for reading */
	infile.open(args.plyfiles[0].c_str());
	if(!(infile.is_open()))
	{
		cerr << "[convert_ply]\tUnable to open input: "
		     << args.plyfiles[0] << endl;
		return -1;
	}

	/* prepare output for writing */
	outfile.open(args.plyfiles[1].c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[convert_ply]\tUnable to open output: "
		     << args.plyfiles[1] << endl;
		return -2;
	}

	/* iterate through file */
	while(!(infile.eof()))
	{
		/* get the next line */
		std::getline(infile, tline);

		/* check if this line is a point */
		num_found = sscanf(tline.c_str(),
				"%lf %lf %lf %d %d %d %d %lf %d", 
				&x, &y, &z, &r, &g, &b,
				&index, &timestamp, &serial);
		if(num_found != 9)
		{
			/* line is not a point, just pipe it */
			outfile << tline << endl;
		}
		else
		{
			/* convert and output it */
			convert_point(args, x, y, z);
			outfile << x << " " << y << " " << z << " " 
			        << r << " " << g << " " << b << " "
			        << index << " " << timestamp << " "
			        << serial << endl;
		}
	}

	/* clean up */
	infile.close();
	outfile.close();
	return 0;
}
