#include "scanorama.h"
#include "scanorama_point.h"
#include <image/fisheye/fisheye_camera.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>
#include <libe57/E57Foundation.h>
#include <libe57/E57Simple.h>
#include <lodepng/lodepng.h>
#include <Eigen/Dense>
#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>

/**
 * @file     scanorama.cpp
 * @author   Eric Turner <elturner@indoorreality.com>
 * @brief    The scanorama_t class represents a single scanorama pointcloud
 *
 * @section DESCRIPTION
 *
 * A scanorama pointcloud is used to represent a panoramic image with
 * depth information.  Each "pixel" of the panorama is stored as a
 * (x,y,z) point in 3D space, along with color.
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void scanorama_t::init_sphere()
{
	Vector3d zero(0,0,0);

	/* call the overloaded function with default arguments */
	this->init_sphere(0, zero, 500, 1000, 0);
}

void scanorama_t::init_sphere(double t, const Eigen::Vector3d& cen,
				size_t r, size_t c, double bw)
{
	double radius, dt, dp, theta, phi, w, x, y, z, unitwidth;
	size_t ri, ci, i;

	/* allocate the appropriate number of points */
	this->points.resize(r*c);
	this->timestamp  = t;
	this->center     = cen;
	this->num_rows   = r;
	this->num_cols   = c;
	this->blendwidth = bw;

	/* iterate over the points and define the geometry of a sphere */
	radius = 10;
	dt = (2 * M_PI) / this->num_cols;
	dp = (M_PI) / this->num_rows;
	unitwidth = sin( (dt+dp) / 4.0 );
	for(ci = 0; ci < this->num_cols; ci++) /* column-major order */
		for(ri = 0; ri < this->num_rows; ri++)
		{
			/* we want to set the current point to reside
			 * on the unit sphere centered at this->center */
			theta = dt * ci;
			phi   = dp * ri;
			w     = radius * sin(phi);
			z     = radius * cos(phi);
			y     =  w * sin(theta);
			x     = -w * cos(theta);

			/* store in appropriate point */
			i = ri*this->num_cols + ci;
			this->points[i].x  = x;
			this->points[i].y  = y;
			this->points[i].z  = z;
			this->points[i].nx = x / radius;
			this->points[i].ny = y / radius;
			this->points[i].nz = z / radius;

			/* estimate the uncertainty width of
			 * this point, based on half the average
			 * of the angular difference between points */
			this->points[i].width = radius * unitwidth;

			/* make some color pattern */
			this->points[i].color.set_2d_pattern(ri, ci);
			this->points[i].quality = -DBL_MAX;
		}
}
		
