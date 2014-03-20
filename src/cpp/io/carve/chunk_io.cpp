#include "chunk_io.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <string>
#include <string.h>
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
		this->infile.close();
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

/*--------------------------*/
/* chunk_header_t functions */
/*--------------------------*/
			
chunk_header_t::chunk_header_t()
{
	/* set default values */
	this->uuid = 0;
	this->center_x = 0.0;
	this->center_y = 0.0;
	this->center_z = 0.0;
	this->halfwidth = -1;
	this->num_points = 0;
}

void chunk_header_t::init(unsigned long long u,
                          double cx, double cy, double cz,
                          double hw)
{
	/* set given values */
	this->uuid = u;
	this->center_x = cx;
	this->center_y = cy;
	this->center_z = cz;
	this->halfwidth = hw;
	this->num_points = 0; /* initialize to no points */
}
			
int chunk_header_t::parse(std::istream& is)
{
	char m[CHUNKFILE_MAGIC_NUMBER_SIZE];
	
	/* check for magic number */
	is.read(m, CHUNKFILE_MAGIC_NUMBER_SIZE);
	if(memcmp(m, CHUNKFILE_MAGIC_NUMBER.c_str(),
			CHUNKFILE_MAGIC_NUMBER_SIZE))
	{
		/* wrong magic number */
		cerr << "[chunk::chunk_header_t::parse]\t"
		     << "Input file is not a valid .chunk file" << endl;
		return -1;
	}

	/* parse the input binary stream,
	 * all values are assumed to be in little-endian ordering */
	is.read((char*) &(this->uuid),       sizeof(this->uuid));
	is.read((char*) &(this->center_x),   sizeof(this->center_x));
	is.read((char*) &(this->center_y),   sizeof(this->center_y));
	is.read((char*) &(this->center_z),   sizeof(this->center_z));
	is.read((char*) &(this->halfwidth),  sizeof(this->halfwidth));
	is.read((char*) &(this->num_points), sizeof(this->num_points));

	/* check file stream */
	if(is.bad())
	{
		/* could not finish header */
		cerr << "[chunk::chunk_header_t::parse]\t"
		     << "Unable to parse header information" << endl;
		return -2;
	}

	/* success */
	return 0;
}

void chunk_header_t::print(std::ostream& os) const
{
	/* write magic number */
	os.write(CHUNKFILE_MAGIC_NUMBER.c_str(),
	         CHUNKFILE_MAGIC_NUMBER_SIZE);

	/* write values */
	os.write((char*) &(this->uuid),       sizeof(this->uuid));
	os.write((char*) &(this->center_x),   sizeof(this->center_x));
	os.write((char*) &(this->center_y),   sizeof(this->center_y));
	os.write((char*) &(this->center_z),   sizeof(this->center_z));
	os.write((char*) &(this->halfwidth),  sizeof(this->halfwidth));
	os.write((char*) &(this->num_points), sizeof(this->num_points));
}
			
void chunk_header_t::write_num_points(std::ostream& os,
                                      unsigned int np)
{
	/* save the number of points */
	this->num_points = np;

	/* go back to start of file, and rewrite header */
	os.seekp(ios_base::beg);
	this->print(os);
}

/*--------------------------*/
/* chunk_reader_t functions */
/*--------------------------*/

chunk_reader_t::chunk_reader_t()
{
	/* No parameters need to be explicitly set */
}

chunk_reader_t::~chunk_reader_t()
{
	/* close the file if it is open */
	this->close();
}
			
int chunk_reader_t::open(const std::string& filename)
{
	int ret;

	/* close any open streams */
	this->close();

	/* attempt to open binary file stream */
	this->infile.open(filename.c_str(),
			ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
	{
		/* could not open file */
		cerr << "[chunk::chunk_reader_t::open]\t"
		     << "Unable to open file for reading: "
		     << filename << endl;
		return -1;
	}

	/* read in header information */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* could not read header */
		cerr << "[chunk::chunk_reader_t::open]\t"
		     << "Unable to parse header from file: "
		     << filename << endl;
		this->infile.close();
		return -2;
	}

	/* success */
	return 0;
}
			
int chunk_reader_t::next(point_index_t& i)
{
	/* check if we're good */
	if(this->infile.bad())
		return -1; /* not good */

	/* read next point */
	i.parse(this->infile);
	
	/* success */
	return 0;
}
			
void chunk_reader_t::close()
{
	/* check if streams are open.  If so, close them */
	if(this->infile.is_open())
		this->infile.close();
}

/*--------------------------*/
/* chunk_writer_t functions */
/*--------------------------*/

chunk_writer_t::chunk_writer_t()
{
	/* set defaults */
	this->num_written_so_far = 0;
}
			
chunk_writer_t::~chunk_writer_t()
{
	/* close the file if it is open */
	this->close();
}
			
void chunk_writer_t::init(unsigned long long uuid,
                          double cx, double cy, double cz,
                          double hw)
{
	/* initialize this writer to have these values */
	this->header.init(uuid, cx, cy, cz, hw);
}
			
int chunk_writer_t::open(const std::string& filename)
{
	/* close streams if necessary */
	this->close();
	this->num_written_so_far = 0;

	/* attempt to open this binary file for writing */
	this->outfile.open(filename.c_str(),
			ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* unable to open file for writing */
		cerr << "[chunk::chunk_writer_t::open]\t"
		     << "Unable to open file for writing: "
		     << filename << endl;
		return -1;
	}

	/* attempt to write header information (as a place-holder) */
	this->header.print(this->outfile);

	/* success */
	return 0;
}
			
void chunk_writer_t::write(const point_index_t& i)
{
	/* write the given point */
	i.print(this->outfile);
	this->num_written_so_far++;
}
			
void chunk_writer_t::close()
{
	/* check if stream is even open */
	if(!(this->outfile.is_open()))
		return; /* don't need to do anything */

	/* since this stream is open, we should update the
	 * number of points written */
	this->header.write_num_points(this->outfile,
	                              this->num_written_so_far);

	/* close the file */
	this->outfile.close();
}

/*-------------------------*/
/* point_index_t functions */
/*-------------------------*/

point_index_t::point_index_t()
{
	/* set default values */
	this->sensor_index = 0;
	this->frame_index  = 0;
	this->point_index  = 0;
}

point_index_t::point_index_t(unsigned int si,
                             unsigned int fi,
                             unsigned int pi)
{
	/* set given values */
	this->sensor_index = si;
	this->frame_index = fi;
	this->point_index = pi;
}
			
void point_index_t::parse(std::istream& is)
{
	/* parse the given stream for points */
	is.read((char*) &(this->sensor_index), sizeof(this->sensor_index));
	is.read((char*) &(this->frame_index),  sizeof(this->frame_index ));
	is.read((char*) &(this->point_index),  sizeof(this->point_index ));
}

void point_index_t::print(std::ostream& os) const
{
	/* print the point info */
	os.write((char*) &(this->sensor_index), sizeof(this->sensor_index));
	os.write((char*) &(this->frame_index),  sizeof(this->frame_index ));
	os.write((char*) &(this->point_index),  sizeof(this->point_index ));
}
