#include "scanolist_io.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/**
 * @file    scanolist_io.cpp
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   Reads and writes scanorama metadata files
 *
 * @section DESCRIPTION
 *
 * When exporting scanoramas as .ptx files, metadata for each
 * scanorama pose is also recorded in a scanorama metadata list
 * file (.scanolist).
 *
 * Created July 8, 2015
 */

using namespace std;
using namespace scanolist_io;

/* the following constants are used for these files */

#define SCANOLIST_MAGIC_NUMBER  "scanolist"

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int scanolist_t::write(const std::string& filename) const
{
	ofstream outfile;
	size_t i, n;

	/* attempt to open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
	{
		/* can't open file */
		cerr << "[scanolist_t::write]\tUnable to open file "
		     << "for writing: \"" << filename << "\"" << endl;
		return -1;
	}

	/* write out header */
	outfile << SCANOLIST_MAGIC_NUMBER << endl
		<< this->num_rows << " " << this->num_cols << endl
		<< this->scano_poses.size() << endl;

	/* write out list of cameras */
	n = this->camera_names.size();
	for(i = 0; i < n; i++)
		outfile << this->camera_names[i] << " ";
	outfile << endl;

	/* add extra new line at end of header */
	outfile << endl;

	/* write out all the poses' metadata */
	n = this->scano_poses.size();
	for(i = 0; i < n; i++)
		this->scano_poses[i].print(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
			
int scanolist_t::read(const std::string& filename)
{
	ifstream infile;
	string magic, tline, camname;
	stringstream ss;
	size_t i, n;
	int ret;

	/* clear any existing data */
	this->clear();

	/* attempt to open file for reading */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		/* can't open file */
		cerr << "[scanolist_t::read]\tUnable to find and/or "
		     << "open file: \"" << filename << "\"" << endl;
		return -1;
	}

	/* check for magic number */
	getline(infile, magic);
	if(magic.compare(SCANOLIST_MAGIC_NUMBER) != 0)
	{
		/* bad file type */
		cerr << "[scanolist_t::read]\tUnrecognized file format."
		     << "  Expected \"" << SCANOLIST_MAGIC_NUMBER << "\" "
		     << "but got \"" << magic << "\"" << endl;
		infile.close();
		return -2;
	}

	/* read header info */
	infile >> this->num_rows;
	infile >> this->num_cols;
	infile >> n; /* number of poses */
	this->scano_poses.resize(n);
	getline(infile, tline); /* the list of cameras */
	ss.str(tline);
	while(!(ss.eof()))
	{
		ss >> camname;
		if(camname.empty())
			break; /* no more names */
		this->camera_names.push_back(camname);
	}

	/* iterate through poses */
	for(i = 0; i < n; i++)
	{
		/* read next pose */
		ret = this->scano_poses[i].parse(infile);
		if(ret)
		{
			cerr << "[scanolist_t::read]\tUnable to read "
			     << "pose #" << i << " (error " << ret
			     << ") of file: \"" << filename << "\""
			     << endl;
			return -2;
		}
	}

	/* clean up */
	infile.close();
	return 0;
}
			
void scanometa_t::print(std::ostream& os) const
{
	/* print the stuff */
	os << this->index << " " 
	   << this->timestamp << " " 
	   << this->filepath << endl;
}
			
int scanometa_t::parse(std::istream& is)
{
	stringstream ss;
	string tline("");

	/* check that file is good */
	if(is.fail())
		return -1;

	/* get the next non-empty line */
	while(tline.empty() && is.good())
		getline(is, tline);
	ss.str(tline);

	/* read in the index as an unsigned int */
	if(!(ss >> this->index))
		return -2; /* no value */

	/* read timestamp as a double */
	if(!(ss >> this->timestamp))
		return -3; /* no value */

	/* read file path */
	if(!(ss >> this->filepath))
		return -4; /* no value */

	/* success */
	return 0;
}
			
void scanometa_t::truncate_filepath()
{
	size_t seppos;

	/* get the position of the last separator character */
	seppos = this->filepath.find_last_of("\\/");
	if(seppos == string::npos)
		return;

	/* truncate the path */
	this->filepath = this->filepath.substr(seppos+1);
}