int scanorama_t::init_geometry(const OctTree<float>& octree,
				double t, const Eigen::Vector3d& cen,
				size_t r, size_t c, double bw)
{
	progress_bar_t progbar;
	double radius, theta, phi, dt, dp, w, x, y, z, nx, ny, nz, unitwidth;
	float origin[3]; /* origin of raytracing */
	float d[3]; /* raytracing direction */
	float inter[3]; /* intersection point */
	size_t triangleID;
	size_t ri, ci, i;
	bool suc;

	/* first, clear any existing information */
	this->clear();
	progbar.set_name("   Pose geometry");

	/* next, allocate the appropriate number of points */
	this->points.resize(r*c);
	this->timestamp  = t;
	this->center     = cen;
	this->num_rows   = r;
	this->num_cols   = c;
	this->blendwidth = bw;

	/* copy scan center to be origin of raytracing */
	origin[0] = this->center[0];
	origin[1] = this->center[1];
	origin[2] = this->center[2];

	/* iterate over the points and define the geometry of a sphere */
	radius = 1; /* direction is unit-vector */
	dt = (2 * M_PI) / this->num_cols; /* delta-theta */
	dp = (M_PI) / this->num_rows; /* delta-phi */
	unitwidth = sin( (dt+dp) / 4.0 );
	for(ci = 0; ci < this->num_cols; ci++) /* column-major order */
	{
		/* update progress bar */
		progbar.update(ci, this->num_cols);

		/* iterate over rows */
		for(ri = 0; ri < this->num_rows; ri++)
		{
			/* we want to set the current point to reside
			 * on the unit sphere centered at this->center */
			theta = dt * ci;
			phi   = dp * ri;
			w     = radius * sin(phi);
			d[2]  = radius * cos(phi);
			d[1]  =      w * sin(theta);
			d[0]  =     -w * cos(theta);

			/* perform a raytrace from the scan center along
			 * the direction vector d to determine
			 * the location of the scan point */
			suc = octree.ray_trace(origin,d,inter,&triangleID);
			if(!suc)
			{
				/* unable to raytrace, set depth
				 * to be zero */
				x = origin[0];
				y = origin[1];
				z = origin[2];
				nx = ny = nz = 0;
			}
			else
			{
				/* successfully raytraced, so determine
				 * position of intersect point in
				 * the scan's coordinate system */
				x = inter[0] - origin[0];
				y = inter[1] - origin[1];
				z = inter[2] - origin[2];
				
				/* get the triangle object that was hit */
				const Triangle3<float>& tri = octree.triangle(
								triangleID);
				nx = tri.normal(0);
				ny = tri.normal(1);
				nz = tri.normal(2);
			}

			/* store in appropriate point */
			i = ci*this->num_rows + ri; /* column major */
			this->points[i].x  = x;
			this->points[i].y  = y;
			this->points[i].z  = z;
			this->points[i].nx = nx;
			this->points[i].ny = ny;
			this->points[i].nz = nz;

			/* estimate the uncertainty width of
			 * this point, based on half the average
			 * of the angular difference between points */
			this->points[i].width = 
				sqrt(x*x + y*y + z*z) * unitwidth;

			/* No color has been assigned yet, so set
			 * it to black */
			this->points[i].color.set(0.0f,0.0f,0.0f);
			this->points[i].quality = -DBL_MAX;
		}
	}

	/* success */
	return 0;
}
		
int scanorama_t::apply(camera_t* cam)
{
	progress_bar_t progbar;
	color_t newcolor;
	double px, py, pz, q;
	double dq, w1, w2;
	int ret, r, g, b;
	size_t i, n;

	/* iterate through all points */
	progbar.set_name("  Applying image");
	n = this->points.size();
	for(i = 0; i < n; i++)
	{
		/* update progress bar */
		progbar.update(i,n);

		/* get the world coordinates for the current point */
		px = this->center(0) + this->points[i].x;
		py = this->center(1) + this->points[i].y;
		pz = this->center(2) + this->points[i].z;

		/* get the color of this point according to the argument
		 * camera */
		ret = cam->color_point_antialias(px,py,pz, 
				this->points[i].width,
				this->timestamp,r,g,b,q);
		if(ret)
		{
			/* report error */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[scanorama_t::apply]\tError " 
			     << ret << ": Unable to color point #"
			     << i << "/" << n << endl;
			return ret;
		}

		/* check if the quality is better */
		dq = q - this->points[i].quality;
		if(q < 0)
		{
			/* if quality is negative, we don't want it */
			continue;
		}
		else if(fabs(dq) < this->blendwidth)
		{
			/* This point has already been colored, and 
			 * the new image provides roughly the same quality
			 * of coloring.  Rather than replace the color,
			 * we should perform a blending to allow for
			 * smooth transistions between texture sources.
			 *
			 * We want to average the values together,
			 * weighted on their qualtiy */
			newcolor.set_ints(r,g,b);

			/* Get the weighting of the new color
			 *
			 * Blending is linear across the blending width */
			w2 = (dq + this->blendwidth)/(2 * this->blendwidth);

			/* weight of original color */
			w1 = 1 - w2; 

			/* apply the average */
			this->points[i].color
				= ( this->points[i].color * w1 )
				+ ( newcolor * w2 );
		}
		else if(q > this->points[i].quality)
		{
			/* replace the color */
			this->points[i].quality = q;
			this->points[i].color.set_ints(r,g,b);
		}
	}

	/* success */
	return 0;
}
		
void scanorama_t::writeobj(std::ostream& os) const
{
	size_t i, n;

	/* write a header */
	os << "#" << endl
	   << "# Scanorama" << endl
	   << "# time: " << this->timestamp << endl
	   << "# dimensions: " << this->num_rows 
	   << ", " << this->num_cols << endl
	   << "#" << endl;

	/* iterate over the points */
	n = this->points.size();
	for(i = 0; i < n; i++)
	{
		os << "v " << this->points[i].x
		   <<  " " << this->points[i].y
		   <<  " " << this->points[i].z
		   <<  " " << this->points[i].color.get_red_int()
		   <<  " " << this->points[i].color.get_green_int()
		   <<  " " << this->points[i].color.get_blue_int()
		   << endl;
	}
}
	
