#include "chunk_io.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <string>
#include <vector>
#include <util/error_codes.h>

/**
 * @file chunk_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * Implements the reader and writer classes for the .chunk and .chunklist
 * file formats.
 */

/* use the following namespaces for the implementations in this file */
using namespace std;
using namespace chunk;

/* function implementations for header_t */

chunklist_header_t::chunklist_header_t()
{
	/* set all parameters to blank (invalid) */
	this->init(0,0,0,-1, /* center position and halfwidth (meters) */ 
	           "", 0); /* chunk subdirectory and num chunks */
}
			
void chunklist_header_t::init(double cx, double cy, double cz, 
                              double hw, const std::string& cd, 
                              size_t nc)
{
	/* set all values in the header to either default values or
	 * to the provided values */
	this->center_x = cx;
	this->center_y = cy;
	this->center_z = cz;
	this->halfwidth = hw;
	this->chunk_dir = cd;
	this->num_chunks = nc;
	this->sensor_names.clear();
}
			
int chunklist_header_t::parse(istream& infile)
{
	stringstream parser;
	string tline, tag, val;

	/* check validity of input */
	if(infile.fail())
		return -1;

	/* look for magic number */
	getline(infile, tline);
	if(tline.compare(CHUNKLIST_MAGIC_NUMBER))
	{
		cerr << "[chunk::chunklist_header_t::parse]\t"
		     << "Attempting to parse "
		     << "stream that is not valid .chunklist format"
		     << endl;
		return -2; /* not a valid chunklist file,
		            * magic numbers differ */
	}

	/* attempt to parse a .chunk header from the open file stream */
	while(infile.good())
	{
		/* get the next line from the file */
		getline(infile, tline);
		parser.clear();
		parser.str(tline);
		parser >> tag;

		/* parse this line */
		if(!tline.compare(END_HEADER_STRING))
			break; /* end of header */
		else if(!tag.compare(HEADER_TAG_CENTER))
		{
			/* store center position for volume */
			parser >> (this->center_x);
			parser >> (this->center_y);
			parser >> (this->center_z);
		}
		else if(!tag.compare(HEADER_TAG_HALFWIDTH))
		{
			/* get the format string */
			parser >> (this->halfwidth);
		}
		else if(!tag.compare(HEADER_TAG_NUM_CHUNKS))
		{
			/* store the number of chunks refernced in file */
			parser >> (this->num_chunks);
		}
		else if(!tag.compare(HEADER_TAG_CHUNK_DIR))
		{
			/* store the chunk subdirectory path */
			parser >> (this->chunk_dir);

			/* make sure it has a trailing '/' */
			if(!this->chunk_dir.empty() 
					&& (*(this->chunk_dir.rbegin()))
						!= FILE_SEPERATOR)
				this->chunk_dir.push_back(FILE_SEPERATOR);
		}
		else if(!tag.compare(HEADER_TAG_SENSOR))
		{
			/* store a new sensor */
			parser >> val;
			this->sensor_names.push_back(val);
		}
		else
		{
			/* unknown tag */
			cerr << "[chunk::chunklist_header_t::parse]\t"
			     << "Unknown header "
			     << "tag found: " << tline << endl;
			return -3;
		}
	}

	/* check quality of stream */
	if(infile.bad())
	{
		/* didn't finish header gracefully */
		cerr << "[chunk::chunklist_header_t::parse]\t"
		     << "Header incomplete!" << endl;
		return -4;
	}
	if(this->halfwidth <= 0)
	{
		/* halfwidth must be positive */
		cerr << "[chunk::chunklist_header_t::parse]\t"
		     << "Parsed halfwidth invalid: " << this->halfwidth
		     << endl;
		return -5;
	}
	if(this->num_chunks == 0)
	{
		/* must has some chunks! */
		cerr << "[chunk::chunklist_header_t::parse]\t"
		     << "No chunks listed in header"
		     << endl;
		return -6;
	}
	if(this->sensor_names.empty())
	{
		/* must have some sensors! */
		cerr << "[chunk::chunklist_header_t::parse]\t"
		     << "No sensors listed in header" << endl;
		return -7;
	}

	/* success */
	return 0;
}
			
