#include "urg_data_reader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <util/error_codes.h>

/**
 * @file urg_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of classes used to
 * parse the output binary data files for the Hakuyo URG laser
 * scanners.
 */

using namespace std;

/* constant values used for parsing files */

#define LASER_FILE_MAGIC_NUMBER_LENGTH 6
#define LASER_FILE_MAGIC_NUMBER_VALUE  "LASER"
#define LASER_FILE_MAX_NAME_LENGTH     128


/* urg_frame_t function implementations */

urg_frame_t::urg_frame_t()
{
	/* set parameters for empty frame */
	this->num_points = 0;
	this->range_values = NULL;
	this->intensity_values = NULL;
	this->timestamp = 0;
	this->index = 0;
}

urg_frame_t::~urg_frame_t()
{
	/* free any allocated arrays */
	if(this->range_values != NULL)
	{
		free(this->range_values);
		this->range_values = NULL;
	}
	if(this->intensity_values != NULL)
	{
		free(this->intensity_values);
		this->intensity_values = NULL;
	}
	this->num_points = 0;
}
		
int urg_frame_t::parse(istream& is, bool capture_mode, unsigned int np)
{
	/* check arguments */
	if(np == 0)
		return -1; /* invalid */

	/* allocate memory for the frame */
	if(np != this->num_points)
	{
		/* need to readjust size of arrays */
		this->num_points = np;
		this->range_values = (unsigned int*) realloc(
				this->range_values,
				np * sizeof(unsigned int));

		/* verify memory */
		if(this->range_values == NULL)
			return -2;

		/* check if we need to store intensity values */
		if(capture_mode)
		{
			/* allocate intensity values */
			this->intensity_values = (unsigned int*) realloc(
				this->intensity_values,
				np * sizeof(unsigned int));
		
			/* verify memory */
			if(this->intensity_values == NULL)
				return -3;
		}
	}

	/* parse the timestamp from the stream */
	is.read((char*) (&(this->timestamp)), sizeof(this->timestamp));

	/* parse the range values from the stream */
	is.read((char*) this->range_values, np * sizeof(unsigned int));

	/* optionally parse the intensity values from stream */
	if(capture_mode)
		is.read((char*) this->intensity_values,
		        np * sizeof(unsigned int));

	/* check stream */
	if(is.fail())
		return -4;

	/* success */
	return 0;
}

/* urg_reader_t function implementations */

urg_reader_t::urg_reader_t()
{
	/* set default values of unopened reader */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->hardware_model = "";
	this->serial_num = "";
	this->capture_mode = false;
	this->num_scans = 0;
	this->points_per_scan = 0;
	this->max_range = 0;
	this->min_range = 0;
	this->angle_map = NULL;
}

urg_reader_t::~urg_reader_t()
{
	/* close file if necessary */
	this->close();
}

int urg_reader_t::open(const std::string& filename)
{
	urg_frame_t frame;
	char magic[LASER_FILE_MAGIC_NUMBER_LENGTH];
	char str[LASER_FILE_MAX_NAME_LENGTH];
	unsigned int size_of_header;
	size_t i, s;
	int ret, cm;

	/* close any open file */
	this->close();

	/* attempt to open file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* attempt to read magic number from file */
	this->infile.read(magic, LASER_FILE_MAGIC_NUMBER_LENGTH);
	if(strcmp(magic, LASER_FILE_MAGIC_NUMBER_VALUE))
	{
		/* this file is not a laser binary data file */
		this->infile.close();
		return -2;
	}

	/* read in the version numbers */
	this->infile.read(&(this->major_version), sizeof(char));
	this->infile.read(&(this->minor_version), sizeof(char));

	/* read in model string */
	this->infile.getline(str, LASER_FILE_MAX_NAME_LENGTH, '\0');
	this->hardware_model = string(str);
	
	/* read in serial number */
	this->infile.getline(str, LASER_FILE_MAX_NAME_LENGTH, '\0');
	this->serial_num = string(str);

	/* read in size of header remaining */
	this->infile.read((char*)(&size_of_header), sizeof(size_of_header));

	/* check the capture mode */
	this->infile.read((char*) (&cm), sizeof(cm));
	this->capture_mode = (cm != 0);

	/* retrieve the number of scans in this file */
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(this->num_scans));

	/* retrieve the number of points within each scan */
	this->infile.read((char*) (&(this->points_per_scan)),
	                  sizeof(this->points_per_scan));

	/* retrieve valid range measurements for the scans */
	this->infile.read((char*) (&(this->max_range)),
	                  sizeof(this->max_range));
	this->infile.read((char*) (&(this->min_range)),
	                  sizeof(this->min_range));

	/* initialize memory for the angle map */
	this->angle_map = (float*) malloc(this->points_per_scan
	                                  * sizeof(float));
	if(this->angle_map == NULL)
		return -3;

	/* retrieve the angle map from file */
	this->infile.read((char*) this->angle_map,
	                  this->points_per_scan * sizeof(float));

	/* prepare list of frame locations in the file, for random access */
	this->frame_locs.resize(this->num_scans); /* init list */
	this->frame_locs[0] = this->infile.tellg(); /* position of first 
	                                               frame */
	ret = this->next(frame);
	if(ret)
		return -4; /* could not read the first frame in file */
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

int urg_reader_t::next(urg_frame_t& frame)
{
	int ret;

	/* check if file is open */
	if(this->eof())
		return -1;

	/* attempt to parse frame from stream */
	ret = frame.parse(this->infile, this->capture_mode, 
	                  this->points_per_scan);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* indicate the index of this frame */
	frame.index = this->next_index;
	this->next_index++;
	
	/* success */
	return 0;
}
		
int urg_reader_t::get(unsigned int i, urg_frame_t& frame)
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

int urg_reader_t::parse_timestamps(vector<double>& times)
{
	urg_frame_t frame;
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
		
bool urg_reader_t::eof() const
{
	if(this->infile.is_open())
		return (this->infile.eof() 
		        || (this->next_index >= this->num_scans));
	return true;
}

void urg_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* free the allocated angle map memory */
	free(this->angle_map);

	/* reset values */
	this->next_index = 0;
	this->major_version = '\0';
	this->minor_version = '\0';
	this->hardware_model = "";
	this->serial_num = "";
	this->capture_mode = false;
	this->num_scans = 0;
	this->points_per_scan = 0;
	this->max_range = 0;
	this->min_range = 0;
	this->angle_map = NULL;
}