void scanorama_t::writeptx(std::ostream& os) const
{
	progress_bar_t progbar;
	size_t i, n;
	
	/* export the header of the PTX file */
	os << this->num_cols << endl /* number of columns */
	   << this->num_rows << endl /* number of rows */
	   << this->center(0) << " "
	   << this->center(1) << " "
	   << this->center(2) << endl /* scanner position */
	   << "1 0 0"   << endl  /* scanner x-axis */
	   << "0 1 0"   << endl  /* scanner y-axis */
	   << "0 0 1"   << endl  /* scanner z-axis */
	   << "1 0 0 0" << endl  /* 1st row of 4x4 transform matrix */
	   << "0 1 0 0" << endl  /* 2nd row of 4x4 transform matrix */
	   << "0 0 1 0" << endl  /* 3rd row of 4x4 transform matrix */
	   << this->center(0) << " "
	   << this->center(1) << " "
	   << this->center(2) << " 1" << endl; 
		/* 4th row of 4x4 transform matrix */

	/* iterate over the points */
	progbar.set_name("   Exporting PTX");
	n = this->points.size();
	for(i = 0; i < n; i++)
	{
		/* update progress bar */
		progbar.update(i, n);

		/* each line is a point: x y z intensity r g b
		 * where intensity is in range [0,1] */
		os << this->points[i].x << " "
		   << this->points[i].y << " "
		   << this->points[i].z << " "
		   << this->points[i].color.get_grayscale() << " "
		   << this->points[i].color.get_red_int() << " "
		   << this->points[i].color.get_green_int() << " "
		   << this->points[i].color.get_blue_int() << endl;
	}
	progbar.clear();
}

