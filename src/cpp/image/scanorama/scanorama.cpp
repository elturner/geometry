#include "scanorama.h"
#include "scanorama_point.h"
#include <image/fisheye/fisheye_camera.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>
#include <Eigen/Dense>
#include <iostream>
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
	double radius, theta, phi, w, x, y, z;
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
	for(ci = 0; ci < this->num_cols; ci++) /* column-major order */
		for(ri = 0; ri < this->num_rows; ri++)
		{
			/* we want to set the current point to reside
			 * on the unit sphere centered at this->center */
			theta = (2 * M_PI * ci) / this->num_cols;
			phi   = (M_PI * ri) / this->num_rows;
			w     = radius * sin(phi);
			z     = radius * cos(phi);
			y     =  w * sin(theta);
			x     = -w * cos(theta);

			/* store in appropriate point */
			i = ri*this->num_cols + ci;
			this->points[i].x = x;
			this->points[i].y = y;
			this->points[i].z = z;

			/* make some color pattern */
			this->points[i].color.set_2d_pattern(ri, ci);
			this->points[i].quality = -DBL_MAX;
		}
}
		
int scanorama_t::init_geometry(const OctTree<float>& octree,
				double t, const Eigen::Vector3d& cen,
				size_t r, size_t c, double bw)
{
	double radius, theta, phi, w, x, y, z;
	float origin[3]; /* origin of raytracing */
	float d[3]; /* raytracing direction */
	float inter[3]; /* intersection point */
	size_t triangleID;
	size_t ri, ci, i;
	bool suc;

	/* first, clear any existing information */
	this->clear();

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
	for(ci = 0; ci < this->num_cols; ci++) /* column-major order */
		for(ri = 0; ri < this->num_rows; ri++)
		{
			/* we want to set the current point to reside
			 * on the unit sphere centered at this->center */
			theta = (2 * M_PI * ci) / this->num_cols;
			phi   = (    M_PI * ri) / this->num_rows;
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
			}
			else
			{
				/* successfully raytraced, so determine
				 * position of intersect point in
				 * the scan's coordinate system */
				x = inter[0] - origin[0];
				y = inter[1] - origin[1];
				z = inter[2] - origin[2];
			}

			/* store in appropriate point */
			i = ri*this->num_cols + ci;
			this->points[i].x = x;
			this->points[i].y = y;
			this->points[i].z = z;

			/* No color has been assigned yet, so set
			 * it to black */
			this->points[i].color.set(0.0f,0.0f,0.0f);
			this->points[i].quality = -DBL_MAX;
		}

	/* success */
	return 0;
}
		
int scanorama_t::apply(fisheye_camera_t& cam)
{
	progress_bar_t progbar;
	color_t newcolor;
	double px, py, pz, q;
	double dq, w1, w2;
	int ret, r, g, b;
	size_t i, n;

	/* iterate through all points */
	progbar.set_name("Applying image");
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
		ret = cam.color_point(px,py,pz,this->timestamp,r,g,b,q);
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
	progbar.set_name("Exporting");
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
