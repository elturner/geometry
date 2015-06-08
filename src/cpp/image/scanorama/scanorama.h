#ifndef SCANORAMA_H
#define SCANORAMA_H

/**
 * @file     scanorama.h
 * @author   Eric Turner <elturner@indoorreality.com>
 * @brief    The scanorama_t class represents a single scanorama pointcloud
 *
 * @section DESCRIPTION
 *
 * A scanorama pointcloud is used to represent a panoramic image with
 * depth information.  Each "pixel" of the panorama is stored as a
 * (x,y,z) point in 3D space, along with color.
 */

#include "scanorama_point.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>

/**
 * The scanorama_t class represents one scanorama
 *
 * A scanorama is composed of a grid of 3D points.  Each point has
 * an associated color, which is also stored.
 */
class scanorama_t
{
	/* parameters */
	private:

		/**
		 * The content of this scanorama
		 *
		 * The scanorama is composed of a grid of points, where
		 * each point contains a 3D position and a color.  The
		 * grid should be complete and stored in row-major order.
		 *
		 * This list should be exactly num_rows * num_cols 
		 * in length.
		 */
		std::vector<scanorama_point_t> points;

		/**
		 * These values indicate the number of points stored
		 *
		 * The points should be in "gridded" row-major order.
		 * 
		 * (num_rows * num_cols) should equal points.cols()
		 */
		size_t num_rows, num_cols;

		/**
		 * The timestamp of this scanorama
		 *
		 * units: seconds
		 */
		double timestamp;

		/**
		 * The spatial center position of this scanorama
		 *
		 * units:  meters
		 */
		Eigen::Vector3d center;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs an empty scanorama
		 */
		scanorama_t()
		{ this->clear(); }

		/**
		 * Constructs scanorama based off of given scanorama
		 *
		 * @param other   The scanorama to copy
		 */
		scanorama_t(const scanorama_t& other)
			:	points(other.points),
				num_rows(other.num_rows),
				num_cols(other.num_cols),
				timestamp(other.timestamp)
		{};

		/**
		 * Frees all memory and resources
		 */
		~scanorama_t()
		{ this->clear(); }

		/**
		 * Frees all memory and resources
		 */
		inline void clear()
		{
			this->points.clear();
			this->num_rows  = 0;
			this->num_cols  = 0;
			this->timestamp = 0.0;
		};

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Initializes the point geometry with a sphere
		 * of unit radius.
		 *
		 * While the resulting set of points does not represent
		 * the real-world geometry observed in the dataset, this
		 * function is useful to just have some sort of surface
		 * to project color onto.
		 *
		 * Any information stored in this object before this
		 * function call will be lost.
		 *
		 * @param t   The timestamp to set for this scanorama
		 * @param cen The center position of this scanorama
		 * @param r   Number of rows to use
		 * @param c   Number of columns to use
		 */
		void init_sphere();
		void init_sphere(double t, const Eigen::Vector3d& cen,
				size_t r, size_t c);

		/*-------*/
		/* color */
		/*-------*/

		// TODO

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Export to OBJ stream
		 *
		 * Exports the contents of this scanorama to the
		 * specified Wavefront OBJ stream.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;

		/**
		 * Export to PTX stream
		 *
		 * Exports the contents of this scanorama to the
		 * specified stream, formatted as a .ptx file.  This
		 * file format is defined by Leica Geosystems.
		 *
		 * See:
		 *
		 * 	http://www.leica-geosystems.com/kb/
		 * 		?guid=5532D590-114C-43CD-A55F-FE79E5937CB2
		 *
		 * @param os   The output stream to write to 
		 */
		void writeptx(std::ostream& os) const;
};

#endif
