#include "fss_io.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <string>
#include <vector>
#include <util/error_codes.h>
#include <util/endian.h>
#include <util/binary_search.h>

/**
 * @file fss_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * Implements the reader and writer classes for the .fss file format.
 * This format is used to store range, depth, or time-of-flight data
 * with synchronized timestamps and statistical information.
 */

/* use the following namespaces for the implementations in this file */
using namespace std;
using namespace fss;

/* function implementations for reader_t */
			
reader_t::reader_t()
{
	/* set default configuration */
	this->frame_positions.clear();
	this->frame_timestamps.clear();
	this->auto_correct_for_bias = false;
	this->auto_convert_to_meters = true;
}
			
reader_t::~reader_t()
{
	/* free all resources */
	this->close();
}

void reader_t::close()
{
	/* check if file is open, and if so,
	 * then close it */
	if(this->infile.is_open())
		this->infile.close();

	/* clear all lists pertaining to this file */
	this->frame_positions.clear();
	this->frame_timestamps.clear();
}

int reader_t::open(const std::string& filename)
{
	frame_t frame;
	unsigned int i;
	int ret;

	/* check if we already have a file open */
	if(this->infile.is_open())
		return -1; /* can't open two files at once */

	/* attempt to open the binary file for reading */
	this->infile.open(filename.c_str(), 
	                  ios_base::in | ios_base::binary);
	if(!(this->infile.is_open()))
		return -2; /* failed to open file */

	/* read the header information from the file */
	ret = this->header.parse(this->infile);
	if(ret)
	{
		/* can't parse the header */
		cerr << "[fss::reader_t::open]\tUnable to open "
		     << filename << endl;
		this->close();
		return PROPEGATE_ERROR(-3, ret);
	}

	/* do a once-through for all the frames, recording their
	 * stream positions in the file */
	this->frame_positions.resize(this->header.num_scans);
	this->frame_timestamps.resize(this->header.num_scans);
	for(i = 0; i < this->header.num_scans; i++)
	{
		/* save position of this frame */
		this->frame_positions[i] = this->infile.tellg();

		/* read the frame */
		ret = frame.parse(this->infile, this->header);
		if(ret)
		{
			cerr << "[fss::reader_t::open]\tUnable to read "
			     << "frame " << i << " of "
			     << this->header.num_scans << endl;
			this->close();
			return PROPEGATE_ERROR(-4, ret);
		}

		/* save the timestamp */
		this->frame_timestamps[i] = frame.timestamp;
	}

	/* success */
	return 0;
}

size_t reader_t::num_frames() const
{
	/* return the number of scans denoted in the header */
	return (this->header.num_scans);
}

string reader_t::scanner_name() const
{
	return (this->header.scanner_name);
}
			
SPATIAL_UNITS reader_t::units() const
{
	if(this->auto_convert_to_meters)
		return UNITS_METERS; /* the points will always be meters */
	return (this->header.units); /* return the units of the file */
}

int reader_t::get(frame_t& frame, unsigned int i)
{
	unsigned int j;
	double c;
	int ret;

	/* check that input index is valid */
	if(i >= this->header.num_scans)
		return -1;

	/* move infile to the appropriate stream position */
	this->infile.seekg(this->frame_positions[i]);
	
	/* get the current frame */
	ret = frame.parse(this->infile, this->header);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* perform auto-conversions, if selected */
	if(this->auto_convert_to_meters)
	{
		/* get conversion factor */
		c = convert_units_from_meters(this->header.units);
		c = 1.0/c; /* want to convert TO meters, not FROM meters */

		/* apply to all points */
		for(j = 0; j < this->header.num_points_per_scan; j++)
			frame.points[j].scale(c);
	}

	/* there is also an option to auto-subtract bias */
	if(this->auto_correct_for_bias)
		for(j = 0; j < this->header.num_points_per_scan; j++)
			frame.points[j].correct_for_bias();

	/* success */
	return 0;
}
			
