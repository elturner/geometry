#include "wedge_io.h"
#include <geometry/shapes/carve_wedge.h>
#include <util/error_codes.h>
#include <fstream>
#include <string>
#include <string.h>
#include <iostream>
#include <mutex> /* this needs to use g++ flag: -std=c++0x */

/**
 * @file wedge_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Provides i/o classes for carve_wedge_t lists
 *
 * @section DESCRIPTION
 *
 * This file contains classes that are used to import and export
 * carve_wedge_t objects from file.
 *
 * Make sure to compile with -std=c++0x to support C++11 standard.
 */

using namespace std;
using namespace wedge;

/*-----------------------------------*/
/* header_t function implementations */
/*-----------------------------------*/

header_t::header_t()
{
	this->num_wedges = 0;
}
			
int header_t::parse(istream& is)
{
	char m[MAGIC_NUMBER_SIZE];

	/* check for magic number */
	is.read(m, MAGIC_NUMBER_SIZE);
	if(memcmp(m, MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE))
	{
		/* wrong magic number */
		cerr << "[wedge::header_t::parse]\t"
		     << "Input file is not a valid .wedge file" << endl;
		return -1;
	}

	/* parse the input binary stream,
	 * all values are assumed to be in little-endian ordering */
	is.read((char*) &(this->num_wedges), sizeof(this->num_wedges));

	/* check file stream */
	if(is.bad())
	{
		/* could not finish header */
		cerr << "[wedge::header_t::parse]\t"
		     << "Unable to parse header information" << endl;
		return -2;
	}

	/* success */
	return 0;
}
			
void header_t::print(ostream& os) const
{
	/* write magic number */
	os.write(MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE);

	/* write values */
	os.write((char*) &(this->num_wedges), sizeof(this->num_wedges));
}

/*-----------------------------------*/
/* reader_t function implementations */
/*-----------------------------------*/

reader_t::~reader_t()
{
	/* close file if necessary */
	this->close();
}

int reader_t::open(const string& filename)
{
	int ret;

	/* close any open streams */
	this->close();
	
	/* lock the mutex, since we're going to edit the stream */
	this->mtx.lock();
	
	/* attempt to open binary file stream */
	this->infile.open(filename.c_str(),
			ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
	{
		/* could not open file */
		cerr << "[wedge::reader_t::open]\t"
		     << "Unable to open file for reading: "
		     << filename << endl;
		this->mtx.unlock();
		return -1;
	}

	/* read in header information */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* could not read header */
		cerr << "[wedge::reader_t::open]\t"
		     << "Unable to parse header from file: "
		     << filename << endl;
		this->infile.close();
		this->mtx.unlock();
		return -2;
	}

	/* success */
	this->mtx.unlock();
	return 0;
}

int reader_t::get(carve_wedge_t& w, unsigned int i)
{
	int ret;
	streampos p;

	/* prepare to access the stream, which requires locking
	 * the mutex */
	this->mtx.lock();

	/* check if the file is open */
	if(!(this->infile.is_open()))
	{
		this->mtx.unlock();
		return -1;
	}

	/* check if index is valid */
	if(i >= this->header.num_wedges)
	{
		this->mtx.unlock();
		return -2;
	}

	/* move file stream to appropriate position */
	p = wedge::HEADER_SIZE + (((size_t) i) * wedge::WEDGE_SIZE);
	this->infile.seekg(p);

	/* parse the wedge from the stream */
	ret = w.parse(this->infile);
	if(ret)
	{
		this->mtx.unlock();
		return PROPEGATE_ERROR(-3, ret);
	}

	/* success */
	this->mtx.unlock();
	return 0;
}
			
void reader_t::close()
{
	/* check if this file is open */
	if(this->infile.is_open())
	{
		this->infile.close(); /* close it */
		this->header.num_wedges = 0; /* reset values */
	}
}

/*------------------------------------------------*/
/* function implementaions for the writer_t class */
/*------------------------------------------------*/

writer_t::~writer_t()
{
	/* close file on deletion */
	this->close();
}

int writer_t::open(const std::string& filename)
{
	/* close streams if necessary */
	this->close();

	/* attempt to open this binary file for writing */
	this->outfile.open(filename.c_str(),
			ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* unable to open file for writing */
		cerr << "[wedge::writer_t::open]\t"
		     << "Unable to open file for writing: "
		     << filename << endl;
		return -1;
	}

	/* attempt to write header information (as a place-holder) */
	this->header.print(this->outfile);

	/* success */
	return 0;
}
			
void writer_t::write(const carve_wedge_t& w)
{	
	/* write the given wedge */
	w.serialize(this->outfile);
	this->header.num_wedges++;
}

void writer_t::close()
{
	/* check if the file is even open */
	if(!(this->outfile.is_open()))
		return; /* do nothing, file isn't open */

	/* export the latest number of wedges to the header */
	this->outfile.seekp(0); /* seek to the beginning */
	this->header.print(this->outfile); /* rewrite header */

	/* close the file */
	this->outfile.close();
}

