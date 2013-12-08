#include "ic4_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>

/**
 * @file ic4_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the Intersense IC4.
 */

using namespace std;

/* constant values used for parsing files */

#define IC4_FILE_MAGIC_NUMBER_LENGTH 4
#define IC4_FILE_MAGIC_NUMBER_VALUE  "IC4"
#define IC4_FILE_MAX_NAME_LENGTH     128

/* ic4_frame_t function implementations */

ic4_frame_t::ic4_frame_t()
{
	/* set parameters for empty frame */
	this->index = 0;
	this->timestamp = 0;
}

ic4_frame_t::~ic4_frame_t()
{
	/* nothing is dynamically allocated */
}
		
int ic4_frame_t::parse(istream& is)
{
	/* parse values from the stream */
	is.read((char*) (&(this->timestamp)), 
	        sizeof(float));
	is.read((char*) (&(this->still_time)), 
	        sizeof(float));
	is.read((char*) (&(this->euler)), 
	        EULER_ANGLE_SIZE * sizeof(float));
	is.read((char*) (&(this->quaternion)),
	        QUATERNION_SIZE * sizeof(float));
	is.read((char*) (&(this->compass_yaw)), 
	        sizeof(float));
	is.read((char*) (&(this->angular_velocity_body)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->angular_velocity_nav)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->acceleration_body)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->acceleration_nav)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->velocity_nav)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->angular_velocity_raw)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->mag_body_frame)), 
	        VECTOR_SIZE * sizeof(float));
	is.read((char*) (&(this->temperature)), 
	        sizeof(float));
	is.read((char*) (&(this->status)), 
	        sizeof(unsigned char));

	/* check stream */
	if(is.fail())
		return -1;

	/* success */
	return 0;
}

/* ic4_reader_t function implementations */

ic4_reader_t::ic4_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->hardware_model = "";
	this->serial_num = "";
	this->enhancement_level = 0;
	this->sensitivity_level = 0;
	this->buffer_query_time = 0;
	this->num_scans = 0;
}

ic4_reader_t::~ic4_reader_t()
{
	/* close file if necessary */
	this->close();
}

int ic4_reader_t::open(const std::string& filename)
{
	ic4_frame_t frame;
	char magic[IC4_FILE_MAGIC_NUMBER_LENGTH];
	char str[IC4_FILE_MAX_NAME_LENGTH];
	unsigned int i, size_of_header;
	size_t s;
	int ret;

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, IC4_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, IC4_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a laser binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in model string */
	this->infile.getline(str, IC4_FILE_MAX_NAME_LENGTH, '\0');
	this->hardware_model = string(str);
	
	/* read in serial number */
	this->infile.getline(str, IC4_FILE_MAX_NAME_LENGTH, '\0');
	this->serial_num = string(str);

	/* read in size of header remaining */
	this->infile.read((char*)(&size_of_header), sizeof(size_of_header));

	/* retrieve enhancement level */
	this->infile.read((char*) (&(this->enhancement_level)), 
	                  sizeof(unsigned int));

	/* retrieve sensitivity level */
	this->infile.read((char*) (&(this->sensitivity_level)), 
	                  sizeof(unsigned int));

	/* retrieve buffer query time */
	this->infile.read((char*) (&(this->buffer_query_time)), 
	                  sizeof(unsigned int));

	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int));

	/* prepare list of frame locations in the file, for random access */
	this->frame_locs.resize(this->num_scans); /* init list */
	this->frame_locs[0] = this->infile.tellg(); /* position of first 
	                                               frame */
	ret = this->next(frame);
	if(ret)
		return -3; /* could not read the first frame in file */
	this->frame_locs[1] = this->infile.tellg(); /* position of second 
	                                               frame */

	/* since all frames are the same size, the frame locations in
	 * the file will be at equal spacing */
	s = (size_t)(this->frame_locs[1] - this->frame_locs[0]); /* size of frame */
	for(i = 2; i < this->num_scans; i++)
		this->frame_locs[i] = i*s + this->frame_locs[0];
				/* position of remaining frames */

	/* reset file back to first frame */
	this->infile.seekg(this->frame_locs[0]);
	this->next_index = 0;

	/* we have now read the entirety of the header, and are
	 * ready to read the actual scan frame blocks */
	return 0;
}

int ic4_reader_t::next(ic4_frame_t& frame)
{
	int ret;

	/* check if file is open */
	if(this->eof())
		return -1;

	/* attempt to parse frame from stream */
	ret = frame.parse(this->infile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* indicate the index of this frame */
	frame.index = this->next_index;
	this->next_index++;
	
	/* success */
	return 0;
}
		
int ic4_reader_t::get(unsigned int i, ic4_frame_t& frame)
{
	int ret;

	/* check arguments */
	if(i >= this->frame_locs.size())
		return -1; /* bad index */

	/* set position of file to i'th frame */
	this->next_index = i;
	this->infile.seekg(this->frame_locs[i]);

	/* get the next frame */
	ret = this->next(frame);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* success */
	return 0;
}
		
int ic4_reader_t::parse_timestamps(vector<double>& times)
{
	ic4_frame_t frame;
	int ret;

	/* clear the vector */
	times.clear();

	/* get all the timestamps */
	while(!(this->eof()))
	{
		/* get next frame */
		ret = this->next(frame);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	
		/* scrape its timestamp */
		times.push_back(frame.timestamp);
	}

	/* success */
	return 0;
}
		
bool ic4_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() || 
		        (this->next_index >= this->num_scans));
	return true;
}

void ic4_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->hardware_model = "";
	this->serial_num = "";
	this->enhancement_level = 0;
	this->sensitivity_level = 0;
	this->buffer_query_time = 0;
	this->num_scans = 0;
}