int reader_t::get_nearest(frame_t& frame, double ts)
{
	unsigned int i;
	int ret;

	/* get the closest frame to this timestamp */
	i = binary_search::get_closest_index(this->frame_timestamps, ts);

	/* populate frame */
	ret = this->get(frame, i);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* success */
	return 0;
}

/* function implementations for writer_t */
			
writer_t::writer_t()
{
	/* initialize empty writer */
	this->points_written_so_far = 0;
}
			
writer_t::~writer_t()
{
	/* free all resources */
	this->close();
}

void writer_t::init(const string& name, const string& type,
                    size_t num_s, size_t num_p, SPATIAL_UNITS u)
{
	/* initialize values to default */
	this->header.init(name, type, num_s, num_p, u);
}
			
int writer_t::open(const string& filename)
{
	/* check if file is already open */
	if(this->outfile.is_open())
		return -1; /* invalid command */

	/* open binary file for writing */
	this->outfile.open(filename.c_str(),
	                   ios_base::out | ios_base::binary);
	if(!(this->outfile.is_open()))
		return -2; /* failed to open file */

	/* write the header to file */
	this->header.print(this->outfile);

	/* check file status */
	if(this->outfile.bad())
	{
		/* close the file and return error */
		this->outfile.close();
		return -3;
	}
	
	/* reset count of points written */
	this->points_written_so_far = 0;

	/* success */
	return 0;
}
			
int writer_t::write(const frame_t& frame)
{
	int ret;

	/* write the frame to file */
	ret = frame.print(this->outfile, this->header);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}
			
void writer_t::close()
{
	/* check if file is open */
	if(this->outfile.is_open())
		this->outfile.close();
}

/* function implmenetations for frame_t */

frame_t::frame_t()
{
	/* initialize empty frame */
	this->timestamp = 0;
	this->points.clear();
}

int frame_t::parse(istream& infile, const header_t& header)
{
	size_t i;
	int ret;

	/* how we parse the input depends on the format for this file */
	switch(header.format)
	{
		case FORMAT_ASCII:
			/* the next value in ascii should be a timestamp */
			infile >> this->timestamp;
			break;
		case FORMAT_LITTLE_ENDIAN:
			/* the next value in binary is the timestamp */
			infile.read((char*) &(this->timestamp),
			            sizeof(this->timestamp));
			break;
		case FORMAT_BIG_ENDIAN:
			/* the next value in binary is the timestamp */
			infile.read((char*) &(this->timestamp),
			            sizeof(this->timestamp));
			
			/* the byte ordering in the file is the reverse
			 * of what is on the machine, so flip dem bits */
			this->timestamp = be2led(this->timestamp);
			break;
		default:
			/* unknown format */
			return -1;
	}

	/* check that the input stream is still good */
	if(infile.bad())
		return -2;

	/* parse the points from the file */
	this->points.resize(header.num_points_per_scan);
	for(i = 0; i < header.num_points_per_scan; i++)
	{
		/* use the point structure to parse the individual
		 * point information from the stream */
		ret = this->points[i].parse(infile, header);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);
	}

	/* success */
	return 0;
}

int frame_t::print(ostream& outfile, const header_t& header) const
{
	size_t i;
	int ret;
	double t;

	/* how we print the output depends on the format for this file */
	switch(header.format)
	{
		case FORMAT_ASCII:
			/* the next value in ascii is the timestamp */
			outfile << this->timestamp << endl;
			break;
		case FORMAT_LITTLE_ENDIAN:
			/* the next value in binary is the timestamp */
			outfile.write((char*) &(this->timestamp),
			              sizeof(this->timestamp));
			break;
		case FORMAT_BIG_ENDIAN:
			/* the byte ordering in the file is the reverse
			 * of what is on the machine, so flip dem bits */
			t = le2bed(this->timestamp);
			
			/* the next value in binary is the timestamp */
			outfile.write((char*) &t, sizeof(t));
			break;
		default:
			/* unknown format */
			return -1;
	}

	/* check that the output stream is still good */
	if(outfile.bad())
		return -2;

	/* print the points from the file */
	for(i = 0; i < header.num_points_per_scan; i++)
	{
		/* use the point structure to parse the individual
		 * point information from the stream */
		ret = this->points[i].print(outfile, header);
		if(ret)
			return PROPEGATE_ERROR(-3, ret);
	}

	/* success */
	return 0;
}

