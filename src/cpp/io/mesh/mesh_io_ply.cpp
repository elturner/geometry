#include "mesh_io.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <float.h>

/**
 * @file   mesh_io_ply.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This class can read in various mesh file formats
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to import and
 * export vertex and triangle information from common mesh
 * file formats, specifically for Stanford Polygon (PLY) format.
 */

using namespace std;
using namespace mesh_io;

/* the following definitions relate to the PLY format */
#define MAGIC_NUMBER         "ply"
#define FORMAT_FLAG          "format"
#define SUPPORTED_VERSION    "1.0"
#define FORMAT_ASCII_FLAG    "ascii"
#define FORMAT_LE_FLAG       "binary_little_endian"
#define FORMAT_BE_FLAG       "binary_big_endian"
#define ELEMENT_FLAG         "element"
#define PROPERTY_FLAG        "property"
#define COMMENT_FLAG         "comment"
#define END_HEADER_FLAG      "end_header"

/* the following data types are used in PLY files */
#define DOUBLE_TYPE          "double"
#define FLOAT_TYPE           "float"
#define INT_TYPE             "int"
#define UCHAR_TYPE           "uchar"
#define LIST_TYPE            "list"
#define LIST_UCHAR_INT_TYPE  (LIST_TYPE " uchar int")
#define LIST_INT_INT_TYPE    (LIST_TYPE " int int")

/* the following names of elements and properties are supported */
const string vertex_names[] = {"vertex", "vert", "Vertex", "VERTEX", 
                               "Vert"};
const string face_names[]   = {"face", "Face", "FACE", "polygon", "Polygon",
                             "poly", "Poly", "POLYGON", "POLY"};

const string x_names[]              = {"x", "X"};
const string y_names[]              = {"y", "Y"};
const string z_names[]              = {"z", "Z"};
const string red_names[]            = {"red", "r", "R", "Red", "RED"};
const string green_names[]          = {"green", "g", "G", "Green", "GREEN"};
const string blue_names[]           = {"blue", "b", "B", "Blue", "BLUE"};
const string vertex_indices_names[] = {"vertex_indices", "vertex_index", 
                                     "vert_inds", "vert_indices",
                                     "vert_index"};

const size_t num_vertex_names       = (sizeof(vertex_names)
					/ sizeof(vertex_names[0]));
const size_t num_face_names         = (sizeof(face_names)
					/ sizeof(face_names[0]));

const size_t num_x_names            = (sizeof(x_names)/sizeof(x_names[0]));
const size_t num_y_names            = (sizeof(y_names)/sizeof(y_names[0]));
const size_t num_z_names            = (sizeof(z_names)/sizeof(z_names[0]));
const size_t num_red_names          = (sizeof(red_names)
					/ sizeof(red_names[0]));
const size_t num_green_names        = (sizeof(green_names)
					/ sizeof(green_names[0]));
const size_t num_blue_names         = (sizeof(blue_names)
					/ sizeof(blue_names[0]));
const size_t num_vertex_indices_names  = (sizeof(vertex_indices_names)
					/ sizeof(vertex_indices_names[0]));

/*------------------*/
/* helper functions */
/*------------------*/

/**
 * check if query string is in array
 */
bool string_in_arr(const string& query, const string* arr, size_t num)
{
	size_t i;
	for(i = 0; i < num; i++)
		if(query.compare(arr[i]) == 0)
			return true;
	return false;
}

/*----------------*/
/* helper classes */
/*----------------*/

/**
 * This class is used to represent a property type from a PLY file
 *
 * Each property is defined based on a name and a type
 */
class ply_property_t
{
	public:
		/* the name of this property */
		string name;

		/* the type of this property */
		string type;
};

/**
 * This class is used to represent an element type from a PLY file
 *
 * An element is defined based on a name, and a list of properties.
 * Each property has some type associated with it
 */
class ply_element_t
{
	public:
		/* name of this element */
		string name;

		/* How many of this element are in the file */
		size_t num_elements;

		/* the properties of this element */
		vector<ply_property_t> props;
};

/**
 * This class represents the indices of which properties of which
 * elements we care about
 */
class ply_indices_t
{
	public:

	int vertex_ind,face_ind,x_ind,y_ind,z_ind,r_ind,g_ind,b_ind,vi_ind;

