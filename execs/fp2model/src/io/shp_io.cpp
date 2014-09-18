#include "shp_io.h"
#include "../structs/building_model.h"
#include <util/error_codes.h>
#include <util/endian.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>

/**
 * @file   shp_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Export model to .shp file
 *
 * @section DESCRIPTION
 *
 * This file contains functions to export floorplans to .shp files.
 * This file format is defined here:
 *
 * http://en.wikipedia.org/wiki/Shapefile#Shapefile_shape_format_.28.shp.29
 *
 * http://www.esri.com/library/whitepapers/pdfs/shapefile.pdf
 */

using namespace std;

/*-----------*/
/* constants */
/*-----------*/

/* the following constants are used for .shp files */
const int shape_file_magic_number   = 0x0000270a; /* start of file magic */
const int shape_file_version        = 1000; 
const size_t shape_file_header_size = 100;      /* units: bytes */
const size_t shape_file_record_header_size = 8; /* units: bytes */
const int shape_type_Point          = 1;
const int shape_type_PolygonM       = 5;

/*-------------------------*/
/* helper function headers */
/*-------------------------*/

void writeboundingbox(ofstream& outfile, const building_model_t& bim);
void skipheader(ofstream& outfile);
void writeheader(ofstream& outfile, ofstream& shxfile,
			const building_model_t& bim);
int writepoint(ofstream& outfile, const building_model_t& bim, int i);
void writeoffset(ofstream& outfile, int off);
int writepolygon(ofstream& outfile, const building_model_t& bim);

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int writeshp(const std::string& filename, const building_model_t& bim)
{
	ofstream outfile, shxfile;
	string shxfilename;
	int ret;

	/* open file for writing */
	outfile.open(filename.c_str(), ios_base::out | ios_base::binary);
	if(!(outfile.is_open()))
	{
		/* report error */
		cerr << "[writeshp]\tUnable to open file for writing: "
		     << filename << endl;
		return -1;
	}

	/* open the shx to write to concurrently with shp file */
	shxfilename = filename.substr(0, filename.size()-1) + "x";
	shxfile.open(shxfilename.c_str(), ios_base::out | ios_base::binary);
	if(!(shxfile.is_open()))
	{
		/* report error */
		cerr << "[writeshp]\tUnable to open shx file: "
		     << shxfilename << endl;
		return -2;
	}

	/* skip the header for now, since we can write it at the end */
	skipheader(outfile);

	/* write the polygon information for the floorplan to the file */
	ret = writepolygon(outfile, bim);
	if(ret)
	{
		cerr << "[writeshp]\tUnable to write floorplan as polygon "
		     << "to " << filename << endl;
		return -2;
	}

	/* write header information, now that we have full info */
	writeheader(outfile, shxfile, bim);

	/* clean up */
	outfile.close();
	shxfile.close();
	return 0;
}

/*------------------------*/
/* helper function bodies */
/*------------------------*/

/**
 * Writes bounding box of floorplan to the file
 *
 * Will write min_x, min_y, max_x, max_y to the file in binary,
 * little-endian format.
 *
 * @param outfile   The stream to write to
 * @param bim       The data to write
 */
void writeboundingbox(ofstream& outfile, const building_model_t& bim)
{
	double min_x, min_y, max_x, max_y;

	/* compute bounds for model */
	bim.floorplan.compute_bounds(min_x, min_y, max_x, max_y);
	
	/* write to stream */
	outfile.write((char*) &min_x, sizeof(min_x));
	outfile.write((char*) &min_y, sizeof(min_y));
	outfile.write((char*) &max_x, sizeof(max_x));
	outfile.write((char*) &max_y, sizeof(max_y));
}

/**
 * Skips the header of the .shp and .shx files
 */
void skipheader(ofstream& outfile)
{
	outfile.seekp(shape_file_header_size
			+ shape_file_record_header_size, ios_base::beg);
}

/**
 * Writes the header to the specified stream
 *
 * This function will move the stream to the start of the file
 * and overwrite the header information.  The BIM struct is necessary
 * in order to compute the bounding box and other features that go
 * into the header.
 *
 * @param outfile   The output stream to write to
 * @param shxfile   The shx file to export indices to
 * @param bim       The model to export
 *
 * @return          Returns zero on success, non-zero on failure.
 */