/* function implementations for point_t */

point_t::point_t()
{
	/* set to default values */
	this->x         = 0.0;
	this->y         = 0.0;
	this->z         = 0.0;
	this->intensity = 0;
	this->bias      = 0.0;
	this->stddev    = 0.0;
	this->width     = 0.0;
}
			
int point_t::parse(istream& infile, const header_t& header)
{
	/* how we parse a point is dependent on the file format
	 * specified in the header */
	switch(header.format)
	{
		/* check for valid formats */
		case FORMAT_ASCII:
			/* space-separated ascii */
			infile >> this->x;
			infile >> this->y;
			infile >> this->z;
			infile >> this->intensity;
			infile >> this->bias;
			infile >> this->stddev;
			infile >> this->width;
			break;

		case FORMAT_LITTLE_ENDIAN:
			/* binary format, ordered with little-endian,
			 * which is what we assume this machine is also
			 * ordered in, so no bit-flipping is needed */
			infile.read((char*) &(this->x), sizeof(this->x));
			infile.read((char*) &(this->y), sizeof(this->y));
			infile.read((char*) &(this->z), sizeof(this->z));
			infile.read((char*) &(this->intensity),
			            sizeof(this->intensity));
			infile.read((char*) &(this->bias),
			            sizeof(this->bias));
			infile.read((char*) &(this->stddev),
			            sizeof(this->stddev));
			infile.read((char*) &(this->width),
			            sizeof(this->width));
			break;

		case FORMAT_BIG_ENDIAN:
			/* binary format, in big endian (network order),
			 * which will require us to flip some bits */
			infile.read((char*) &(this->x), sizeof(this->x));
			this->x = be2led(this->x);
			infile.read((char*) &(this->y), sizeof(this->y));
			this->y = be2led(this->y);
			infile.read((char*) &(this->z), sizeof(this->z));
			this->z = be2led(this->z);
			infile.read((char*) &(this->intensity),
			            sizeof(this->intensity));
			this->intensity = be2leq(this->intensity);
			infile.read((char*) &(this->bias),
			            sizeof(this->bias));
			this->bias = be2led(this->bias);
			infile.read((char*) &(this->stddev),
			            sizeof(this->stddev));
			this->stddev = be2led(this->stddev);
			infile.read((char*) &(this->width),
			            sizeof(this->width));
			this->width = be2led(this->width);
			break;

		/* unknown format */
		default:
			return -1; /* error, invalid header object */
	}

	/* check that stream is still good */
	if(infile.bad())
		return -2; /* error occurred at some point */

	/* success */
	return 0;
}
			