int scanorama_t::writeptg(const std::string& filename) const
{
	const char*    ptg_filestart   = "PTG";
	const uint32_t ptg_magic       = 2458887111;
	const char*    ptg_headerstart = "%%header_begin";
	const char*    ptg_versionkey  = "%%version";
	const int32_t  ptg_versionnum  = 1;
	const char*    ptg_colskey     = "%%cols";
	const char*    ptg_rowskey     = "%%rows";
	const char*    ptg_transkey    = "%%transform";
	const char*    ptg_propskey    = "%%properties";
	const int32_t  ptg_propsval    = (0x1 | 0x8);
	const char*    ptg_headerend   = "%%header_end";
	progress_bar_t progbar;
	double d0, d1;
	float x,y,z;
	unsigned char r,g,b, validitybits;
	int32_t v;
	ofstream outfile;
	streampos bodystart, colstart;
	int64_t colstart_int;
	size_t ci, ri, i;

	/* start progress bar */
	progbar.set_name("   Exporting PTG");

	/* attempt to open the file */
	outfile.open(filename.c_str(), ios::out | ios::binary);
	if(!(outfile.is_open()))
	{
		progbar.clear();
		cerr << "[scanorama_t::writeptg]\tUnable to export "
		     << "to file: \"" << filename << "\"" << endl;
		return -1;
	}

	/* write the header information 
	 *
	 * Note:  PTG is in little-endian format, so we don't
	 * need to do any bit-flipping */
	outfile.write(ptg_filestart, 1+strlen(ptg_filestart));
	outfile.write((char*) &ptg_magic, sizeof(ptg_magic));
	
	v = 1+strlen(ptg_headerstart);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_headerstart, v);

	v = 1+strlen(ptg_versionkey);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_versionkey, v);
	outfile.write((char*) &ptg_versionnum, sizeof(ptg_versionnum));
	
	/* write # of cols and rows to header */
	v = 1+strlen(ptg_colskey);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_colskey, v);
	v = (int32_t) (this->num_cols);
	outfile.write((char*) &v, sizeof(v));
	
	v = 1+strlen(ptg_rowskey);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_rowskey, v);
	v = (int32_t) (this->num_rows);
	outfile.write((char*) &v, sizeof(v));

	/* write scanner transform to header
	 *
	 * We are writing the following 4x4 matrix in row-major order:
	 *
	 * 	1	0	0	0
	 * 	0	1	0	0
	 * 	0	0	1	0
	 * 	cx	cy	cz	1
	 *
	 */
	v = 1+strlen(ptg_transkey);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_transkey, v);
	d0 = 0.0; d1 = 1.0;
	
	outfile.write((char*) &d1, sizeof(d1));
	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d0, sizeof(d0));
	
	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d1, sizeof(d1));
	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d0, sizeof(d0));

	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d0, sizeof(d0));
	outfile.write((char*) &d1, sizeof(d1));
	outfile.write((char*) &d0, sizeof(d0));

	outfile.write((char*) &(this->center(0)), sizeof(this->center(0)));
	outfile.write((char*) &(this->center(1)), sizeof(this->center(1)));
	outfile.write((char*) &(this->center(2)), sizeof(this->center(2)));
	outfile.write((char*) &d1, sizeof(d1));

	/* write properties to header.  this states that we will
	 * export (x,y,z) as floats, then (r,g,b) each as unsigned chars */
	v = 1+strlen(ptg_propskey);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_propskey, v);
	outfile.write((char*) &ptg_propsval, sizeof(ptg_propsval));
	
	/* end the header */
	v = 1+strlen(ptg_headerend);
	outfile.write((char*) &v, sizeof(v));
	outfile.write(ptg_headerend, v);

	/* write the body of the file:
	 *
	 * The start of the body is a list of num_col int64_t's that
	 * denote the file position of each column.
	 *
	 * This values are populated as we write out each column,
	 * so for now export junk bytes */
	bodystart = outfile.tellp();
	colstart_int = 0;
	for(ci = 0; ci < this->num_cols; ci++)
		outfile.write((char*) &colstart_int, sizeof(colstart_int));

	/* now write the actual point data, in column-major order */
	for(ci = 0; ci < this->num_cols; ci++)
	{
		/* update progress bar */
		progbar.update(ci, this->num_cols);

		/* record the start of this column */
		colstart = outfile.tellp();
		outfile.seekp(bodystart 
				+ (streamoff) (ci*sizeof(colstart_int)));
		colstart_int = (int64_t) colstart;
		outfile.write((char*) &colstart_int, sizeof(colstart_int));
		outfile.seekp(colstart);

		/* write out the bitmask indicating that all points
		 * are valid */
		validitybits = 0xFF; /* all ones --> all valid */
		for(ri = 0; ri < ceil(this->num_rows/8.0); ri++)
			outfile.put(validitybits);

		/* iterate over the points in this column */
		for(ri = 0; ri < this->num_rows; ri++)
		{
			/* get index of this point */
			i = ri + (ci * this->num_rows);

			/* each point is: x y z r g b */

			/* geometry in scanner local coords */
			x = (float)  this->points[i].x;
			y = (float)  this->points[i].y;
			z = (float)  this->points[i].z;

			/* color is in range [0-255] */
			r = (unsigned char) 
				this->points[i].color.get_red_int();
			g = (unsigned char) 
				this->points[i].color.get_green_int();
			b = (unsigned char) 
				this->points[i].color.get_blue_int();

			/* export to file */
			outfile.write((char*) &x, sizeof(x));
			outfile.write((char*) &y, sizeof(y));
			outfile.write((char*) &z, sizeof(z));
			outfile.write((char*) &r, sizeof(r));
			outfile.write((char*) &g, sizeof(g));
			outfile.write((char*) &b, sizeof(b));
		}
	}

	/* clean up */
	progbar.clear();
	outfile.close();
	return 0;
}