void chunklist_header_t::print(ostream& outfile) const
{
	unsigned int i, n;

	/* print current header information to stream */
	outfile << CHUNKLIST_MAGIC_NUMBER << endl
	   << HEADER_TAG_CENTER << " "
	   << this->center_x << " "
	   << this->center_y << " "
	   << this->center_z << endl
	   << HEADER_TAG_HALFWIDTH << " "
	   << this->halfwidth << endl
	   << HEADER_TAG_NUM_CHUNKS << " "
	   << this->num_chunks << endl
	   << HEADER_TAG_CHUNK_DIR << " "
	   << this->chunk_dir << endl;

	/* print all sensors */
	n = this->sensor_names.size();
	for(i = 0; i < n; i++)
		outfile << HEADER_TAG_SENSOR << " "
		        << this->sensor_names[i] << endl;

	/* close the header */
	outfile << END_HEADER_STRING << endl;
}

/*--------------------*/
/* chunklist_reader_t */
/*--------------------*/

chunklist_reader_t::chunklist_reader_t()
{
	/* no processing necessary to set up */
}

chunklist_reader_t::~chunklist_reader_t()
{
	/* when destroying this object, make sure
	 * to close any open streams */
	this->infile.close();
}

void chunklist_reader_t::close()
{
	/* check if stream is open.  If so, then close it */
	if(this->infile.is_open())
		this->infile.close();
}

int chunklist_reader_t::open(const std::string& filename)
{
	int ret;

	/* close any open files */
	this->close();
	
	/* open file for reading, and parse the header */
	this->infile.open(filename.c_str());
	if(!(this->infile.is_open()))
	{
		cerr << "[chunklist_reader_t::open]\tUnable to locate "
		     << "file: " << filename << endl;
		return -1; /* unable to open file */
	}

	/* attempt to parse the header */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* unable to parse header */
		cerr << "[chunklist_reader_t::open]\tUnable to parse "
		     << "file header: " << filename << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* successfully opened file and parsed the header.  Now it is
	 * in a state to read the list of uuids */
	return 0;
}

size_t chunklist_reader_t::num_chunks() const
{
	return this->header.num_chunks;
}
			
int chunklist_reader_t::next(std::string& file)
{
	string uuid;

	/* check if we reached the end of the file yet */
	if(this->infile.eof())
		return -1;

	/* get the next line of the file */
	getline(this->infile, uuid);

	/* verify that retrieved id is reasonable */
	if(uuid.empty())
		return -2;

	/* populate the file location */
	file = this->header.chunk_dir + uuid + CHUNKFILE_EXTENSION;

	/* success */
	return 0;
}

/*--------------------*/
/* chunklist_writer_t */
/*--------------------*/

chunklist_writer_t::chunklist_writer_t()
{
	/* initialize parameters */
	this->chunks_written_so_far = 0;
}

chunklist_writer_t::~chunklist_writer_t()
{
	/* if any streams are open, close them */
	this->close();
}

void chunklist_writer_t::init(double cx, double cy, double cz, 
                              double hw, const std::string& cd, 
                              size_t nc)
{
	/* initialize header object */
	this->header.init(cx, cy, cz, hw, cd, nc);
}
			
int chunklist_writer_t::open(const std::string& filename)
{
	/* check if the header has been initialized */
	if(this->header.num_chunks == 0 || this->header.halfwidth <= 0)
	{
		/* you need to initialze before opening file */
		cerr << "[chunklist_writer_t::open]\t"
		     << "This writer has not yet been initialized,"
		     << " cannot write out to file." << endl;
		return -1;
	}
	
	/* close any open streams */
	this->close();
	
	/* attempt to open this file for writing */
	this->outfile.open(filename.c_str());
	if(!(this->outfile.is_open()))
	{
		/* could not open file for writing */
		cerr << "[chunklist_writer_t::open]\t"
		     << "Unable to open file for writing: "
		     << filename << endl;
		return -2;
	}

	/* now that we've opened the stream, attempt to
	 * write the header information.  Note that the
	 * header should be initialized at this point. */
	this->header.print(this->outfile);

	/* success */
	return 0;
}

void chunklist_writer_t::write(const std::string& uuid)
{
	/* write the uuid on its own line */
	this->outfile << uuid << endl;
	this->chunks_written_so_far++;
}
			
void chunklist_writer_t::close()
{
	/* if we have any streams open, close them */
	if(this->outfile.is_open())
		this->outfile.close();
	this->chunks_written_so_far = 0;
}

// TODO
