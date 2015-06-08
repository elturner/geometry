#include "scanorama.h"
#include "scanorama_point.h"
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
	this->init_sphere(0, zero, 1000, 2000);
}

void scanorama_t::init_sphere(double t, const Eigen::Vector3d& cen,
				size_t r, size_t c)
{
	double radius, theta, phi, w, x, y, z;
	size_t ri, ci, i;

	/* first, clear any existing information */
	this->clear();

	/* next, allocate the appropriate number of points */
	this->points.resize(r*c);
	this->timestamp = t;
	this->center    = cen;
	this->num_rows  = r;
	this->num_cols  = c;

	/* iterate over the points and define the geometry of a sphere */
	radius = 1;
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
		}
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
	n = this->points.size();
	for(i = 0; i < n; i++)
	{
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
}