	/**
	 * Searches through the parsed element types in a 
	 * ply file, looking for the indices of useful properties
	 */
	void init(const vector<ply_element_t>& elements)
	{
		int i, n, k, m;

		/* check if vertices/faces are defined, and if so, check how
		 * to parse them */
		n = elements.size();
		vertex_ind = face_ind 
			= x_ind = y_ind = z_ind 
			= r_ind = g_ind = b_ind 
			= vi_ind = -1; /* not yet found */
		for(i = 0; i < n; i++)
		{
			/* check the type */
			if(string_in_arr(elements[i].name, 
					vertex_names, num_vertex_names))
			{
				/* this element type is a vertex */
				vertex_ind = i;

				/* check its properties */
				m = elements[i].props.size();
				for(k = 0; k < m; k++)
				{
					/* check what type of property 
					 * this is */
					if(string_in_arr(
						elements[i].props[k].name,
						x_names, num_x_names))
						x_ind = k;
					else if(string_in_arr(
						elements[i].props[k].name,
						y_names, num_y_names))
						y_ind = k;
					else if(string_in_arr(
						elements[i].props[k].name,
						z_names, num_z_names))
						z_ind = k;
					else if(string_in_arr(
						elements[i].props[k].name,
						red_names, num_red_names))
						r_ind = k;
					else if(string_in_arr(
						elements[i].props[k].name,
						green_names, 
						num_green_names))
						g_ind = k;
					else if(string_in_arr(
						elements[i].props[k].name,
						blue_names, 
						num_blue_names))
						b_ind = k;
				}
			}
			else if(string_in_arr(elements[i].name,
					face_names, num_face_names))
			{
				/* this element type is a face */
				face_ind = i;
	
				/* check its properties */
				m = elements[i].props.size();
				for(k = 0; k < m; k++)
				{
					if(string_in_arr(
						elements[i].props[k].name,
						vertex_indices_names, 
						num_vertex_indices_names))
						vi_ind = k;
				}
			}
		}
	};

	/**
	 * Parses next available vertex information from the specified
	 * file stream, based on the index values initialized in this
	 * structure.
	 *
	 * @param is   The input file stream to parse
	 * @param ff   The format of the stream
	 * @param v    The vertex to populate
	 */
	void parse(istream& is, FILE_FORMAT ff, vertex_t& v) const
	{
		// TODO
	}

	/**
	 * Parses next available face information from the specified
	 * file stream, based on the index values initialized in
	 * this structure.
	 *
	 * @param is   The input file stream to parse
	 * @param ff   The format of the stream
	 * @param p    The polygon to populate
	 */
	void parse(istream& is, FILE_FORMAT ff, polygon_t p) const
	{
		// TODO
	}

	/**
	 * Reads the next value from the stream, given the expected
	 * type of this value.
	 *
	 * The value will be cast to a double and returned.
	 *
	 * @param is   The input file stream
	 * @param ff   The format of the stream
	 * @param t    The type of the value to parse
	 *
	 * @return     The value casted to a double
	 */
	double parse_val(istream& is, FILE_FORMAT ff, string& t) const
	{
		double d;

		/* how we parse depends on the format of the stream */
		switch(ff)
		{
			default:
				cerr << "[read_ply::parse_val]\tNot a "
				     << "valid ply file format: "
				     << ff << endl;
				return DBL_MAX;
			case FORMAT_PLY_ASCII:
			case FORMAT_PLY_ASCII_COLOR:
				/* just read ascii as a double and 
				 * return it */
				is >> d;
				return d;
			case FORMAT_PLY_BE:
			case FORMAT_PLY_BE_COLOR:
			case FORMAT_PLY_LE:
			case FORMAT_PLY_LE_COLOR:
		
				cerr << "[read_ply::parse_val]\t"
				     << "Binary format is unsupported"
				     << endl;
				return DBL_MAX;
		}

		/* if got here, not valid */
		return DBL_MAX;
	};

	/**
	 * Parses the list from the file
	 *
	 * @param is     The file stream to parse
	 * @param ff     The file format
	 * @param list   Where to store the list of indices
	 */
	void parse_list(istream& is, FILE_FORMAT ff, vector<size_t>& list)
	{
		size_t i, num, val;

		/* remove any existing items */
		list.clear();

		/* we only support ascii */
		if(ff != FORMAT_PLY_ASCII && ff != FORMAT_PLY_ASCII_COLOR)
		{
			cerr << "[read_ply::parse_list]\tCurrently only "
			     << "ascii ply files are supported." << endl;
			return;
		}

		/* get number of items in list */
		is >> num;

		/* get list */
		for(i = 0; i < num; i++)
		{
			is >> val;
			list.push_back(val);
		}
	};
};