int point_t::print(ostream& outfile, const header_t& header) const
{
	double d;
	int i;

	/* how we print a point is dependent on the file format
	 * specified in the header */
	switch(header.format)
	{
		/* check for valid formats */
		case FORMAT_ASCII:
			/* space-separated ascii */
			outfile << this->x << " "
			        << this->y << " "
			        << this->z << " "
			        << this->intensity << " "
			        << this->bias << " "
			        << this->stddev << " "
			        << this->width << endl;
			break;

		case FORMAT_LITTLE_ENDIAN:
			/* binary format, ordered with little-endian,
			 * which is what we assume this machine is also
			 * ordered in, so no bit-flipping is needed */
			outfile.write((char*) &(this->x), sizeof(this->x));
			outfile.write((char*) &(this->y), sizeof(this->y));
			outfile.write((char*) &(this->z), sizeof(this->z));
			outfile.write((char*) &(this->intensity),
			              sizeof(this->intensity));
			outfile.write((char*) &(this->bias),
			              sizeof(this->bias));
			outfile.write((char*) &(this->stddev),
			              sizeof(this->stddev));
			outfile.write((char*) &(this->width),
			              sizeof(this->width));
			break;

		case FORMAT_BIG_ENDIAN:
			/* binary format, in big endian (network order),
			 * which will require us to flip some bits */
			d = le2bed(this->x);
			outfile.write((char*) &d, sizeof(d));
			d = le2bed(this->y);
			outfile.write((char*) &d, sizeof(d));
			d = le2bed(this->z);
			outfile.write((char*) &d, sizeof(d));
			i = le2beq(this->intensity);
			outfile.write((char*) &i, sizeof(i));
			d = le2bed(this->bias);
			outfile.write((char*) &d, sizeof(d));
			d = le2bed(this->stddev);
			outfile.write((char*) &d, sizeof(d));
			d = le2bed(this->width);
			outfile.write((char*) &d, sizeof(d));
			break;

		/* unknown format */
		default:
			return -1; /* error, invalid header object */
	}

	/* check that stream is still good */
	if(outfile.bad())
		return -2; /* error occurred at some point */

	/* success */
	return 0;
}

void point_t::correct_for_bias()
{
	double mag, xhat, yhat, zhat;

	/* get distance of point from origin */
	mag = sqrt( (this->x * this->x)
		+ (this->y * this->y) 
		+ (this->z * this->z) );

	/* get look vector for this point from the origin */
	xhat = this->x / mag;
	yhat = this->y / mag;
	zhat = this->z / mag;


	/* subtract bias from the look vector */
	this->x += this->bias * xhat;
	this->y += this->bias * yhat;
	this->z += this->bias * zhat;
}

void point_t::scale(double s)
{
	/* scale the distance values */
	this->x      *= s;
	this->y      *= s;
	this->z      *= s;
	this->bias   *= s;
	this->stddev *= s;
	this->width  *= s;
}

/* function implementations for header_t */

header_t::header_t()
{
	/* set all parameters to blank (invalid) */
	this->version = 0.0;
	this->format = FORMAT_UNKNOWN;
	this->scanner_name = "";
	this->scanner_type = "";
	this->units = UNITS_UNKNOWN;
	this->num_scans = 0;
	this->num_points_per_scan = 0;
}
			
void header_t::init(const string& name, const string& type,
                    size_t num_s, size_t num_p, SPATIAL_UNITS u)
{
	/* set all values in the header to either default values or
	 * to the provided values */
	this->version = LATEST_SUPPORTED_VERSION; /* current version */
	this->format = FORMAT_LITTLE_ENDIAN; /* default output format */
	this->scanner_name = name;
	this->scanner_type = type;
	this->units = u;
	this->num_scans = num_s;
	this->num_points_per_scan = num_p;
}
			
