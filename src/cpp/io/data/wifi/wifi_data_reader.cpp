#include "wifi_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>

/**
 * @file wifi_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the wifi antennas.
 */

using namespace std;

/* constant values used for parsing files */

#define WIFI_FILE_MAGIC_NUMBER_LENGTH 5
#define WIFI_FILE_MAGIC_NUMBER_VALUE  "WIFI"
#define WIFI_FILE_MAX_NAME_LENGTH     256

/* wifi_frame_t function implementations */

wifi_frame_t::wifi_frame_t()
{
	/* set parameters for empty frame */
	this->index = 0;
	this->wifi_time_sec = 0;
	this->wifi_time_usec = 0;
	this->windows_time = 0;
	this->freq = 0;
	this->tag_num = 0;
	this->ssid = "";
	this->sig_level = 0;
}

wifi_frame_t::~wifi_frame_t()
{
	/* no data explicitely allocated */
}
		
int wifi_frame_t::parse(istream& is)
{
	unsigned char tag_len;
	char s[WIFI_FILE_MAX_NAME_LENGTH];

	/* parse timestamp values */
	is.read((char*) (&(this->wifi_time_sec)),
		sizeof(unsigned int));
	is.read((char*) (&(this->wifi_time_usec)),
		sizeof(unsigned int));
	is.read((char*) (&(this->windows_time)),
		sizeof(unsigned long long));

	/* read bssid */
	is.read((char*) (this->bssid),
		BSSID_SIZE * sizeof(short));

	/* read signal and frequency values */
	is.read((char*) &(this->sig_level),
		sizeof(char));
	is.read((char*) &(this->freq),
		sizeof(unsigned short));
	
	/* read tag information */
	is.read((char*) &(this->tag_num),
		sizeof(unsigned char));
	is.read((char*) &(tag_len),
		sizeof(unsigned char));
	is.read(s, tag_len * sizeof(char));
	this->ssid = string(s);
	
	/* check stream */
	if(is.fail())
		return -1;

	/* success */
	return 0;
}

/* wifi_reader_t function implementations */

wifi_reader_t::wifi_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->antenna_name = "";
	this->num_scans = 0;
}

wifi_reader_t::~wifi_reader_t()
{
	/* close file if necessary */
	this->close();
}

int wifi_reader_t::open(const std::string& filename)
{
	char magic[WIFI_FILE_MAGIC_NUMBER_LENGTH];
	char str[WIFI_FILE_MAX_NAME_LENGTH];

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, WIFI_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, WIFI_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a wifi binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in antenna name */
	this->infile.getline(str, WIFI_FILE_MAX_NAME_LENGTH, '\0');
	this->antenna_name = string(str);

	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int));

	/* we have now read the entirety of the header, and are
	 * ready to read the actual scan frame blocks */
	this->next_index = 0;
	return 0;
}

int wifi_reader_t::next(wifi_frame_t& frame)
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
		
bool wifi_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() || 
		        (this->next_index >= this->num_scans));
	return true;
}

void wifi_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->antenna_name = "";
	this->num_scans = 0;
}