int scanorama_t::writee57(const std::string& filename) const
{
	progress_bar_t progbar;
	string coordinate_metadata(""); /* optional string */
	e57::Writer outfile(filename, coordinate_metadata);
	e57::Data3D header;
	double*   xdata;
	double*   ydata;
	double*   zdata;
	int8_t*   xyzinvalid;
	double*   intdata; /* intensity */
	int8_t*   intinvalid;
	uint16_t* reddata;
	uint16_t* greendata;
	uint16_t* bluedata;
	int8_t*   colorinvalid;
	int32_t*  rowindex;
	int32_t*  colindex;
	double*   timedata;
	size_t bpos, epos, nSize, r, c, i;
	int scan_index;

	/* make sure the file is open */
	if(!(outfile.IsOpen()))
	{
		cerr << "[scanorama_t::writee57]\tUnable to open output "
		     << "file: \"" << filename << "\"" << endl;
		return -1;
	}

	/* get the name of this scan position by parsing the filename */
	bpos = filename.find_last_of("\\/");
	if(bpos == string::npos)
		bpos = 0;
	else
		bpos++;
	epos = filename.find_last_of(".");
	header.name = filename.substr(bpos, epos);

	/* populate the header of the output e57 file */
	header.guid = "{D3817EC3-A3DD-4a81-9EF5-2FFD0EC91D5A}";
	header.acquisitionStart.dateTimeValue   = this->timestamp;
	header.acquisitionEnd.dateTimeValue     = this->timestamp;
	header.pointsSize                       = this->points.size(); 
	
	header.pointFields.cartesianXField      = true;
	header.pointFields.cartesianYField      = true;
	header.pointFields.cartesianZField      = true;
	header.pointFields.cartesianInvalidStateField = true;
	header.pointFields.intensityField       = true;
	header.pointFields.isIntensityInvalidField = true;
	header.pointFields.colorRedField        = true;
	header.pointFields.colorGreenField      = true;
	header.pointFields.colorBlueField       = true;
	header.pointFields.isColorInvalidField  = true;
	header.pointFields.rowIndexField        = true;
	header.pointFields.columnIndexField     = true;
	header.pointFields.timeStampField       = true;
	
	header.indexBounds.rowMinimum           = 0;
	header.indexBounds.rowMaximum           = this->num_rows - 1;
	header.indexBounds.columnMinimum        = 0;
	header.indexBounds.columnMaximum        = this->num_cols - 1;
	header.indexBounds.returnMinimum        = 0;
	header.indexBounds.returnMaximum        = 0;
	header.intensityLimits.intensityMinimum = 0.0;
	header.intensityLimits.intensityMaximum = 1.0;
	header.colorLimits.colorRedMinimum      = 0;
	header.colorLimits.colorRedMaximum      = 255;
	header.colorLimits.colorGreenMinimum    = 0;
	header.colorLimits.colorGreenMaximum    = 255;
	header.colorLimits.colorBlueMinimum     = 0;
	header.colorLimits.colorBlueMaximum     = 255;
	header.pose.rotation.w                  = 1;
	header.pose.rotation.x                  = 0;
	header.pose.rotation.y                  = 0;
	header.pose.rotation.z                  = 0;
	header.pose.translation.x               = this->center(0);
	header.pose.translation.y               = this->center(1);
	header.pose.translation.z               = this->center(2);
	header.pointGroupingSchemes.groupingByLine.groupsSize 
						= this->num_cols;
	header.pointGroupingSchemes.groupingByLine.pointCountSize
						= this->num_rows;
	scan_index = outfile.NewData3D(header);
	
	/* set up scan buffers to write the points */
	nSize        = this->num_rows;
	xdata        = new double[nSize];
	ydata        = new double[nSize];
	zdata        = new double[nSize];
	xyzinvalid   = new int8_t[nSize];
	intdata      = new double[nSize];
	intinvalid   = new int8_t[nSize];
	reddata      = new uint16_t[nSize];
	greendata    = new uint16_t[nSize];
	bluedata     = new uint16_t[nSize];
	colorinvalid = new int8_t[nSize];
	rowindex     = new int32_t[nSize];
	colindex     = new int32_t[nSize];
	timedata     = new double[nSize];
	e57::CompressedVectorWriter datawriter
			= outfile.SetUpData3DPointsData(
					scan_index, /* data block index */
					nSize, /* size of each buffer */
					xdata, 
					ydata,
					zdata,
					xyzinvalid,
					intdata,
					intinvalid,
					reddata,
					greendata,
					bluedata,
					colorinvalid,
					NULL, /* spherical range */
					NULL, /* spherical azimuth */
					NULL, /* spherical elevation */
					NULL, /* spherical invalid */
					rowindex,
					colindex,
					NULL, /* return index */
					NULL, /* return count */
					timedata,
					NULL  /* time invalid */
					);

	/* iterate over the points, inserting into output file 
	 *
	 * Remember, scanoramas are in column-major order */
	progbar.set_name("   Exporting E57");
	for(c = 0; c < this->num_cols; c++)
	{
		/* update user on progress */
		progbar.update(c, this->num_cols);

		/* populate buffer with current column */
		for(r = 0; r < this->num_rows; r++)
		{
			/* get index of this point */
			i = r + (c * this->num_rows);

			/* add this point to the buffers */
			xdata[r]        = this->points[i].x;
			ydata[r]        = this->points[i].y;
			zdata[r]        = this->points[i].z;
			xyzinvalid[r]   = false;
			intdata[r]      =
				this->points[i].color.get_grayscale();
			intinvalid[r]   = (this->points[i].quality <= 0);
			reddata[r]      = 
				this->points[i].color.get_red_int();
			greendata[r]    =
				this->points[i].color.get_green_int();
			bluedata[r]     = 
				this->points[i].color.get_blue_int();
			colorinvalid[r] = (this->points[i].quality <= 0);
			rowindex[r]     = this->num_rows - r - 1;
			colindex[r]     = c;
			timedata[r]     = this->timestamp;
		}

		/* export buffer */
		datawriter.write(nSize);
	}

	/* success */
	progbar.clear();
	datawriter.close();
	outfile.Close();
	delete[] xdata;
	delete[] ydata;
	delete[] zdata;
	delete[] xyzinvalid;
	delete[] intdata;
	delete[] intinvalid;
	delete[] reddata;
	delete[] greendata;
	delete[] bluedata;
	delete[] colorinvalid;
	delete[] rowindex;
	delete[] colindex;
	delete[] timedata;
	return 0;
}
		
