#include "hia_io.h"
#include <util/error_codes.h>
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>

/**
 * @file   hia_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Reader/writer for the .hia file format.
 *
 * @section DESCRIPTION
 *
 * The .hia (Histogrammed Interior Area) file format stores
 * a top-down 2D histogram of a building model's interior volume.
 *
 * This file stores the local floor and ceiling heights
 * for each 2D cell, as well as the amount of interior height
 * occurs within that cell.
 *
 * It is intended to be used for the generation of building floorplans
 *
 * Created February 25, 2015
 */

using namespace std;
using namespace hia;

/*-----------------------------------*/
/* header_t function implementations */ 
/*-----------------------------------*/

void header_t::init(int levind, unsigned int num, 
				double xmin, double ymin, double zmin,
				double xmax, double ymax, double zmax,
				double res)
{
	/* set versions to be the latest values */
	this->version_major = VERSION_MAJOR;
	this->version_minor = VERSION_MINOR;

	/* copy input arguments */
	this->level_index   = levind;
	this->num_cells     = num;
	this->x_min         = xmin;
	this->y_min         = ymin;
	this->z_min         = zmin;
	this->x_max         = xmax;
	this->y_max         = ymax;
	this->z_max         = zmax;
	this->resolution    = res;
}
			
bool header_t::is_valid() const
{
	/* check if any values are invalid */
	if(this->resolution < 0)
		return false;

	/* check if invalid version */
	if(this->version_major != VERSION_MAJOR)
		return false;

	/* check if bounding box invalid */
	if( this->x_min > this->x_max
			|| this->y_min > this->y_max
			|| this->z_min > this->z_max )
		return false;

	/* this is valid */
	return true;
}

int header_t::parse(std::istream& is)
{
	char magic[MAGIC_NUMBER_SIZE];

	/* attempt to read the magic number from the input stream */
	is.read(magic, MAGIC_NUMBER_SIZE);
	if(strcmp(magic, MAGIC_NUMBER.c_str()))
	{
		/* wrong magic number */
		cerr << "[hia::header_t::parse]\tError!  Input file is "
		     << "not a valid .hia file." << endl;
		return -1;
	}

	/* read in the version number from the file */
	is.read((char*) &(this->version_major),
			sizeof(this->version_major));
	is.read((char*) &(this->version_minor),
			sizeof(this->version_minor));

	/* check if version is appropriate */
	if(this->version_major < VERSION_MAJOR)
	{
		cerr << "[hia::header_t::parse]\tFile is outdated version!"
		     << endl
		     << "\tfile: Version " << this->version_major
		     << "." << this->version_minor << endl
		     << "\tCode: Version " << VERSION_MAJOR
		     << "." << VERSION_MINOR << endl;
		return -2;
	}
	if(this->version_major > VERSION_MAJOR)
	{
		cerr << "[hia::header_t::parse]\tFile is using newer "
		     << "format version than the code.  Are you using "
		     << "latest version of code?" << endl
		     << "\tfile: Version " << this->version_major
		     << "." << this->version_minor << endl
		     << "\tCode: Version " << VERSION_MAJOR
		     << "." << VERSION_MINOR << endl;
		return -3;
	}

	/* read in data */
	is.read((char*) &(this->level_index), sizeof(this->level_index));
	is.read((char*) &(this->num_cells),   sizeof(this->num_cells));
	is.read((char*) &(this->x_min),       sizeof(this->x_min));
	is.read((char*) &(this->y_min),       sizeof(this->y_min));
	is.read((char*) &(this->z_min),       sizeof(this->z_min));
	is.read((char*) &(this->x_max),       sizeof(this->x_max));
	is.read((char*) &(this->y_max),       sizeof(this->y_max));
	is.read((char*) &(this->z_max),       sizeof(this->z_max));
	is.read((char*) &(this->resolution),  sizeof(this->resolution));
	
	/* check that information is valid */
	if(!(this->is_valid()))
	{
		cerr << "[hia::header_t::parse]\tHeader info invalid!"
		     << endl;
		return -4;
	}

	/* check that stream is still good */
	if(!(is.good()))
	{
		/* bad stream */
		cerr << "[hia::header_t::parse]\tError!  Could not "
		     << "read header completely from file." << endl;
		return -5;
	}

	/* success */
	return 0;
}
			
int header_t::serialize(std::ostream& os) const
{
	/* check that stream is good */
	if(!(os.good()))
	{
		cerr << "[hia::header_t::serialize]\tCan't write out!"
		     << endl;
		return -1;
	}

	/* write info */
	os.write(MAGIC_NUMBER.c_str(), MAGIC_NUMBER_SIZE);
	os.write((char*) &(this->version_major),
			sizeof(this->version_major));
	os.write((char*) &(this->version_minor),
			sizeof(this->version_minor));
	os.write((char*) &(this->level_index), sizeof(this->level_index));
	os.write((char*) &(this->num_cells),   sizeof(this->num_cells));
	os.write((char*) &(this->x_min),       sizeof(this->x_min));
	os.write((char*) &(this->y_min),       sizeof(this->y_min));
	os.write((char*) &(this->z_min),       sizeof(this->z_min));
	os.write((char*) &(this->x_max),       sizeof(this->x_max));
	os.write((char*) &(this->y_max),       sizeof(this->y_max));
	os.write((char*) &(this->z_max),       sizeof(this->z_max));
	os.write((char*) &(this->resolution),  sizeof(this->resolution));

	/* check that stream is still good */
	if(!(os.good()))
	{
		cerr << "[hia::header_t::serialize]\tStream went bad!"
		     << endl;
		return -2;
	}
	
	/* success */
	return 0;
}

