#include "tango_io.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * @file   tango_io.cpp
 * @author Eric Turner <elturner@indoorreality.com>
 * @brief  Parses tango .dat files
 *
 * @section DESCRIPTION
 *
 * The tango_io namespace contains classes used to parse and
 * represent the data products stored in the .dat files generated
 * from the Google Tango data collection application.
 */

using namespace std;
using namespace tango_io;

/*-----------------------------------------------*/
/* the following constants are used in this file */
/*-----------------------------------------------*/

/* the magic number at the beginning of a valid tango file */ 
#define TANGO_MAGIC_NUMBER 74960

/*--------------------------*/
/* function implementations */
/*--------------------------*/

tango_reader_t::tango_reader_t()
{
	/* reset values */
	this->current_index = 0;
	this->frame_locs.clear();
}

tango_reader_t::tango_reader_t(const std::string& filename)
{
	/* reset values */
	this->current_index = 0;
	this->frame_locs.clear();

	/* attempt to open file, ignoring return value */
	this->open(filename);
}
			
tango_reader_t::~tango_reader_t()
{
	/* close file if open */
	this->close();
}

int tango_reader_t::open(const std::string& filename)
{
	tango_frame_t frame;
	int magic, ret;

	/* close any files if open */
	this->close();

	/* attempt to open the specified file */
	this->infile.open(filename.c_str(), ifstream::binary);
	if(!(this->infile.is_open()))
	{
		/* can't find file */
		cerr << "[tango_reader_t::open]\tUnable to access "
		     << "tango data file.  Make sure it is present "
		     << "with readbale permissions.  File name: \""
		     << filename << "\"" << endl;
		return -1;
	}

	/* read the magic number from the file (little-endian) */
	magic = this->read_int();
	if(magic != TANGO_MAGIC_NUMBER)
	{
		/* bad magic number */
		cerr << "[tango_reader_t::open]\tInvalid magic number "
		     << "detected (" << magic << ").  Are you sure this "
		     << "is a tango data file?  \"" << filename << "\""
		     << endl;
		this->close();
		return -2;
	}

	/* the next character should be ignored */
	this->read_char();

	/* make sure the input file is still good */
	if(this->infile.fail())
	{
		/* i/o error occurred */
		cerr << "[tango_reader_t::open]\tUnable to successfully "
		     << "read header of input tango data file: \""
		     << filename << "\"" << endl;
		this->close();
		return -3;
	}

	/* populate the frame locations within the file, 
	 * for random access */
	while(!(this->eof()))
	{
		/* save the frame position */
		this->frame_locs.push_back(this->infile.tellg());

		/* get the next frame */
		ret = this->next(frame);
		if(ret)
		{
			/* first, check for eof */
			if(this->eof())
			{
				/* remove last position from frame_locs
				 * list, since it's actually the end
				 * of the file */
				this->frame_locs.pop_back();
				break;
			}

			/* an error occurred */
			cerr << "[tango_reader_t::open]\tUnable to parse "
			     << "frame #" << this->current_index
			     << " (Error " << ret << ") "
			     << "in file: \"" << filename << "\"" << endl;
			this->close();
			return -4;
		}
	}

	/* reset to beginning of frames */
	this->current_index = 0;
	this->infile.clear();
	this->infile.seekg(this->frame_locs[0]);

	/* success, ready to start reading frames */
	return 0;
}
			
int tango_reader_t::get(size_t i, tango_frame_t& frame)
{
	/* check that i is a valid index */
	if(i >= this->frame_locs.size())
	{
		cerr << "[tango_reader_t::get]\tGiven invalid index: "
		     << i << ".  There are " << this->frame_locs.size()
		     << " frame(s) total." << endl;
		return -1;
	}

	/* move to position #i */
	this->current_index = i;
	this->infile.seekg(this->frame_locs[i]);

	/* get the next frame */
	return this->next(frame);
}
			
