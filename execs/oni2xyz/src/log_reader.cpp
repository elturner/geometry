#include "log_reader.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/StdVector>

/**
 * @file   log_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Reader class for OpenNI log files
 *
 * @section DESCRIPTION
 *
 * This file contains the log_reader_t class, which is used to 
 * read .log files.  These files contain the trajectory of a
 * PrimeSense sensor and correspond to the data in a .oni file.
 *
 * This class requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;

/* hard-coded constants for the Kinect Sensor */

#define FOCAL_LENGTH_X    525.0 /* default focal length */
#define FOCAL_LENGTH_Y    525.0 /* default focal length */
#define OPTICAL_CENTER_X  319.5 /* defualt optical center */
#define OPTICAL_CENTER_Y  239.5 /* default optical center */
#define MM2METERS(x)   ((x) / 1000.0) /* convert millimeters to meters */

/* function implementations */

int log_reader_t::parse(const string& filename)
{
	ifstream infile;
	string tline;
	stringstream ss;
	Matrix4d T;
	size_t i1, i2, i3, j;
	
	/* attempt to open file for reading */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		/* unable to open file */
		cerr << "[log_reader_t::parse]\tUnable to open file: "
		     << filename << endl;
		return -1; 
	}

	/* clear any existing data */
	this->poses.clear();

	/* read all of file */
	while(!(infile.eof()))
	{
		/* read next line */
		std::getline(infile, tline);
		ss.clear();
		ss.str(tline);

		/* check for empty lines */
		if(tline.empty())
			continue;

		/* the first line should have three indices */
		if(!(ss >> i1 >> i2 >> i3))
		{
			/* report error */
			cerr << "[log_reader_t::parse]\t"
			     << "Could not parse line: " << tline << endl;
			infile.close();
			return -2;
		}

		/* check that this line makes sense
		 *
		 * We expect:
		 *
		 * 	<i> <i> <i+1>
		 *
		 * For the i'th pose
		 */
		if(i1 != i2 || (i1+1) != i3 || i1 != this->poses.size())
		{
			/* report error */
			cerr << "[log_reader_t::parse]\t"
			     << "Invalid index line for frame: "
			     << tline << endl;
			infile.close();
			return -3;
		}

		/* the next four lines will be a 4x4 matrix */
		for(j = 0; j < 16; j++)
			infile >> T(j);
		T.transposeInPlace(); /* file was written in row-major */

		/* store this pose */
		this->poses.push_back(T);
	}

	/* clean up */
	infile.close();
	return 0;
}
		
int log_reader_t::compute_point(size_t f, size_t u, size_t v, double d,
				Vector3d& p) const
{
	Vector4d w;
	double x,y,z;

	/* check arguments */
	if(f >= this->poses.size())
		return -1;

	/* translation from depth pixel (u,v,d) to a point (x,y,z) */
	z = MM2METERS(d);
	x = (u - OPTICAL_CENTER_X) * z / FOCAL_LENGTH_X;
	y = (v - OPTICAL_CENTER_Y) * z / FOCAL_LENGTH_Y;

	/* transform (x,y,z) from sensor coords to world coords */
	w = this->poses[f] * Eigen::Vector4d(x, y, z, 1);
	
	/* save the first three elements */
	p(0) = w(0); 
	p(1) = w(1);
	p(2) = w(2);

	/* success */
	return 0;
}
