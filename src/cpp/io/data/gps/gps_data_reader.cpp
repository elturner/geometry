#include "gps_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>

/**
 * @file gps_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the GPS antenna.
 */

using namespace std;

/* constant values used for parsing files */

#define GPS_FILE_MAGIC_NUMBER_LENGTH 4
#define GPS_FILE_MAGIC_NUMBER_VALUE  "GPS"
#define GPS_FILE_MAX_NAME_LENGTH     128

/* gps_frame_t function implementations */

gps_frame_t::gps_frame_t()
{
	/* set parameters for empty frame */
	this->index = 0;
	this->timestamp = 0;
	this->data = NULL;
}

gps_frame_t::~gps_frame_t()
{
	/* free the data if allocated */
	if(this->data != NULL)
	{
		free(this->data);
		this->data = NULL;
	}
}
		
int gps_frame_t::parse(istream& is)
{
	/* parse values from the stream */
	is.read((char*) (&(this->timestamp)), 
	        sizeof(unsigned long long));
	is.read((char*) (&(this->data_size)), 
	        sizeof(unsigned int));

	/* allocate space for data */
	this->data = (char*) realloc(this->data,
		this->data_size * sizeof(char));

	/* store data */
	is.read(this->data, 
	        this->data_size * sizeof(char));

	/* check stream */
	if(is.fail())
		return -1;

	/* success */
	return 0;
}

/* gps_reader_t function implementations */

gps_reader_t::gps_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->serial_num = "";
	this->num_scans = 0;
}

gps_reader_t::~gps_reader_t()
{
	/* close file if necessary */
	this->close();
}

int gps_reader_t::open(const std::string& filename)
{
	char magic[GPS_FILE_MAGIC_NUMBER_LENGTH];
	char str[GPS_FILE_MAX_NAME_LENGTH];
	unsigned int size_of_header;

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, GPS_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, GPS_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a laser binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in serial number */
	this->infile.getline(str, GPS_FILE_MAX_NAME_LENGTH, '\0');
	this->serial_num = string(str);

	/* read in size of header remaining */
	this->infile.read((char*)(&size_of_header), sizeof(size_of_header));

	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int));

	/* we have now read the entirety of the header, and are
	 * ready to read the actual scan frame blocks */
	this->next_index = 0;
	return 0;
}

int gps_reader_t::next(gps_frame_t& frame)
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
		
bool gps_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() || 
		        (this->next_index >= this->num_scans));
	return true;
}

void gps_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->serial_num = "";
	this->num_scans = 0;
}
