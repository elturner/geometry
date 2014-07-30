#include "msd_io.h"
#include <iostream>
#include <fstream>
#include <string>
#include <Eigen/Dense>

/**
 * @file   msd_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to export .msd files
 *
 * @section DESCRIPTION
 *
 * The .msd file format is a deprecated format used by the gen-1
 * backpack processing code.  In order to interface the data products
 * of later generation backpacks with the gen-1 pipeline, this
 * writer class can be used to export information into the MSD format.
 *
 * This class requires the Eigen framework
 */

using namespace std;
using namespace Eigen;
using namespace msd;

/*-----------------------------------*/
/* header_t function implementations */
/*-----------------------------------*/

void header_t::serialize(std::ostream& os) const
{
	/* export data to stream (little-endian) */
	os.write((char*) &(this->serial_num), sizeof(this->serial_num));
	os.write((char*) &(this->R(0,0)),     sizeof(double));
	os.write((char*) &(this->R(0,1)),     sizeof(double));
	os.write((char*) &(this->R(0,2)),     sizeof(double));
	os.write((char*) &(this->R(1,0)),     sizeof(double));
	os.write((char*) &(this->R(1,1)),     sizeof(double));
	os.write((char*) &(this->R(1,2)),     sizeof(double));
	os.write((char*) &(this->R(2,0)),     sizeof(double));
	os.write((char*) &(this->R(2,1)),     sizeof(double));
	os.write((char*) &(this->R(2,2)),     sizeof(double));
	os.write((char*) &(this->T(0)),       sizeof(double));
	os.write((char*) &(this->T(1)),       sizeof(double));
	os.write((char*) &(this->T(2)),       sizeof(double));
	os.write((char*) &(this->num_scans),  sizeof(this->num_scans));
}

/*----------------------------------*/
/* frame_t function implementations */
/*----------------------------------*/

void frame_t::serialize(std::ostream& os) const
{
	size_t i, n;

	/* export number of points in this frame */
	os.write((char*) &(this->num_points), sizeof(this->num_points));

	/* export timestamp */
	os.write((char*) &(this->timestamp), sizeof(this->timestamp));

	/* export each point */
	n = this->num_points;
	for(i = 0; i < n; i++)
	{
		/* export current point (units: millimeters) */
		os.write((char*) &(this->points(0,i)), sizeof(double));
		os.write((char*) &(this->points(1,i)), sizeof(double));
	}
}

/*-----------------------------------*/
/* writer_t function implementations */
/*-----------------------------------*/

void writer_t::init(int serial, const Eigen::Matrix3d& R,
				const Eigen::Vector3d& T, int num)
{
	/* populate the header object */
	this->header.serial_num = serial;
	this->header.T = T;
	this->header.R = R;
	this->header.num_scans = num;
}

int writer_t::open(const std::string& filename)
{
	/* close any opened streams */
	this->close();

	/* attempt to open this file */
	this->outfile.open(filename.c_str(),
				ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* unable to open */
		return -1;
	}

	/* write the header information to file */
	this->header.serialize(this->outfile);

	/* success */
	return 0;
}
			
void writer_t::write(const frame_t& frame)
{
	/* write this frame to the current outfile */
	frame.serialize(this->outfile);
}

void writer_t::close()
{
	/* check if a stream is open */
	if(this->outfile.is_open())
	{
		/* close the stream */
		this->outfile.close();
	}
}