int scanorama_t::writepng(const std::string& filename) const
{
	vector<unsigned char> image; /* RGBA pixel values */
	size_t i, j, r, c, n;
	unsigned int error;

	/* create list of pixels to encode as a png */
	n = this->points.size();
	image.resize(4*n);
	for(i = 0; i < n; i++)
	{
		/* get the row,col index of this point */
		c = i / this->num_rows; /* column-major */
		r = i % this->num_rows; /* column-major */

		/* since the points are stored in column-major order,
		 * but the image wants them in row-major order, save
		 * the colors appropriately */
		j = r * this->num_cols + c; /* row-major, for image */

		/* encode the four color components of each point */
		image[4*j    ] = this->points[i].color.get_red_int();
		image[4*j + 1] = this->points[i].color.get_green_int();
		image[4*j + 2] = this->points[i].color.get_blue_int();
		image[4*j + 3] = 255; /* alpha value */
	}

	/* encode as a png */
	error = lodepng::encode(filename.c_str(), image,
					this->num_cols, this->num_rows);
	if(error)
	{
		cerr << "[scanorama_t::writepng[\tError " << error
		     << ": Unable to export to .png file: \""
		     << filename << "\"" << endl
		     << lodepng_error_text(error) << endl;
		return -1;
	}

	/* success */
	return 0;
}
		
int scanorama_t::writepng_normal(const std::string& filename) const
{	
	vector<unsigned char> image; /* RGBA pixel values */
	size_t i, j, r, c, n;
	unsigned int error;

	/* create list of pixels to encode as a png */
	n = this->points.size();
	image.resize(4*n);
	for(i = 0; i < n; i++)
	{
		/* get the row,col index of this point */
		c = i / this->num_rows; /* column-major */
		r = i % this->num_rows; /* column-major */

		/* since the points are stored in column-major order,
		 * but the image wants them in row-major order, save
		 * the colors appropriately */
		j = r * this->num_cols + c; /* row-major, for image */

		/* encode the four color components of each point */
		image[4*j    ] = (unsigned char) ((this->points[i].nz + 1) * 127.5);
		image[4*j + 1] = (unsigned char) ((this->points[i].ny + 1) * 127.5);
		image[4*j + 2] = (unsigned char) ((this->points[i].nx + 1) * 127.5);
		image[4*j + 3] = 255; /* alpha value */
	}

	/* encode as a png */
	error = lodepng::encode(filename.c_str(), image,
					this->num_cols, this->num_rows);
	if(error)
	{
		cerr << "[scanorama_t::writepng_normal]\tError " << error
		     << ": Unable to export normal to .png file: \""
		     << filename << "\"" << endl
		     << lodepng_error_text(error) << endl;
		return -1;
	}

	/* success */
	return 0;
}
