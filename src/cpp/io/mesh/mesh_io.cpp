#include "mesh_io.h"
#include <iostream>
#include <sstream>
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
}

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
			ret = this->write_obj(filename, false);
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
		case FORMAT_OBJ_COLOR:
			ret = this->write_obj(filename, true);
			if(ret)
			{
				/* report error */
				cerr << "[mesh_t::write]\t"
				     << "Error " << ret 
				     << ": Unable to write colored "
				     << "OBJ file: " << filename << endl;
				return -3;
			}
			break;

		/* ply */
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			ret = this->write_ply(filename, f);
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
			
void mesh_t::set_color(bool color)
{
	if(color)
	{
		switch(this->format)
		{
			default:
				/* do nothing */
				return;

			/* go from non-colored to color */
			case FORMAT_UNKNOWN:
			case FORMAT_OBJ:
				this->format = FORMAT_OBJ_COLOR;
				break;
			case FORMAT_PLY_ASCII:
				this->format = FORMAT_PLY_ASCII_COLOR;
				break;
			case FORMAT_PLY_BE:
				this->format = FORMAT_PLY_BE_COLOR;
				break;
			case FORMAT_PLY_LE:
				this->format = FORMAT_PLY_LE_COLOR;
				break;
		}
	}
	else
	{
		switch(this->format)
		{
			default:
				/* do nothing */
				return;

			/* go from colored to non-colored */
			case FORMAT_OBJ_COLOR:
				this->format = FORMAT_OBJ;
				break;
			case FORMAT_PLY_ASCII_COLOR:
				this->format = FORMAT_PLY_ASCII;
				break;
			case FORMAT_PLY_BE_COLOR:
				this->format = FORMAT_PLY_BE;
				break;
			case FORMAT_PLY_LE_COLOR:
				this->format = FORMAT_PLY_LE;
				break;
		}
	}
}
			
void mesh_t::clear()
{
	/* clear values */
	this->vertices.clear();
	this->polygons.clear();
	this->format = FORMAT_UNKNOWN;
}
			
void mesh_t::add(const mesh_t& other)
{
	size_t index_offset, i, j, num_verts, num_polys;
	polygon_t poly;

	/* the following is the index offset from the other mesh to
	 * its elements being copied into this mesh */
	index_offset = this->vertices.size();
	num_verts = other.num_verts();
	num_polys = other.num_polys();

	/* iterate over the vertices of the other mesh, adding them */
	for(i = 0; i < num_verts; i++)
		this->add(other.get_vert(i));

	/* now add the polygons from the other mesh, making sure
	 * to update the referenced vertex indices */
	for(i = 0; i < num_polys; i++)
	{
		/* get polygon from other mesh */
		poly.set(other.get_poly(i).vertices);
		
		/* update vertex indices */
		num_verts = poly.vertices.size();
		for(j = 0; j < num_verts; j++)
			poly.vertices[j] += index_offset;

		/* store polygon in this mesh */
		this->add(poly);
	}
}
			
FILE_FORMAT mesh_t::get_format(const string& filename) const
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
	{
		/* check if we read in an OBJ, in which
		 * case we should write out with the same
		 * formatting options */
		switch(this->format)
		{
			/* check if already an obj */
			case FORMAT_OBJ:
			case FORMAT_OBJ_COLOR:
				return this->format;

			/* check if colored of another format */
			case FORMAT_PLY_ASCII_COLOR:
			case FORMAT_PLY_BE_COLOR:
			case FORMAT_PLY_LE_COLOR:
				return FORMAT_OBJ_COLOR;

			/* return uncolored obj */
			default:
				return FORMAT_OBJ;
		}
	}
	if(suffix.compare(".ply") == 0)
	{
		/* check if we read in a PLY file, in 
		 * which case we should write out in the
		 * same manner */
		switch(this->format)
		{
			/* check if already a ply format */
			case FORMAT_PLY_ASCII:
			case FORMAT_PLY_BE:
			case FORMAT_PLY_LE:
			case FORMAT_PLY_ASCII_COLOR:
			case FORMAT_PLY_BE_COLOR:
			case FORMAT_PLY_LE_COLOR:
				return this->format;

			/* check if colored of another format */
			if(FORMAT_OBJ_COLOR)
				return FORMAT_PLY_LE_COLOR;

			/* return uncolored ply */
			default:
				return FORMAT_PLY_LE;
		}
	}

	/* unknown format */
	return FORMAT_UNKNOWN;
}

/*----------------------------------*/
/* vertex_t function implemenations */
/*----------------------------------*/
			
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
			/* don't support big endian..yet */
			cerr << "[vertex_t::serialize]\tError, this format "
			     << "is not yet supported for serialization: "
			     << ff << endl;
			return -2;
		case FORMAT_PLY_LE:
			os.write((char*) &(this->x), sizeof(this->x));
			os.write((char*) &(this->y), sizeof(this->y));
			os.write((char*) &(this->z), sizeof(this->z));
			break;
		case FORMAT_PLY_LE_COLOR:
			os.write((char*) &(this->x), sizeof(this->x));
			os.write((char*) &(this->y), sizeof(this->y));
			os.write((char*) &(this->z), sizeof(this->z));
			os.write((char*) &(this->red), sizeof(this->red));
			os.write((char*) &(this->green),
						sizeof(this->green));
			os.write((char*) &(this->blue), sizeof(this->blue));
			break;
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
			
int polygon_t::serialize(std::ostream& os, FILE_FORMAT ff) const
{
	size_t i, n;
	int v;
	unsigned char num;

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
			/* don't support big endian...yet */
			cerr << "[polygon_t::serialize]\tError, format "
			     << "is not yet supported for serialization: "
			     << ff << endl;
			return -2;
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			/* export size of face as an unsigned char,
			 * and the face vertex indices as ints */
			num = (int) n;
			os.write((char*) &num, sizeof(num));
			for(i = 0; i < n; i++)
			{
				v = this->vertices[i];
				os.write((char*) &v, sizeof(v));
			}
			break;
	}

	/* success */
	return 0;
}