/*---------------------------------*/
/* mesh_t function implementations */ 
/*---------------------------------*/
		
int mesh_t::read_ply(const std::string& filename)
{
	ifstream infile;
	vector<ply_element_t> elements;
	stringstream ss;
	string tline, field;
	vertex_t vert;
	polygon_t poly;
	bool readingheader;
	size_t i, j, k, n, m;
	ply_indices_t inds;

	/* open file for reading */
	infile.open(filename.c_str(), ios_base::in | ios_base::binary);
	if(!(infile.is_open()))
	{
		cerr << "[mesh_t::ready_ply]\tUnable to open file for "
		     << "reading: " << filename << endl;
		return -1;
	}

	/* parse the header */
	readingheader = true;
	while(readingheader)
	{
		/* read the next line */
		getline(infile, tline);
		ss.clear();
		ss.str(tline);

		/* parse the line */
		ss >> field;
		if(field.empty())
			continue; /* ignore empty lines */
		else if(field.compare(MAGIC_NUMBER) == 0)
		{
			/* good! it's a ply file.
			 *
			 * Note that the way this is
			 * set up, we can still correctly
			 * parse a file without this line. */
			continue;
		}
		else if(field.compare(FORMAT_FLAG) == 0)
		{
			/* get the format of this file */
			ss >> field;

			/* check against known formats */
			if(field.compare(FORMAT_ASCII_FLAG) == 0)
				this->format = FORMAT_PLY_ASCII;
			else if(field.compare(FORMAT_BE_FLAG) == 0)
				this->format = FORMAT_PLY_BE;
			else if(field.compare(FORMAT_LE_FLAG) == 0)
				this->format = FORMAT_PLY_LE;
			else
			{
				cerr << "[mesh_t::read_ply]\tUnknown PLY "
				     << "format: " << field << endl;
				return -2;
			}

			/* the version number comes next, but we don't
			 * care abou that */
		}
		else if(field.compare(END_HEADER_FLAG) == 0)
		{
			/* the header is done, exit loop */
			readingheader = false;
		}
		else if(field.compare(COMMENT_FLAG) == 0)
			continue; /* ignore comments */
		else if(field.compare(ELEMENT_FLAG) == 0)
		{
			/* add a new element to our list */
			elements.resize(elements.size()+1);

			/* store its name */
			ss >> elements.back().name;

			/* store how many there are */
			ss >> elements.back().num_elements;
		}
		else if(field.compare(PROPERTY_FLAG) == 0)
		{
			/* check that element exists */
			if(elements.empty())
			{
				cerr << "[mesh_t::read_ply]\tProperty flag "
				     << "appeared before element flag in "
				     << "header of " << filename << endl;
				return -3;
			}
			
			/* add a new property to the latest element */
			elements.back().props.resize(
					elements.back().props.size()+1);

			/* set property type */
			ss >> elements.back().props.back().type;
			if(elements.back().props.back().type.compare(
						LIST_TYPE) == 0)
			{
				/* a list has two more types, one to
				 * identify the number of elements,
				 * and another to identify each element
				 * type */
				ss >> field;
				elements.back().props.back().type +=
					(" " + field);
				ss >> field;
				elements.back().props.back().type +=
					(" " + field);
			}
			
			/* set property name */
			ss >> elements.back().props.back().name;
		}
		else
		{
			/* unknown flag */
			cerr << "[mesh_t::read_ply]\tUnable to parse "
			     << "line in header: \"" << tline << "\"" 
			     << endl;
			return -4;
		}
	}

	/* find the indices of useful parameters and elements */
	inds.init(elements);

	/* parse the data by iterating through the elements of the header */
	n = elements.size();
	for(i = 0; i < n; i++)
	{
		/* get info about this element */
		m = elements[i].props.size();

		/* iterate through instances of this element */
		for(j = 0; j < elements[i].num_elements; j++)
		{
			/* iterate through properties of this element */
			for(k = 0; k < m; k++)
			{
				/* check what type of element we're 
				 * reading */
				if(i == (size_t) inds.vertex_ind)
				{
					/* parse this vertex */
					inds.parse(infile,
						this->format, vert);
					this->add(vert);
				}
				else if(i == (size_t) inds.face_ind)
				{
					/* parse this face */
					inds.parse(infile,
						this->format, poly);
					this->add(poly);
				}
				else
				{
					/* we just want to skip over
					 * this element without storing
					 * its info. */
					// TODO
				}
			}
		}
	}

	/* clean up */
	infile.close();
	return 0;
}
			
