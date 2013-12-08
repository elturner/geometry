#include "hmc_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>
#include <util/endian.h>

/**
 * @file hmc_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the HMC magnetometer
 */

using namespace std;

/* constant values used for parsing files */

#define HMC_FILE_MAGIC_NUMBER_LENGTH 9
#define HMC_FILE_MAGIC_NUMBER_VALUE  "HMC5883L"

/* hmc_frame_t function implementations */

hmc_frame_t::hmc_frame_t()
{
	/* set parameters for empty frame */
	this->index = 0;
	this->timestamp = 0;
	this->readings_x = NULL;
	this->readings_y = NULL;
	this->readings_z = NULL;
}

hmc_frame_t::~hmc_frame_t()
{
	/* free arrays if allocated */
	if(this->readings_x != NULL)
	{
		free(this->readings_x);
		this->readings_x = NULL;
	}
	if(this->readings_y != NULL)
	{
		free(this->readings_y);
		this->readings_y = NULL;
	}
	if(this->readings_z != NULL)
	{
		free(this->readings_z);
		this->readings_z = NULL;
	}
}
		
int hmc_frame_t::parse(istream& is, double conversion_to_seconds,
		       unsigned char ns)
{
	unsigned int i, t;
	unsigned short s;

	/* allocate arrays */
	this->num_sensors = ns;
	this->readings_x = (unsigned short*) realloc(this->readings_x,
	                   this->num_sensors * sizeof(unsigned short));
	this->readings_y = (unsigned short*) realloc(this->readings_y,
	                   this->num_sensors * sizeof(unsigned short));
	this->readings_z = (unsigned short*) realloc(this->readings_z,
	                   this->num_sensors * sizeof(unsigned short));

	/* parse timestamp from the stream */
	is.read((char*) (&t), sizeof(unsigned int)); /* little endian */
	this->timestamp = conversion_to_seconds * t;

	/* parse vector readings from stream */
	for(i = 0; i < this->num_sensors; i++)
	{
		/* read the next sensor reading */
		is.read((char*) (&s),
		        sizeof(unsigned short)); /* big endian */
		this->readings_x[i] = be2les(s); /* store x coord */
		is.read((char*) (&s),
		        sizeof(unsigned short)); /* big endian */
		this->readings_y[i] = be2les(s); /* store y coord */
		is.read((char*) (&s),
		        sizeof(unsigned short)); /* big endian */
		this->readings_z[i] = be2les(s); /* store z coord */
	}

	/* check stream */
	if(is.fail())
		return -1;

	/* success */
	return 0;
}

/* hmc_reader_t function implementations */

hmc_reader_t::hmc_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->num_sensors = 0;
	this->gain = 1;
	this->conversion_to_seconds = 1;
	this->num_scans = 0;
}

hmc_reader_t::~hmc_reader_t()
{
	/* close file if necessary */
	this->close();
}

int hmc_reader_t::open(const std::string& filename)
{
	char magic[HMC_FILE_MAGIC_NUMBER_LENGTH];
	unsigned int size_of_header;

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, HMC_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, HMC_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a laser binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in size of header remaining */
	this->infile.read((char*) (&size_of_header),
	                  sizeof(size_of_header));

	/* retrieve number of sensors */
	this->infile.read((char*) &(this->num_sensors), 
	                  sizeof(unsigned char)); 

	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int));

	/* retrieve gain, used for units of magnetometer readings */
	this->infile.read((char*) (&(this->gain)), 
	                  sizeof(double));

	/* retrieve conversion of timestamps to seconds */
	this->infile.read((char*) (&(this->conversion_to_seconds)), 
	                  sizeof(double));

	/* we have now read the entirety of the header, and are
	 * ready to read the actual scan frame blocks */
	this->next_index = 0;
	return 0;
}

int hmc_reader_t::next(hmc_frame_t& frame)
{
	int ret;

	/* check if file is open */
	if(this->eof())
		return -1;

	/* attempt to parse frame from stream */
	ret = frame.parse(this->infile, this->conversion_to_seconds,
	                  this->num_sensors);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* indicate the index of this frame */
	frame.index = this->next_index;
	this->next_index++;
	
	/* success */
	return 0;
}
		
bool hmc_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() || 
		        (this->next_index >= this->num_scans));
	return true;
}

void hmc_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->num_sensors = 0;
	this->gain = 1;
	this->conversion_to_seconds = 1;
	this->num_scans = 0;
}
