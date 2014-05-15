#include "carve_map_io.h"
#include <geometry/carve/gaussian/carve_map.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <fstream>
#include <string>
#include <string.h>
#include <iostream>

/**
 * @file carve_map_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief These files contain the i/o functionality for carve_map_t
 *
 * @section DESCRIPTION
 *
 * The classes in this file are used to read and write .carvemap files,
 * which house the probability distributions of the input scan points
 * and the sensor positions, which are modeled as gaussians in global 3D
 * coordinates.
 */

using namespace std;
using namespace Eigen;
using namespace cm_io;

/* function implementations */

/*----------------*/
/* reader_t class */
/*----------------*/

reader_t::reader_t()
{
	/* initialize values */
	this->frames = NULL;
}
			
reader_t::~reader_t()
{
	/* free all memory and resources */
	this->close();
}
			
int reader_t::open(const std::string& filename)
{
	int ret;
	size_t f, n, m;
	gauss_dist_t gd;

	/* close any open files */
	this->close();

	/* attempt to open this file */
	this->infile.open(filename.c_str(),
			ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
	{
		/* report error to user */
		cerr << "[cm_io::reader_t::open]\tUnable to open file: "
		     << filename << endl << endl;
		return -1;
	}

	/* parse the header */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* report error */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[cm_io::reader_t::open]\tError " << ret << ": "
		     << "Could not parse header from " << filename << ".  "
		     << "Are you sure this is the right file?"
		     << endl << endl;
		return ret;
	}

	/* allocate space for all frames */
	n = this->num_frames();
	this->frames = new frame_t[n];

	/* attempt to read all frames in this file */
	for(f = 0; f < n; f++)
	{
		/* parse the next frame */
		ret = this->frames[f].parse(this->infile);
		if(ret)
		{
			/* report error */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[cm_io::reader_t::open]\tError " << ret
			     << ": Unable to parse frame #" << f
			     << endl << endl;
			return ret;
		}

		/* iterate through points in frame */
		m = this->num_points_in_frame(f);
		this->infile.seekg(m*POINT_INFO_SIZE, ios_base::cur);
	}

	/* success */
	return 0;
}
			
void reader_t::close()
{
	/* check if stream is open */
	if(this->infile.is_open())
	{
		/* since stream is open, close it */
		this->infile.close();
	}

	/* check if memory is allocated */
	if(this->frames != NULL)
	{
		/* free this memory */
		delete[] (this->frames);
	}
}
			
