#include "noisypath_io.h"
#include <util/binary_search.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <vector>
#include <fstream>
#include <string>
#include <string.h>
#include <iostream>

/**
 * @file noisypath_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief These files contain the i/o functionality for noisypath_t
 *
 * @section DESCRIPTION
 *
 * The classes in this file are used to read and write .noisypath files,
 * which house the probability distributions of the localization path
 * positions and rotations.  These files contain a superset of the info
 * stored in .mad files.
 */

using namespace std;
using namespace Eigen;
using namespace noisypath_io;

/* function implementations */

/*--------------*/
/* zupt_t class */
/*--------------*/

void zupt_t::serialize(ostream& os) const
{
	os.write((char*) &(this->start_time), sizeof(this->start_time));
	os.write((char*) &(this->end_time),   sizeof(this->end_time));
}

int zupt_t::parse(istream& is)
{
	is.read((char*) &(this->start_time), sizeof(this->start_time));
	is.read((char*) &(this->end_time),   sizeof(this->end_time));
	
	/* verify that stream is good */
	if(is.bad())
		return -1;

	/* success */
	return 0;
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
	/* import mean position */
	is.read((char*) &(this->mean(0)), sizeof(this->mean(0)));
	is.read((char*) &(this->mean(1)), sizeof(this->mean(1)));
	is.read((char*) &(this->mean(2)), sizeof(this->mean(2)));

	/* import covariance (upper triangle) */
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

/*--------------*/
/* pose_t class */
/*--------------*/
			
int pose_t::parse(istream& is)
{
	int ret;

	/* read timestamp */
	is.read((char*) &(this->timestamp), sizeof(this->timestamp));

	/* import position */
	ret = this->position.parse(is);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* import rotation */
	ret = this->rotation.parse(is);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
	return 0;
}
			
void pose_t::serialize(ostream& os) const
{
	/* export timestamp */
	os.write((char*) &(this->timestamp), sizeof(this->timestamp));

	/* export position distribution */
	this->position.serialize(os);
	
	/* export rotation distribution */
	this->rotation.serialize(os);
}

/*----------------*/
/* header_t class */
/*----------------*/
			
header_t::header_t()
{
	/* initialize empty */
	this->num_poses = 0;
	this->zupts.clear();
}
			
int header_t::parse(istream& is)
{
	char m[MAGIC_NUMBER_SIZE];
	unsigned int i, num_zupts;
	int ret;

	/* check for magic number */
	is.read(m, MAGIC_NUMBER_SIZE);
	if(memcmp(m, MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE))
	{
		/* wrong magic number */
		cerr << "[noisypath_io::header_t::parse]\t"
		     << "Input file is not a valid .noisypath file" << endl;
		return -1;
	}

	/* parse the input binary stream,
	 * all values are assumed to be in little-endian ordering */
	is.read((char*) &num_zupts, sizeof(num_zupts));
	is.read((char*) &(this->num_poses), sizeof(this->num_poses));

	/* read all zupts into vector */
	this->zupts.resize(num_zupts);
	for(i = 0; i < num_zupts; i++)
	{
		/* read the next zupt block */
		ret = this->zupts[i].parse(is);
		if(ret)
		{
			/* could not finish header */
			cerr << "[noisypath_io::header_t::parse]\t"
			     << "Unable to parse header information, "
			     << "zupt #" << i << endl;
			return PROPEGATE_ERROR(-2, ret);
		}
	}
	
	/* check file stream */
	if(is.bad())
	{
		/* could not finish header */
		cerr << "[noisypath_io::header_t::parse]\t"
		     << "Unable to parse header information" << endl;
		return -3;
	}

	/* success */
	return 0;
}

void header_t::serialize(ostream& os) const
{
	unsigned int i, num_zupts;

	/* write magic number */
	os.write(MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE);

	/* write values */
	num_zupts = this->zupts.size();
	os.write((char*) &num_zupts, sizeof(num_zupts));
	os.write((char*) &(this->num_poses), sizeof(this->num_poses));

	/* write zupt information */
	for(i = 0; i < num_zupts; i++)
		this->zupts[i].serialize(os);
}

/*----------------*/
/* reader_t class */
/*----------------*/

reader_t::reader_t()
{
	/* initialize values */
	this->timestamps.clear();
}
			
reader_t::~reader_t()
{
	/* free all memory and resources */
	this->close();
}
			
int reader_t::open(const std::string& filename)
{
	int ret;
	unsigned int i;
	pose_t p;

	/* close any open files */
	this->close();

	/* attempt to open this file */
	this->infile.open(filename.c_str(),
			ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
	{
		/* report error to user */
		cerr << "[noisypath_io::reader_t::open]\t"
		     << "Unable to open file: "
		     << filename << endl << endl;
		return -1;
	}

	/* parse the header */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* report error */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[noisypath_io::reader_t::open]\tError " 
		     << ret << ": Could not parse header from " 
		     << filename << ".  "
		     << "Are you sure this is the right file?"
		     << endl << endl;
		return ret;
	}

	/* attempt to read all poses in this file */
	this->timestamps.resize(this->header.num_poses);
	for(i = 0; i < this->header.num_poses; i++)
	{
		/* parse the next pose */
		ret = p.parse(this->infile);
		if(ret)
		{
			/* report error */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[noisypath_io::reader_t::open]\t"
			     << "Error " << ret
			     << ": Unable to parse pose #" << i
			     << endl << endl;
			return ret;
		}

		/* store the timestamp for this pose */
		this->timestamps[i] = p.timestamp;
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
		this->timestamps.clear();
		this->header.num_poses = 0;
		this->header.zupts.clear();
		this->infile.close();
	}
}
			
int reader_t::read(pose_t& p, unsigned int i)
{
	int ret;

	/* verify input */
	if(this->num_poses() <= i)
		return -1;
	
	/* move the stream to the appropriate position for the
	 * given pose in the file*/
	this->infile.seekg((streampos) 
		(HEADER_SIZE 
		+ (this->header.zupts.size()*ZUPT_ELEMENT_SIZE)
		+ (i*POSE_ELEMENT_SIZE) ));

	/* read the pose from file */
	ret = p.parse(this->infile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
	return 0;
}
			
int reader_t::read_nearest(pose_t& p, double t)
{
	unsigned int i;
	int ret;

	/* get the nearest time */
	i = binary_search::get_closest_index(this->timestamps, t);

	/* get the pose */
	ret = this->read(p, i);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	return 0;
}

/*----------------*/
/* writer_t class */
/*----------------*/

writer_t::~writer_t()
{
	this->close();
}

int writer_t::open(const string& filename, const vector<zupt_t>& zupts)
{
	/* close any open file streams */
	this->close();

	/* attempt to open file for writing */
	this->outfile.open(filename.c_str(),
			ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* report error */
		cerr << "[noisypath_io::writer_t::open]\t"
		     << "Unable to open file: "
		     << filename << endl << endl;
		return -1;
	}

	/* populate and write header info */
	this->header.num_poses = 0; /* populated by end of writing */
	this->header.zupts.clear();
	this->header.zupts.insert(this->header.zupts.begin(),
			zupts.begin(), zupts.end());
	this->header.serialize(this->outfile);

	/* success */
	return 0;
}
			
void writer_t::close()
{
	/* check if stream is open */
	if(this->outfile.is_open())
	{
		/* rewrite header with correct number of frames */
		this->outfile.seekp(0);
		this->header.serialize(this->outfile);

		/* clear stored info */
		this->header.num_poses = 0;
		this->header.zupts.clear();

		/* close the stream */
		this->outfile.close();
	}
}
			
int writer_t::write(const pose_t& p)
{
	/* write pose to file */
	p.serialize(this->outfile);

	/* success */
	this->header.num_poses++;
	return 0;
}
