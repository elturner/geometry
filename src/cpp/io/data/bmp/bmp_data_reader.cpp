#include "bmp_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>
#include <util/endian.h>

/**
 * @file bmp_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the BMP barometer
 */

using namespace std;

/* constant values used for parsing files */

#define BMP_FILE_MAGIC_NUMBER_LENGTH 7
#define BMP_FILE_MAGIC_NUMBER_VALUE  "BMP085"

/* bmp_frame_t function implementations */

bmp_frame_t::bmp_frame_t()
{
	/* set parameters for empty frame */
	this->index = 0;
	
	this->temp_timestamp = 0;
	this->temp = 0;

	this->pressure_timestamp = 0;
	this->pressure = 0;
	this->pressure_xlsb = 0;
}

bmp_frame_t::~bmp_frame_t()
{
	/* nothing is dynamically allocated */
}
		
int bmp_frame_t::parse(istream& is, double conversion_to_seconds,
                       unsigned char oversampling)
{
	unsigned int t;
	unsigned short s;
	unsigned char c;

	/* parse values from the stream */
	is.read((char*) (&t), sizeof(unsigned int)); /* little endian */
	this->temp_timestamp = conversion_to_seconds * t;
	is.read((char*) (&s), sizeof(unsigned short)); /* big endian */
	this->temp = be2les(s);
	is.read((char*) (&t), sizeof(unsigned int)); /* little endian */
	this->pressure_timestamp = conversion_to_seconds * t;
	is.read((char*) (&s), sizeof(unsigned short)); /* big endian */
	this->pressure = be2les(s);
	is.read((char*) (&c), sizeof(unsigned char));
	this->pressure_xlsb = c;

	/* check stream */
	if(is.fail())
		return -1;

	/* calculate pressure from received values */
	this->pressure = ((this->pressure << 8) + this->pressure_xlsb)
	                 >> (8 - oversampling);

	/* success */
	return 0;
}

/* bmp_reader_t function implementations */

bmp_reader_t::bmp_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->oversampling = 0;
	this->conversion_to_seconds = 1;
	this->num_scans = 0;
}

bmp_reader_t::~bmp_reader_t()
{
	/* close file if necessary */
	this->close();
}

int bmp_reader_t::open(const std::string& filename)
{
	char magic[BMP_FILE_MAGIC_NUMBER_LENGTH];
	unsigned int size_of_header;
	int i;

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, BMP_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, BMP_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a laser binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in size of header remaining */
	this->infile.read((char*)(&size_of_header), sizeof(size_of_header));

	/* read in calibration coefficients */
	this->infile.read((char*)(this->calib_coeffs),
		NUM_CALIBRATION_COEFFICIENTS * sizeof(unsigned short));

	/* convert each of these to little-endian ordering */
	for(i = 0; i < NUM_CALIBRATION_COEFFICIENTS; i++)
		this->calib_coeffs[i] = be2les(this->calib_coeffs[i]);
	
	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int));

	/* retrieve oversampling value */
	this->infile.read((char*) &(this->oversampling),
	                  sizeof(unsigned char));

	/* retrieve conversion of timestamps to seconds */
	this->infile.read((char*) (&(this->conversion_to_seconds)), 
	                  sizeof(double));

	/* we have now read the entirety of the header, and are
	 * ready to read the actual scan frame blocks */
	this->next_index = 0;
	return 0;
}

int bmp_reader_t::next(bmp_frame_t& frame)
{
	int ret;

	/* check if file is open */
	if(this->eof())
		return -1;

	/* attempt to parse frame from stream */
	ret = frame.parse(this->infile, this->conversion_to_seconds,
	                  this->oversampling);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* indicate the index of this frame */
	frame.index = this->next_index;
	this->next_index++;
	
	/* success */
	return 0;
}
		
bool bmp_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() || 
		        (this->next_index >= this->num_scans));
	return true;
}

void bmp_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->oversampling = 0;
	this->conversion_to_seconds = 1;
	this->num_scans = 0;
}
