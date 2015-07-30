#include "color_image_metadata_reader.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <string.h>
#include <util/error_codes.h>

/**
 * @file color_image_metadata_reader.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains definitions for classes used to read
 * and parse the output image metadata files generated after
 * demosaicing and time synchronization has occurred.
 *
 * These files include the file names of the jpeg images,
 * as well as the meta-information for each image,
 * such as timestamp and camera settings.
 */

using namespace std;

/* function implementations for color_image_frame_t */
color_image_frame_t::color_image_frame_t()
{
	/* set default values */
	this->image_file = "";
	this->index = -1;
	this->timestamp = -1.0;
	this->exposure = -1;
	this->gain = -1;
}

color_image_frame_t::~color_image_frame_t()
{
	/* no resources explicitly allocated */
}

int color_image_frame_t::parse(std::istream& is)
{
	stringstream ss;
	string line;

	/* check that stream is still valid */
	if(is.fail())
		return -1;

	/* parse the next line from file */
	getline(is, line);
	if(line.empty())
		return -2;

	/* parse values from line */
	ss.str(line);
	ss >> this->index;
	ss >> this->image_file;
	ss >> this->timestamp;
	ss >> this->exposure;
	ss >> this->gain;

	/* check that values are valid */
	if(this->image_file.empty() || this->index < 0)
		return -3;

	/* check that stream is good */
	if(is.fail())
		return -4;
	
	/* success */
	return 0;
}

/* function implementations for color_image_reader_t */

color_image_reader_t::color_image_reader_t()
{
	/* default values already set */
}

color_image_reader_t::~color_image_reader_t()
{
	/* close the file if necessary */
	this->close();
}

/* helper function:
 *
 * Removes all occurances of carriage returns
 * from the string.
 */
void color_image_reader_remove_all_cr(string& m)
{
	size_t pos;
	
	/* remove all \r that are found */
	while(true)
	{
		/* check if there are any carriage returns
		 * on this line */
		pos = m.find('\r');
		if(pos == string::npos)
			break;

		/* remove them if present */
		m.erase(pos);
	}
}

int color_image_reader_t::open(const std::string& filename)
{
	string m;

	/* close any open files */
	this->close();

	/* open the file */
	this->infile.open(filename.c_str());
	if(!(this->infile.is_open()))
		return -1;

	/* read the header */
	
	/* read camera hardware data */
	this->infile >> this->camera_name;
	this->infile >> this->num_images;
	this->infile >> this->jpeg_quality;

	/* get location of image directory */
	do
	{
		getline(this->infile, this->output_dir);
		color_image_reader_remove_all_cr(this->output_dir);
	}
	while(this->output_dir.empty()); /* ignore blank lines */

	/* the header should end with an extra newline */
	getline(this->infile, m);
	color_image_reader_remove_all_cr(m);
	if(m.size() > 0)
	{
		this->close();
		cerr << "[color_image_reader_t::open]\tExpected blank"
		     << " line after header, got: \"" << m << "\"" << endl;
		return -2;
	}

	/* success */
	return 0;
}

int color_image_reader_t::next(color_image_frame_t& frame)
{
	int ret;

	/* parse the next frame from the file */
	ret = frame.parse(this->infile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

bool color_image_reader_t::eof() const
{
	/* check if at end of file */
	if(this->infile.is_open())
		return this->infile.eof();
	return true;
}

void color_image_reader_t::close()
{
	/* close the file if it is open */
	if(this->infile.is_open())
		this->infile.close();

	/* no other resources need freeing */
}

/* static functions */

int color_image_reader_t::copy_file(const std::string& oldfile,
                                    const std::string& newfile,
                                    const std::string& camname,
                                    int quality,
                                    const std::string& outputdir)
{
	color_image_reader_t infile;
	ofstream outfile;
	color_image_frame_t frame;
	int i, ret;

	/* open existing file for reading */
	ret = infile.open(oldfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* attempt to open output file */
	outfile.open(newfile.c_str());
	if(!(outfile.is_open()))
	{
		infile.close();
		return -2;
	}

	/* configure output file precision */
	outfile.setf(ios::fixed, ios::floatfield);
	outfile.precision(16);

	/* write header information */
	outfile << (camname.empty() ? infile.get_camera_name() : camname)
	        << endl /* writes camera name to file */
		<< infile.get_num_images() << endl /* number of images */
		<< (quality < 0 ? infile.get_jpeg_quality() : quality)
		<< endl /* writes jpeg quality value [0,100] */
		<< (outputdir.empty() ? infile.get_output_dir() : outputdir)
		<< endl /* writes the output directory for images */
		<< endl; /* extra line between header and content */

	/* iterate over images in file body */
	for(i = 0; i < infile.get_num_images(); i++)
	{
		/* get current frame information */
		ret = infile.next(frame);
		if(ret)
		{
			/* could not read frame */
			infile.close();
			outfile.close();
			return PROPEGATE_ERROR(-3, ret);
		}

		/* write metadata for this image */
		outfile << frame.index << " "
		        << frame.image_file << " "
			<< frame.timestamp << " "
			<< frame.exposure << " "
			<< frame.gain << endl;
	}

	/* clean up */
	infile.close();
	outfile.close();
	return 0;
}