int header_t::parse(istream& infile)
{
	stringstream parser;
	string tline, tag, val;

	/* check validity of input */
	if(infile.fail())
		return -1;

	/* look for magic number */
	getline(infile, tline);
	if(tline.compare(MAGIC_NUMBER))
	{
		cerr << "[fss::header_t::parse]\tAttempting to parse "
		     << "stream that is not valid .fss format" << endl;
		return -2; /* not a valid fss file, magic numbers differ */
	}

	/* attempt to parse a .fss header from the open file stream */
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
		else if(!tag.compare(HEADER_TAG_VERSION))
			parser >> (this->version); /* store the version */
		else if(!tag.compare(HEADER_TAG_FORMAT))
		{
			/* get the format string */
			parser >> val;

			/* convert to enum */
			this->format = string_to_format(val);
		
			/* check if valid format */
			if(this->format == FORMAT_UNKNOWN)
			{
				/* report error */
				cerr << "[fss::header_t::parse]\tCould not "
				     << "parse file format: " << val
				     << endl;
				return -3;
			}
		}
		else if(!tag.compare(HEADER_TAG_SCANNER_NAME))
		{
			/* store the scanner name */
			parser >> (this->scanner_name);
		}
		else if(!tag.compare(HEADER_TAG_SCANNER_TYPE))
		{
			/* store the scanner type */
			parser >> (this->scanner_type);
		}
		else if(!tag.compare(HEADER_TAG_NUM_SCANS))
		{
			/* store the number of scan frames in this file */
			parser >> (this->num_scans);
		}
		else if(!tag.compare(HEADER_TAG_NUM_POINTS_PER_SCAN))
		{
			/* store the number of points per scan frame */
			parser >> (this->num_points_per_scan);
		}
		else if(!tag.compare(HEADER_TAG_UNITS))
		{
			/* get the units string */
			parser >> val;

			/* convert to enum */
			this->units = string_to_units(val);
		
			/* check if valid */
			if(this->units == UNITS_UNKNOWN)
			{
				/* report error */
				cerr << "[fss::header_t::parse]\t"
				     << "Unrecognized units: " << val
				     << endl;
				return -4;
			}
		}
		else
		{
			/* unknown tag */
			cerr << "[fss::header_t::parse]\tUnknown header "
			     << "tag found: " << tline << endl;
			return -5;
		}
	}

	/* check validity of parsed data */
	if(this->version < EARLIEST_SUPPORTED_VERSION)
	{
		/* version is deprecated */
		cerr << "[fss::header_t::parse]\tVersion "
		     << (this->version) << " of FSS format no longer "
		     << "supported by this file reader." << endl;
		return -6;
	}
	if(this->version > LATEST_SUPPORTED_VERSION)
	{
		/* version is from a future iteration of file format */
		cerr << "[fss::header_t::parse]\tWARNING, File version "
		     << (this->version) << " is beyond latest known version"
		     << ": " << LATEST_SUPPORTED_VERSION << endl;
	}
	if(this->format == FORMAT_UNKNOWN)
	{
		/* file format value not specified */
		cerr << "[fss::header_t::parse]\tNo file format provided!"
		     << endl;
		return -7;
	}
	if(this->units == UNITS_UNKNOWN)
	{
		/* units value not specified */
		cerr << "[fss::header_t::parse]\tNo units specified!"
		     << endl;
		return -8;
	}
	if(this->num_scans == 0)
	{
		/* num scans not specified */
		cerr << "[fss::header_t::parse]\tNumber of scan frames not "
		     << "specified!" << endl;
		return -9;
	}
	if(this->num_points_per_scan == 0)
	{
		/* number of points per scan not specified */
		cerr << "[fss::header_t::parse]\tNumber of points per scan "
		     << "not specified!" << endl;
		return -10;
	}
	if(this->scanner_name.empty())
	{
		/* must specify name of scanner */
		cerr << "[fss::header_t::parse]\tFile does not contain "
		     << "scanner name!" << endl;
		return -11;
	}
	
	/* success */
	return 0;
}
			
void header_t::print(ostream& outfile) const
{
	/* print current header information to stream */
	outfile << MAGIC_NUMBER << endl
	   << HEADER_TAG_VERSION << " " << this->version << endl
	   << HEADER_TAG_FORMAT
	   << " " << format_to_string(this->format) << endl
	   << HEADER_TAG_SCANNER_NAME << " " << this->scanner_name << endl;

	/* the scanner type is an optional tag */
	if(!(this->scanner_type.empty()))
		outfile << HEADER_TAG_SCANNER_TYPE << " " 
		        << this->scanner_type << endl;

	/* the remainder of the tags are required */
	outfile << HEADER_TAG_NUM_SCANS << " " << this->num_scans << endl
	   << HEADER_TAG_NUM_POINTS_PER_SCAN 
	   << " " << this->num_points_per_scan << endl
	   << HEADER_TAG_UNITS 
	   << " " << units_to_string(this->units) << endl
	   << END_HEADER_STRING << endl;
}
