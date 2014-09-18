#include "mesh_io.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

/**
 * @file   mesh_io_obj.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This class can read in various mesh file formats
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to import and
 * export vertex and triangle information from common mesh
 * file formats, specifically for Wavefront OBJ file formats.
 */

using namespace std;
using namespace mesh_io;

/* the following definitions are used for the OBJ format */
#define COMMENT_CHARACTER     "#"
#define WHITESPACE            " \t\n\r"
#define VERTEX_FLAG           "v"
#define FACE_FLAG             "f"
#define TEXTURE_SEP_CHARACTER "/"

/*---------------------------------*/
/* mesh_t function implementations */ 
/*---------------------------------*/
	
int mesh_t::read_obj(const string& filename)
{
	ifstream infile;
	stringstream ss;
	string tline, val;
	polygon_t poly;
	vertex_t vert;
	size_t p, q;
	int ind;

	/* open file for reading */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		/* report error */
		cerr << "[mesh_t::read_obj]\tUnable to open file for "
		     << "reading: " << filename << endl;
		return -1;
	}

	/* iterate through the lines of the file */
	while(!(infile.eof()))
	{
		/* get the next line */
		getline(infile, tline);

		/* remove any comments from this line */
		p = tline.find_first_of(COMMENT_CHARACTER);
		if(p != string::npos)
			tline = tline.substr(0, p);

		/* trim any whitespace in the front */
		p = tline.find_first_not_of(WHITESPACE);
		if(p == string::npos)
			continue; /* blank line */
		if(p != 0)
			tline = tline.substr(p);

		/* check for unsupported characters */
		while(true)
		{
			/* check for any texture/normal separators */
			p = tline.find_first_of(TEXTURE_SEP_CHARACTER);
			if(p == string::npos)
				break;
			
			/* remove any values between a slash and
			 * the next whitespace, since these denote
			 * either texture or normal indices, which
			 * we don't care about */
			q = tline.find_first_of(WHITESPACE, p);	
			if(q == string::npos)
				/* remove rest of string */
				tline = tline.substr(0,p);
			else
				/* remove segment involving texture
				 * or normal indices */
				tline = tline.substr(0,p)
						+ tline.substr(q);
		}

		/* determine the type of line based on the first value */
		ss.clear();
		ss.str(tline);
		ss >> val;
		if(val.compare(VERTEX_FLAG) == 0)
		{
			/* this is a vertex */
			ss >> vert.x >> vert.y >> vert.z;

			/* check if there's color */
			if(!(ss.eof()))
			{
				/* read in color */
				ss >> vert.red >> vert.green >> vert.blue;
				this->format = FORMAT_OBJ_COLOR;
			}

			/* store vertex */
			this->add(vert);
		}
		else if(val.compare(FACE_FLAG) == 0)
		{
			/* this is a face */
			poly.clear();

			/* read in as many indices as possible */
			while(!(ss.eof()))
			{
				/* get next index */
				ss >> ind;
		
				/* check value */
				if(ind > 0)
				{
					/* obj indexes from 1, not 0 */
					poly.vertices.push_back(ind-1);
				}
				else if(ind < 0)
				{
					/* relative indexing */
					poly.vertices.push_back(
						this->vertices.size()+ind);
				}
				else
				{
					/* can't handle zero */
					cerr << "[mesh_t::read_obj]Error! "
					     << "face #"
					     << this->polygons.size()
					     << " has vertex index 0"
					     << endl;
					return -3;
				}
			}

			/* store it */
			this->add(poly);
		}
		else
		{
			/* ignore everything else */
			continue;
		}
	}

	/* clean up */
	infile.close();
	return 0;
}
			
int mesh_t::write_obj(const std::string& filename, bool color) const
{
	ofstream outfile;
	FILE_FORMAT ff;
	size_t i, n;
	int ret;

	/* attempt to open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[mesh_t::write_obj]\tUnable to open file "
		     << "for writing: " << filename << endl;
		return -1;
	}

	/* prepare file format */
	ff = color ? FORMAT_OBJ_COLOR : FORMAT_OBJ;

	/* OBJ files don't have a header, but write some comments
	 * at the top just to describe the file */
	outfile << "# Mesh generated using [mesh_io] c++ package" << endl
		<< "#" << endl
		<< "# [mesh_io] Written by Eric Turner" << endl
		<< "#           <elturner@eecs.berkeley.edu>" << endl
		<< "#" << endl
		<< "# Num vertices: " << this->vertices.size() << endl
		<< "# Num polygons: " << this->polygons.size() << endl
		<< "#" << endl;

	/* iterate over vertices in mesh */
	n = this->vertices.size();
	for(i = 0; i < n; i++)
	{
		/* write this vertex */
		ret = this->vertices[i].serialize(outfile, ff);
		if(ret)
		{
			/* report error */
			cerr << "[mesh_t::write_obj]\tError " << ret << ": "
			     << "Unable to write vertex #" << i << endl;
			return -2;
		}
	}

	/* iterate over triangles (or general polygons) in mesh */
	n = this->polygons.size();
	for(i = 0; i < n; i++)
	{
		/* write this polygon */
		ret = this->polygons[i].serialize(outfile, ff);
		if(ret)
		{
			/* report error */
			cerr << "[mesh_t::write_obj]\tError " << ret << ": "
			     << "Unable to write poly #" << i << endl;
			return -3;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
