#include "mesh_io.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * @file   mesh_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This class can read in various mesh file formats
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to import and
 * export vertex and triangle information from common mesh
 * file formats.
 */

using namespace std;
using namespace mesh_io;

/*---------------------------------*/
/* mesh_t function implementations */ 
/*---------------------------------*/

mesh_t::mesh_t(const string& filename)
{
	/* parse the file */
	this->read(filename);
};

int mesh_t::read(const string& filename)
{
	int ret;

	/* infer the file format from the name */
	this->format = mesh_t::get_format(filename);

	/* call the file-specific functions */
	switch(this->format)
	{
		/* unknown */
		default:
		case FORMAT_UNKNOWN:
			cerr << "[mesh_t::read]\tUnknown file format: "
			     << filename << endl;
			return -1;

		/* obj */
		case FORMAT_OBJ:
		case FORMAT_OBJ_COLOR:
			ret = this->read_obj(filename);
			if(ret)
			{
				/* report error */
				cerr << "[mesh_t::read]\t"
				     << "Error " << ret 
				     << ": Unable to parse "
				     << "OBJ file: " << filename << endl;
				return -2;
			}
			break;

		/* ply */
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			ret = this->read_ply(filename);
			if(ret)
			{
				/* report error */
				cerr << "[mesh_t::read]\tError " << ret 
				     << "Unable to parse PLY file: "
				     << filename << endl;
				return -3;
			}
			break;
	}

	/* success */
	return 0;
}
			
int mesh_t::write(const string& filename) const
{
	FILE_FORMAT f;
	
	/* infer the file format from the name */
	f = mesh_t::get_format(filename);

	/* write it based on format */
	return this->write(filename, f);
}
			
int mesh_t::write(const string& filename, FILE_FORMAT f) const
{
	int ret;

	/* call format-specific write function */
	switch(f)
	{
		/* unknown */
		default:
		case FORMAT_UNKNOWN:
			cerr << "[mesh_t::write]\tUnknown file format: "
			     << filename << endl;
			return -1;

		/* obj */
		case FORMAT_OBJ:
		case FORMAT_OBJ_COLOR:
			ret = this->write_obj(filename);
			if(ret)
			{
				/* report error */
				cerr << "[mesh_t::write]\t"
				     << "Error " << ret 
				     << ": Unable to write "
				     << "OBJ file: " << filename << endl;
				return -2;
			}
			break;

		/* ply */
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			ret = this->write_ply(filename);
			if(ret)
			{
				/* report error */
				cerr << "[mesh_t::write]\tError " << ret 
				     << "Unable to write PLY file: "
				     << filename << endl;
				return -3;
			}
			break;
	}

	/* success */
	return 0;
}
			
void mesh_t::clear()
{
	/* clear values */
	this->vertices.clear();
	this->polygons.clear();
	this->format = FORMAT_UNKNOWN;
}
			
FILE_FORMAT mesh_t::get_format(const string& filename)
{
	size_t p;
	string suffix;

	/* get position of last dot */
	p = filename.find_last_of('.');
	if(p == string::npos)
		return FORMAT_UNKNOWN;

	/* determine format from suffix */
	suffix = filename.substr(p);
	if(suffix.compare(".obj") == 0)
		return FORMAT_OBJ;
	if(suffix.compare(".ply") == 0)
		return FORMAT_PLY_ASCII;

	/* unknown format */
	return FORMAT_UNKNOWN;
}
			
int mesh_t::read_obj(const string& filename)
{
	ifstream infile;

	// TODO
}
			
int mesh_t::write_obj(const std::string& filename) const
{
	ofstream outfile;

	/* attempt to open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[mesh_t::write_obj]\tUnable to open file "
		     << "for writing: " << filename << endl;
		return -1;
	}

	// TODO
	
	/* clean up */
	outfile.close();
	return 0;
}
			
int mesh_t::read_ply(const std::string& filename)
{
	// TODO
}
			
int mesh_t::write_ply(const std::string& filename) const
{
	// TODO
}

/*----------------------------------*/
/* vertex_t function implemenations */
/*----------------------------------*/
			
int vertex_t::parse(istream& is, FILE_FORMAT ff)
{
	// TODO
}
			
int vertex_t::serialize(ostream& os, FILE_FORMAT ff) const
{
	/* how we serialize depends on the file format */
	switch(ff)
	{
		default:
		case FORMAT_UNKNOWN:
			cerr << "[vertex_t::serialize]\tCan't serialize "
			     << "to unknown file format." << endl;
			return -1; /* unable to serialize to unknown */
		
		/* obj */
		case FORMAT_OBJ:
			os << "v " << this->x 
			   <<  " " << this->y 
			   <<  " " << this->z
			   << endl;
			break;
		case FORMAT_OBJ_COLOR:
			os << "v " << this->x 
			   <<  " " << this->y 
			   <<  " " << this->z
			   <<  " " << this->red
			   <<  " " << this->green
			   <<  " " << this->blue
			   << endl;
			break;

		/* ply */
		case FORMAT_PLY_ASCII:
			os << this->x << " " 
			   << this->y << " "
			   << this->z << endl;
			break;
		case FORMAT_PLY_ASCII_COLOR:
			os << this->x     << " "
			   << this->y     << " "
			   << this->z     << " "
			   << this->red   << " "
			   << this->green << " "
			   << this->blue  << endl;
			break;
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			cerr << "[vertex_t::serialize]\tError, this format "
			     << "is not yet supported for serialization: "
			     << ff << endl;
			return -2;

	}

	/* success */
	return 0;
}

/*------------------------------------*/
/* polygon_t function implementations */
/*------------------------------------*/
			
polygon_t::polygon_t(size_t i, size_t j, size_t k)
{
	/* reset this polygon to a triangle */
	this->vertices.clear();
	this->vertices.push_back(i);
	this->vertices.push_back(j);
	this->vertices.push_back(k);
}
			
int polygon_t::parse(std::istream& is, FILE_FORMAT ff)
{
	// TODO
}

int polygon_t::serialize(std::ostream& os, FILE_FORMAT ff) const
{
	size_t i, n;

	/* get number of indices to export */
	n = this->vertices.size();
	
	/* how we serialize depends on the file format */
	switch(ff)
	{
		default:
		case FORMAT_UNKNOWN:
			cerr << "[polygon_t::serialize]\tCan't serialize "
			     << "to unknown file format." << endl;
			return -1; /* unable to serialize to unknown */
		
		/* obj */
		case FORMAT_OBJ:
		case FORMAT_OBJ_COLOR:
			os << "f";
			for(i = 0; i < n; i++)
				/* OBJ indexes from 1, not from 0 */
				os << " " << (this->vertices[i]+1);
			os << endl;
			break;

		/* ply */
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
			os << n; /* write out size of face */
			for(i = 0; i < n; i++)
				os << " " << this->vertices[i];
			os << endl;
			break;
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			cerr << "[polygon_t::serialize]\tError, format "
			     << "is not yet supported for serialization: "
			     << ff << endl;
			return -2;

	}

	/* success */
	return 0;
}