int tango_reader_t::next(tango_frame_t& frame)
{
	int i, bufsize, num_points;

	/* check if file is open */
	if(!(this->is_open()))
	{
		/* can't read from a file that's not open */
		cerr << "[tango_reader_t::next]\tAttempted to read frame "
		     << "from unopened file." << endl;
		return -1;
	}

	/* prepare the frame */
	frame.index = this->current_index;

	/* read the pose data from file */
	frame.timestamp = this->read_double();
	if(this->eof())
		return -2; /* don't print error for eof */

	frame.position[0] = this->read_double();
	frame.position[1] = this->read_double();
	frame.position[2] = this->read_double();

	frame.quaternion[0] = this->read_double();
	frame.quaternion[0] = this->read_double();
	frame.quaternion[0] = this->read_double();
	frame.quaternion[0] = this->read_double();
	
	/* get the size of the buffer (in bytes) of the depth points */
	bufsize = this->read_int();
	if(this->infile.fail())
	{
		/* error occurred during i/o */
		cerr << "[tango_frame_t::read]\tFile i/o error occurred "
		     << "when reading frame pose data." << endl;
		return -3;
	}

	/* compute the number of points */
	num_points = bufsize / (3 * sizeof(float));

	/* read in the points */
	frame.points.resize(num_points);
	for(i = 0; i < num_points; i++)
	{
		/* read in the next point as three floats
		 *
		 * NOTE:  these floats are stored in their original
		 * little-endian format, NOT in Java's DataInput
		 * format. */
		this->infile.read((char*) (&(frame.points[i].x)), 
					sizeof(frame.points[i].x)); 
		this->infile.read((char*) (&(frame.points[i].y)), 
					sizeof(frame.points[i].y)); 
		this->infile.read((char*) (&(frame.points[i].z)), 
					sizeof(frame.points[i].z)); 
	}

	/* check if the file is still in good condition */
	if(this->infile.fail())
	{
		/* error occurred during i/o */
		cerr << "[tango_frame_t::read]\tFile i/o error occurred "
		     << "when reading frame depth data." << endl;
		return -4;
	}

	/* success */
	this->current_index++;
	return 0;
}

void tango_reader_t::close()
{
	/* check if there's actually a file to close */
	if(!(this->is_open()))
		return;

	/* close the file */
	this->infile.close();

	/* reset the counter */
	this->current_index = 0;
	this->frame_locs.clear();
}
			
char tango_reader_t::read_char()
{
	char b1, b2;

	/* in Java's DataInput, each character is represented
	 * by two bytes. */
	this->infile.get(b1);
	this->infile.get(b2);

	/* return the resulting character (which is really just the
	 * second character */
	return (char) ( (b1 << 8) | (b2 & 0xff) );
}

int tango_reader_t::read_int()
{
	char a, b, c, d;

	/* in Java's DataInput, each integer is represented by
	 * four bytes */
	this->infile.get(a);
	this->infile.get(b);
	this->infile.get(c);
	this->infile.get(d);

	/* reconstruct the integer based on the endianness described
	 * in Java's InputData */
	return (((a & 0xff) << 24) | ((b & 0xff) << 16) |
			  ((c & 0xff) << 8) | (d & 0xff));
}

double tango_reader_t::read_double()
{
	union
	{
		long lon;
		double dub;
	} conv;
	char a, b, c, d, e, f, g, h;

	/* in Java's DataInput, each double is represented by
	 * eight bytes */
	this->infile.get(a);
	this->infile.get(b);
	this->infile.get(c);
	this->infile.get(d);
	this->infile.get(e);
	this->infile.get(f);
	this->infile.get(g);
	this->infile.get(h);

	/* reconstruct the double */
	conv.lon =     (((long)(a & 0xff) << 56) |
                        ((long)(b & 0xff) << 48) |
                        ((long)(c & 0xff) << 40) |
                        ((long)(d & 0xff) << 32) |
                        ((long)(e & 0xff) << 24) |
                        ((long)(f & 0xff) << 16) |
                        ((long)(g & 0xff) <<  8) |
                        ((long)(h & 0xff)      ));
	return conv.dub;
}