int mesh_t::write_ply(const std::string& filename, FILE_FORMAT ff) const
{
	ofstream outfile;
	size_t i, n;
	int ret;

	/* attempt to open file for writing.  The nature of the output
	 * file depends on the format */
	switch(ff)
	{
		default:
			cerr << "[mesh_t::write_ply]\tNot valid PLY file "
			     << "format: " << ff << endl;
			return -1;
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
			outfile.open(filename.c_str());
			break;
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			outfile.open(filename.c_str(), 
					ios_base::out | ios_base::binary);
			break;
	}
	
	/* check file */
	if(!(outfile.is_open()))
	{
		cerr << "[mesh_t::write_ply]\tUnable to open file for "
		     << "writing: " << filename << endl;
		return -2;
	}

	/* magic number */
	outfile << MAGIC_NUMBER << endl;

	/* specify format */
	switch(ff)
	{
		default:
			cerr << "[mesh_t::write_ply]\tNot valid PLY file "
			     << "format: " << ff << endl;
			return -3;
		case FORMAT_PLY_ASCII:
		case FORMAT_PLY_ASCII_COLOR:
			outfile << FORMAT_FLAG       << " "
				<< FORMAT_ASCII_FLAG << " "
				<< SUPPORTED_VERSION << endl;
			break;
		case FORMAT_PLY_BE:
		case FORMAT_PLY_BE_COLOR:
			outfile << FORMAT_FLAG       << " "
				<< FORMAT_BE_FLAG    << " "
				<< SUPPORTED_VERSION << endl;
			break;
		case FORMAT_PLY_LE:
		case FORMAT_PLY_LE_COLOR:
			outfile << FORMAT_FLAG       << " "
				<< FORMAT_LE_FLAG    << " "
				<< SUPPORTED_VERSION << endl;
			break;
	}

	/* specify vertex format */
	outfile << ELEMENT_FLAG    << " "
		<< vertex_names[0] << " " << this->vertices.size() << endl
	        << PROPERTY_FLAG   << " "
		<< DOUBLE_TYPE     << " "
		<< x_names[0]      << endl
	        << PROPERTY_FLAG   << " "
		<< DOUBLE_TYPE     << " "
		<< y_names[0]      << endl
	        << PROPERTY_FLAG   << " "
		<< DOUBLE_TYPE     << " " 
		<< z_names[0]      << endl;

	/* add color info if desired */
	switch(ff)
	{
		case FORMAT_PLY_ASCII_COLOR:
		case FORMAT_PLY_BE_COLOR:
		case FORMAT_PLY_LE_COLOR:
			outfile << PROPERTY_FLAG   << " "
				<< INT_TYPE        << " " 
				<< red_names[0]    << endl
			        << PROPERTY_FLAG   << " "
				<< INT_TYPE        << " " 
				<< green_names[0]  << endl
			        << PROPERTY_FLAG   << " "
				<< INT_TYPE        << " " 
				<< blue_names[0]   << endl;
			break;
		default:
			/* no color info */
			break;
	}

	/* specify faces */
	outfile << ELEMENT_FLAG            << " "
	        << face_names[0]           << " " 
		<< this->polygons.size()   << endl
	        << PROPERTY_FLAG           << " "
		<< LIST_UCHAR_INT_TYPE     << " "
		<< vertex_indices_names[0] << endl;

	/* end header information */
	outfile << END_HEADER_FLAG << endl;

	/* write out vertices */
	n = this->vertices.size();
	for(i = 0; i < n; i++)
	{
		/* export this vertex */
		ret = this->vertices[i].serialize(outfile, ff);
		if(ret)
		{
			/* report error */
			cerr << "[mesh_t::write_ply]\tError " << ret << ": "
			     << "Unable to write vertex #" << i << endl;
			return -4;
		}
	}

	/* write out faces */
	n = this->polygons.size();
	for(i = 0; i < n; i++)
	{
		/* export this polygon */
		ret = this->polygons[i].serialize(outfile, ff);
		if(ret)
		{
			/* report error */
			cerr << "[mesh_t::write_ply]\tError " << ret << ": "
			     << "Unable to write polygon #" << i << endl;
			return -5;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