int reader_t::read(carve_map_t& cm, size_t f, size_t i)
{
	gauss_dist_t dist;
	double planar_prob, corner_prob;
	int ret;

	/* verify input */
	if(this->frames == NULL || this->num_frames() <= f)
		return -1;

	/* move the stream to the appropriate position for the
	 * given frame and point*/
	this->infile.seekg(this->frames[f].fileloc
			+ ((streampos) (FRAME_HEADER_SIZE 
			+ i*POINT_INFO_SIZE)));

	/* read the scanpoint distribution from file */
	ret = dist.parse(this->infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	this->infile.read((char*) &planar_prob, sizeof(planar_prob));
	this->infile.read((char*) &corner_prob, sizeof(corner_prob));

	/* populate carve map with appropriate values */
	cm.init(this->frames[f].sensor_pos.mean,
	        this->frames[f].sensor_pos.cov, dist.mean, dist.cov);
	// TODO export planar_prob and corner_prob

	/* success */
	return 0;
}

/*----------------*/
/* writer_t class */
/*----------------*/

writer_t::~writer_t()
{
	this->close();
}

int writer_t::open(const string& filename, size_t num_frames)
{
	header_t header;

	/* close any open file streams */
	this->close();

	/* attempt to open file for writing */
	this->outfile.open(filename.c_str(),
			ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* report error */
		cerr << "[cm_io::writer_t::open]\tUnable to open file: "
		     << filename << endl << endl;
		return -1;
	}

	/* writer header info */
	header.num_frames = num_frames;
	header.print(this->outfile);

	/* success */
	return 0;
}
			
void writer_t::close()
{
	/* check if stream is open */
	if(this->outfile.is_open())
		this->outfile.close(); /* close it */
}
			
int writer_t::write_frame(const carve_map_t* cm_arr, size_t num)
{
	frame_t frame;
	gauss_dist_t dist;
	double p, c;
	size_t i;

	/* verify the input */
	if(cm_arr == NULL || num == 0)
		return -1;

	/* prepare the frame information */
	frame.num_points = num;
	cm_arr[0].get_sensor_mean(frame.sensor_pos.mean);
	cm_arr[0].get_sensor_cov(frame.sensor_pos.cov);

	/* write the frame information */
	frame.serialize(this->outfile);

	/* write each point's info */
	for(i = 0; i < num; i++)
	{
		/* get distribution info */
		cm_arr[i].get_scanpoint_mean(dist.mean);
		cm_arr[i].get_scanpoint_cov(dist.cov);

		/* write to file */
		dist.serialize(this->outfile);

		/* write planarity/corner info to file */
		p = cm_arr[i].get_planar_prob();
		c = cm_arr[i].get_corner_prob();
		this->outfile.write((char*) &p, sizeof(p));
		this->outfile.write((char*) &c, sizeof(c));
	}

	/* success */
	return 0;
}

/*----------------*/
/* header_t class */
/*----------------*/
			
header_t::header_t()
{
	/* initialize empty */
	this->num_frames = 0;
}
			
int header_t::parse(istream& is)
{
	char m[MAGIC_NUMBER_SIZE];

	/* check for magic number */
	is.read(m, MAGIC_NUMBER_SIZE);
	if(memcmp(m, MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE))
	{
		/* wrong magic number */
		cerr << "[cm_io::header_t::parse]\t"
		     << "Input file is not a valid .carvemap file" << endl;
		return -1;
	}

	/* parse the input binary stream,
	 * all values are assumed to be in little-endian ordering */
	is.read((char*) &(this->num_frames), sizeof(this->num_frames));

	/* check file stream */
	if(is.bad())
	{
		/* could not finish header */
		cerr << "[cm_io::header_t::parse]\t"
		     << "Unable to parse header information" << endl;
		return -2;
	}

	/* success */
	return 0;
}

void header_t::print(ostream& os) const
{
	/* write magic number */
	os.write(MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE);

	/* write values */
	os.write((char*) &(this->num_frames), sizeof(this->num_frames));
}

/*-------------------*/
/* frame_t functions */
/*-------------------*/

frame_t::frame_t()
{
	/* initialize default frame */
	this->fileloc = 0;
	this->num_points = 0;
}

int frame_t::parse(std::istream& is)
{
	int ret;

	/* store location of this frame in file*/
	this->fileloc = is.tellg();

	/* parse stream */
	is.read((char*) &(this->num_points), sizeof(this->num_points));
	ret = this->sensor_pos.parse(is);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

void frame_t::serialize(std::ostream& os) const
{
	/* write info to stream */
	os.write((char*) &(this->num_points), sizeof(this->num_points));
	this->sensor_pos.serialize(os);

	/* note that the field 'fileloc' is not used here, since it
	 * only references input streams */
}

/*------------------------*/
/* gauss_dist_t functions */
/*------------------------*/
			
void gauss_dist_t::serialize(ostream& os) const
{
	/* export mean position */
	os.write((char*) &(this->mean(0)), sizeof(this->mean(0)));
	os.write((char*) &(this->mean(1)), sizeof(this->mean(1)));
	os.write((char*) &(this->mean(2)), sizeof(this->mean(2)));
	
	/* export covariance (upper triangle) */
	os.write((char*) &(this->cov(0,0)), sizeof(this->cov(0,0)));
	os.write((char*) &(this->cov(0,1)), sizeof(this->cov(0,1)));
	os.write((char*) &(this->cov(0,2)), sizeof(this->cov(0,2)));
	os.write((char*) &(this->cov(1,1)), sizeof(this->cov(1,1)));
	os.write((char*) &(this->cov(1,2)), sizeof(this->cov(1,2)));
	os.write((char*) &(this->cov(2,2)), sizeof(this->cov(2,2)));
}
			
int gauss_dist_t::parse(istream& is)
{
	/* export mean position */
	is.read((char*) &(this->mean(0)), sizeof(this->mean(0)));
	is.read((char*) &(this->mean(1)), sizeof(this->mean(1)));
	is.read((char*) &(this->mean(2)), sizeof(this->mean(2)));
	
	/* export covariance (upper triangle) */
	is.read((char*) &(this->cov(0,0)), sizeof(this->cov(0,0)));
	is.read((char*) &(this->cov(0,1)), sizeof(this->cov(0,1)));
	is.read((char*) &(this->cov(0,2)), sizeof(this->cov(0,2)));
	is.read((char*) &(this->cov(1,1)), sizeof(this->cov(1,1)));
	is.read((char*) &(this->cov(1,2)), sizeof(this->cov(1,2)));
	is.read((char*) &(this->cov(2,2)), sizeof(this->cov(2,2)));	

	/* copy covariance values to lower triangle */
	this->cov(1,0) = this->cov(0,1);
	this->cov(2,0) = this->cov(0,2);
	this->cov(2,1) = this->cov(1,2);

	/* verify that stream is good */
	if(is.bad())
		return -1;

	/* success */
	return 0;
}
