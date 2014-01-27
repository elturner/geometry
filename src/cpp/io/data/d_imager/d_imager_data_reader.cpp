#include "d_imager_data_reader.h"
#include <istream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <util/error_codes.h>

/**
 * @file d_imager_data_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu
 *
 * @section DESCRIPTION
 *
 * This is the implementation of the d+imager_reader_t
 * class and associated classes.
 */

using namespace std;

/* The following constants are used for parsing the datafile */

#define MAGIC_NUMBER_VALUE  "dimager"
#define MAGIC_NUMBER_LENGTH 8 /* including null-terminator */

/* D-Imager Frame class function implementations */

d_imager_frame_t::d_imager_frame_t()
{
	/* initialize empty frame */
	this->image_width = 0;
	this->image_height = 0;
	this->index = -1;
	this->timestamp = 0;
	this->xdat = NULL;
	this->ydat = NULL;
	this->zdat = NULL;
	this->ndat = NULL;
}

d_imager_frame_t::~d_imager_frame_t()
{
	/* check if things are allocated */
	if(this->xdat != NULL)
	{
		free(this->xdat);
		this->xdat = NULL;
	}
	if(this->ydat != NULL)
	{
		free(this->ydat);
		this->ydat = NULL;
	}
	if(this->zdat != NULL)
	{
		free(this->zdat);
		this->zdat = NULL;
	}
	if(this->ndat != NULL)
	{
		free(this->ndat);
		this->ndat = NULL;
	}
}

int d_imager_frame_t::init_resolution(int w, int h)
{
	int buf_size;

	/* check validity */
	if(w <= 0 || h <= 0)
		return -1;

	/* store resolution */
	this->image_width = w;
	this->image_height = h;

	/* allocate buffers */
	buf_size = w*h;
	this->xdat = (short*) realloc(this->xdat, sizeof(short) * buf_size);
	this->ydat = (short*) realloc(this->ydat, sizeof(short) * buf_size);
	this->zdat = (short*) realloc(this->zdat, sizeof(short) * buf_size);
	this->ndat = (unsigned short*) realloc(this->ndat, 
				sizeof(unsigned short) * buf_size);

	/* check for errors */
	if(this->xdat == NULL || this->ydat == NULL || this->zdat == NULL
			|| this->ndat == NULL)
		return -2;

	/* success */
	return 0;
}

int d_imager_frame_t::parse(istream& is)
{
	int buf_size;

	/* assume values for image width and height are valid,
	 * and that buffers are sufficiently sized */
	buf_size = this->image_width * this->image_height;

	/* read the timestamp */
	is.read( (char*) (&(this->timestamp)), sizeof(unsigned long long) );

	/* read the data */
	is.read( (char*) this->xdat, sizeof(short) * buf_size ); 
	is.read( (char*) this->ydat, sizeof(short) * buf_size ); 
	is.read( (char*) this->zdat, sizeof(short) * buf_size ); 
	is.read( (char*) this->ndat, sizeof(unsigned short) * buf_size ); 

	/* check for error */
	if(is.fail())
		return -1;

	/* success */
	return 0;
}
		
/* The D-Imager reader class function implementations */

d_imager_reader_t::d_imager_reader_t()
{
	/* set default values */
	this->image_width = 0;
	this->image_height = 0;
	this->fps = -1;
	this->freq = -1;
	this->read_so_far = 0;
	this->num_scans = 0;
}

d_imager_reader_t::~d_imager_reader_t()
{
	/* close file if necessary */
	this->close();
}

int d_imager_reader_t::open(const std::string& filename)
{
	char magic[MAGIC_NUMBER_LENGTH];

	/* close any open files */
	this->close();

	/* attempt to open the file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
		return -1;

	/* read the magic number from the file */
	this->infile.read(magic, MAGIC_NUMBER_LENGTH);

	/* check if valid D-Imager data file */
	if(strcmp(magic, MAGIC_NUMBER_VALUE))
		return -2;

	/* read header metadata */
	this->infile.read((char*) (&(this->image_width)), 
	                  sizeof(unsigned int) );
	this->infile.read((char*) (&(this->image_height)),
	                  sizeof(unsigned int) );
	this->infile.read((char*) (&(this->fps)), sizeof(int) );
	this->infile.read((char*) (&(this->freq)), sizeof(int) );
	this->infile.read((char*) (&(this->num_scans)),
	                  sizeof(unsigned int) );

	/* initialize counter */
	this->read_so_far = 0;

	/* success */
	return 0;
}
		
int d_imager_reader_t::next(d_imager_frame_t& frame)
{
	int ret;

	/* check the resolution */
	if(frame.image_width != this->image_width
			|| frame.image_height != this->image_height)
		frame.init_resolution(this->image_width, 
		                      this->image_height);

	/* read the next frame from file */
	ret = frame.parse(this->infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* record the index of this frame */
	frame.index = this->read_so_far;
	this->read_so_far++;

	/* success */
	return 0;
}

bool d_imager_reader_t::eof() const
{
	if(this->infile.is_open())
		return this->infile.eof();
	return true;
}

void d_imager_reader_t::close()
{
	/* check if file open, and close it if so */
	if(this->infile.is_open())
		this->infile.close();
}