void writeheader(ofstream& outfile, ofstream& shxfile,
			const building_model_t& bim)
{
	int filelen, i;
	streampos filestart, fileend;
	double d;

	/* get the length of the file */
	outfile.seekp(0, ios_base::end);
	fileend = outfile.tellp();
	outfile.seekp(0, ios_base::beg);
	filestart = outfile.tellp();
	filelen = (fileend - filestart);

	/* write out header information at the start of the file */
	
	/* magic number */
	i = le2beq(shape_file_magic_number); 
	outfile.write((char*) &i, sizeof(i)); /* written in big-endian */
	shxfile.write((char*) &i, sizeof(i));

	/* skip unused bytes */
	outfile.seekp(24, ios_base::beg);
	shxfile.seekp(24, ios_base::beg);

	/* write out file length */
	writeoffset(outfile, filelen);
	writeoffset(outfile, shape_file_header_size + 8); 
			/* shx file will only have one record,
			 * so total size is 100 (header) + 8 (record) */

	/* write out version */
	outfile.write((char*) &shape_file_version,
			sizeof(shape_file_version));
	shxfile.write((char*) &shape_file_version,
			sizeof(shape_file_version));

	/* export shape type */
	outfile.write((char*) &shape_type_PolygonM,
			sizeof(shape_type_PolygonM));
	shxfile.write((char*) &shape_type_PolygonM,
			sizeof(shape_type_PolygonM));
	
	/* export the bounding box */
	writeboundingbox(outfile, bim);
	writeboundingbox(shxfile, bim);

	/* export range of z's and m's (which should all be zero) */
	d = 0.0;
	outfile.write((char*) &d, sizeof(d)); /* min z */
	outfile.write((char*) &d, sizeof(d)); /* max z */
	outfile.write((char*) &d, sizeof(d)); /* min m */
	outfile.write((char*) &d, sizeof(d)); /* max m */
	shxfile.write((char*) &d, sizeof(d)); /* min z */
	shxfile.write((char*) &d, sizeof(d)); /* max z */
	shxfile.write((char*) &d, sizeof(d)); /* min m */
	shxfile.write((char*) &d, sizeof(d)); /* max m */

	/* write record header for the one shape in this file */
	i = le2beq(1);
	outfile.write((char*) &i, sizeof(i)); /* record indexed from 1 */

	/* write size of record */
	i = filelen - shape_file_header_size 
		- shape_file_record_header_size; /* size of record */
	writeoffset(outfile, i);

	/* write record to shape file */
	int j = shape_file_header_size;
	writeoffset(shxfile, j); 
				/* record starts at end of header */
	writeoffset(shxfile, i); /* size of record in main file */
}

/**
 * Writes a point shape to the .shp file stream
 *
 * Writes the (x,y) position of the point in little-endian ordering
 * to the file.  Does NOT write the point field tag (i.e. 1).
 *
 * @param outfile   The output stream to write to
 * @param bim       The originating floorplan
 * @param i         The index of the point to write
 *
 * @return          Returns zero on success, non-zero on failure.
 */
int writepoint(ofstream& outfile, const building_model_t& bim, int i)
{
	/* check arguments */
	if(i < 0 || i > (int) bim.floorplan.verts.size())
		return -1;

	/* write point value */
	outfile.write((char*) &(bim.floorplan.verts[i].x), sizeof(double));
	outfile.write((char*) &(bim.floorplan.verts[i].y), sizeof(double));

	/* success */
	return 0;
}

/**
 * Writes the offset value to a shx or shp file
 *
 * @param outfile   The file to write to
 * @param off       The offset to write
 */
void writeoffset(ofstream& outfile, int off)
{
	int i;

	/* prepare offset as big-endian count of 16-bit words */
	i = le2beq(off / 2);

	/* write it */
	outfile.write((char*) &i, sizeof(i));
}

/**
 * Writes a polygon shape to the .shp file stream
 *
 * Will export a PolygonM record to the shape file, which represents
 * the entirety of the floorplan.
 *
 * The metadata fields are not provided.
 *
 * @param outfile   The output stream to write to
 * @param bim       The floorplan to export
 *
 * @return          Returns zero on success, non-zero on failure.
 */
int writepolygon(ofstream& outfile, const building_model_t& bim)
{
	vector<vector<int> > boundary_list;
	vector<int> parts;
	int numParts;
	int numPoints;
	int i;
	size_t j, n;
	int ret;

	/* export shape type */
	outfile.write((char*) &shape_type_PolygonM,
			sizeof(shape_type_PolygonM));

	/* export bounding box */
	writeboundingbox(outfile, bim);

	/* get the edges of the floorplan */
	bim.floorplan.compute_oriented_boundary(boundary_list);

	/* export number of disjoint rings */
	numParts = (int) boundary_list.size();
	outfile.write((char*) &numParts, sizeof(numParts));

	/* find parts */
	numPoints = 0;
	parts.resize(numParts);
	for(i = 0; i < numParts; i++)
	{
		/* record the start of this part */
		parts[i] = numPoints;
		
		/* the last point in the part must be equal to
		 * the first point in the part, so copy it */
		boundary_list[i].push_back(boundary_list[i][0]);
				
		/* shp file expects clockwise */
		std::reverse(boundary_list[i].begin(),
				boundary_list[i].end()); 

		/* count number of points in this ring */
		numPoints += (int) boundary_list[i].size();
	}

	/* export number of points */
	outfile.write((char*) &numPoints, sizeof(numPoints));

	/* export the set of parts, which indicate which points
	 * start a new ring */
	for(i = 0; i < numParts; i++)
		outfile.write((char*) &(parts[i]), sizeof(parts[i]));

	/* export the points */
	for(i = 0; i < numParts; i++)
	{
		/* iterate over points in this part */
		n = boundary_list[i].size();
		for(j = 0; j < n; j++)
		{
			/* write out this point's location */
			ret = writepoint(outfile, bim, boundary_list[i][j]);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
		}
	}

	/* success */
	return 0;
}