/*---------------------------------*/
/* cell_t function implementations */ 
/*---------------------------------*/

int cell_t::parse(std::istream& is)
{
	/* check that stream is good */
	if(!(is.good()))
	{
		cerr << "[hia::cell_t::parse]\tGiven bad input stream!"
		     << endl;
		return -1;
	}

	/* read info */
	is.read((char*) &(this->center_x),    sizeof(this->center_x));
	is.read((char*) &(this->center_y),    sizeof(this->center_y));
	is.read((char*) &(this->min_z),       sizeof(this->min_z));
	is.read((char*) &(this->max_z),       sizeof(this->max_z));
	is.read((char*) &(this->open_height), sizeof(this->open_height));

	/* check validity of input */
	if(this->open_height < 0 || this->min_z > this->max_z)
	{
		cerr << "[hia::cell_t::parse]\tCell info not valid!"
		     << endl;
		return -2;
	}

	/* success  */
	return 0;
}

int cell_t::serialize(std::ostream& os) const
{
	/* check that stream is good */
	if(!(os.good()))
	{
		cerr << "[hia::cell_t::serialize]\tCan't write out!"
		     << endl;
		return -1;
	}

	/* write info */
	os.write((char*) &(this->center_x),    sizeof(this->center_x));
	os.write((char*) &(this->center_y),    sizeof(this->center_y));
	os.write((char*) &(this->min_z),       sizeof(this->min_z));
	os.write((char*) &(this->max_z),       sizeof(this->max_z));
	os.write((char*) &(this->open_height), sizeof(this->open_height));

	/* check that stream is still good */
	if(!(os.good()))
	{
		cerr << "[hia::cell_t::serialize]\tStream went bad!"
		     << endl;
		return -2;
	}
	
	/* success */
	return 0;
}

/*-----------------------------------*/
/* reader_t function implementations */ 
/*-----------------------------------*/

int reader_t::open(const std::string& filename)
{
	int ret;

	/* first, close any open streams */
	this->close();

	/* next, attempt to open file as a binary stream */
	this->infile.open(filename.c_str(), 
			ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
	{
		/* could not open file */
		cerr << "[hia::reader_t::open]\tUnable to open file "
		     << "for reading:  " << filename << endl;
		return -1;
	}

	/* parse the header */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		cerr << "[hia::reader_t::open]\tUnable to read header of "
		     << "file:  " << filename << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	return 0;
}

int reader_t::next(cell_t& cell)
{
	int ret;

	/* check if the file is open */
	if(!(this->infile.is_open()))
	{
		cerr << "[hia::reader_t::next]\tFile is not open!" << endl;
		return -1;
	}

	/* parse the next cell from the file */
	ret = cell.parse(this->infile);
	if(ret)
	{
		cerr << "[hia::reader_t::next]\tUnable to get next cell"
		     << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	return 0;
}
			
void reader_t::close()
{
	/* just close the file stream if it is open */
	if(this->infile.is_open())
		this->infile.close();
}

/*-----------------------------------*/
/* writer_t function implementations */ 
/*-----------------------------------*/

int writer_t::open(const std::string& filename, double res, int level,
					double minz, double maxz)
{
	int ret;

	/* first, close any open streams */
	this->close();

	/* next, populate the header */
	this->header.init(level, 0, /* no cells yet */
			1, 1, minz, 0, 0, maxz, /* invalid bounding box */
			res); /* resolution of cells */

	/* next, attempt to open file as a binary stream */
	this->outfile.open(filename.c_str(), 
			ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
	{
		/* could not open file */
		cerr << "[hia::writer_t::open]\tUnable to open file "
		     << "for writing:  " << filename << endl;
		return -1;
	}

	/* export the header
	 *
	 * Note that we will rewrite the header info after writing
	 * all the data, but for now we need to fill the space */
	ret = this->header.serialize(this->outfile);
	if(ret)
	{
		cerr << "[hia::writer_t::open]\tUnable to read header of "
		     << "file:  " << filename << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	return 0;
}

int writer_t::write(const cell_t& cell)
{
	int ret;

	/* check file stream */
	if(!(this->outfile.is_open()))
	{
		cerr << "[hia::writer_t::write]\tFile not open!" << endl;
		return -1;
	}

	/* write this cell */
	ret = cell.serialize(this->outfile);
	if(ret)
	{
		cerr << "[hia::writer_t::write]\tUnable to write cell"
		     << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* update count of cells */
	this->header.num_cells++;

	/* update bounding box */
	if(this->header.x_min > this->header.x_max 
			|| this->header.y_min > this->header.y_max)
	{
		/* header not valid, so just replace values with current
		 * cell */
		this->header.x_min = this->header.x_max = cell.center_x;
		this->header.y_min = this->header.y_max = cell.center_y;
	}
	else
	{
		/* update bounds based on this cell */
		if(this->header.x_min > cell.center_x)
			this->header.x_min = cell.center_x;
		if(this->header.x_max < cell.center_x)
			this->header.x_max = cell.center_x;

		if(this->header.y_min > cell.center_y)
			this->header.y_min = cell.center_y;
		if(this->header.y_max < cell.center_y)
			this->header.y_max = cell.center_y;
	}

	/* success */
	return 0;
}
			
void writer_t::close()
{
	/* check if stream is open */
	if(this->outfile.is_open())
	{
		/* rewrite header with correct number of cells */
		this->outfile.seekp(0);
		this->header.serialize(this->outfile);

		/* close the stream */
		this->outfile.close();
	}
}

