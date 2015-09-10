#ifndef SCANORAMA_POINT_H
#define SCANORAMA_POINT_H

/**
 * @file    scanorama_point.h
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   scanorama_point_t class represents a single point in a scanorama
 *
 * @section DESCRIPTION
 *
 * This file contains the scanorama_point_t class, which is used to store
 * a single point in a scanorama.  One point contains 3D position and color
 * information.
 */

#include <image/color.h>

/**
 * The scanorama_point_t class stores a single point for a scanorama
 */
class scanorama_point_t
{
	/* parameters */
	public:

		/**
		 * The 3D position of this point
		 *
		 * This position should be in units of meters.
		 */
		double x;
		double y;
		double z;

		/**
		 * The normal vector of this point
		 */
		double nx;
		double ny;
		double nz;

		/**
		 * The width of this point
		 *
		 * Effectively half the distance to the next point
		 * in the grid.  This value is used for anti-aliasing
		 * during coloring the points.
		 */
		double width;

		/**
		 * The color of this point
		 */
		color_t color;

		/**
		 * The quality value of the color of this point
		 *
		 * Bigger is better for this value.
		 */
		double quality;
};

#endif
