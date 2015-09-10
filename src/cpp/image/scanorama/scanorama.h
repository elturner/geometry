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
#include <image/fisheye/fisheye_camera.h>
#include <geometry/raytrace/OctTree.h>
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
		 * grid should be complete and stored in column-major order.
		 *
		 * This list should be exactly num_rows * num_cols 
		 * in length.
		 *
		 * These points are stored in world orientation, but NOT
		 * world translation.  You need to apply the this->center
		 * translation to go from scan coordinates to world
		 * coordinates.  No rotation is needed.
		 */
		std::vector<scanorama_point_t> points;

		/**
		 * These values indicate the number of points stored
		 *
		 * The points should be in "gridded" column-major order.
		 * 
		 * (num_rows * num_cols) should equal points.size()
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

		/**
		 * Blending value.
		 *
		 * This value represents how aggressively colors
		 * between separate images will be blended together.
		 *
		 * The value should be in the range [0,1].  If zero,
		 * no blending will occur.  If one, LOTS of blending will
		 * occur.
		 */
		double blendwidth;

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
				timestamp(other.timestamp),
				blendwidth(other.blendwidth)
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
			this->num_rows   = 0;
			this->num_cols   = 0;
			this->timestamp  = 0.0;
			this->blendwidth = 0.0;
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
		 * @param bw  The blending width to use (range [0,1])
		 */
		void init_sphere();
		void init_sphere(double t, const Eigen::Vector3d& cen,
				size_t r, size_t c, double bw);

		/**
		 * Initialize the point geometry from the specified model
		 *
		 * Given a mesh model stored in an octree, position each
		 * point in this scanorama by raytracing from the scan
		 * center in each pixel direction.
		 *
		 * @param octree   The octree model to use
		 * @param t        The timestamp to set for this scanorama
		 * @param cen      The center position of this scanorama
		 * @param r        Number of rows to use
		 * @param c        Number of columns to use
		 * @param bw       The blendwidth to use (range [0,1])
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init_geometry(const OctTree<float>& octree,
				double t, const Eigen::Vector3d& cen,
				size_t r, size_t c, double bw);

		/*-------*/
		/* color */
		/*-------*/

		/**
		 * Applies color of specified camera to this scanorama
		 *
		 * Given a camera object, which tracks a camera through
		 * time, will use the camera's closest temporal image to
		 * apply color to the points of this scanorama.  Any point
		 * whose quality is improved by this camera's imagery will
		 * have its color replaced.
		 *
		 * @param cam  The camera object to utilize
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int apply(camera_t* cam);

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

		/**
		 * Export to PTG file
		 *
		 * Exports the contents of this scanorama to the
		 * specified .ptg file.  The Cyclone PTG File
		 * Format was developed by Leica and its 
		 * specification can be found here:
		 *
		 * www.xdesy.de/freeware/PTG-DLL/PTG-1.0.pdf
		 *
		 * This function will export one binary file based
		 * on this specification.
		 *
		 * @param filename   The path to output file to write
		 *
		 * @return     Returns zero on success, non-zero on
		 *             failure.
		 */
		int writeptg(const std::string& filename) const;

		/**
		 * Export to E57 output file
		 *
		 * Will export this scanorama to a E57 file, as specified
		 * by the ASTM E57.3 data interchange standard for 3D
		 * laser scanning.
		 *
		 * This function makes calls to the libe57 library.
		 *
		 * @param filename    The .e57 file to write to.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writee57(const std::string& filename) const;

		/**
		 * Export to PNG image
		 *
		 * Exports the content of this scanorama to the
		 * specified png image file.
		 *
		 * @param filename   The file to write to as a png image
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int writepng(const std::string& filename) const;

		/**
		 * Export the normal-map to a PNG image
		 *
		 * Exports the contents of this scanorama's normal fields to
		 * the specified png image file.
		 *
		 * A normal map is encoded as a 24-bit png file. 
		 * Each pixel's B, G, R values from [0, 255] map to 
		 * its normal's X, Y, Z from [-1, 1].
		 *
		 * @param filename   The file to write to as a png normal map image
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int writepng_normal(const std::string& filename) const;

		/**
		 * Export the depth-map to a png image
		 *
		 * Exports the contents of this scanorama's depth fields to
		 * the specified png image file.
		 *
		 * A depth map is encoded as a 16-bit grayscale png file. 
		 * The distance of each pixel from the camera is that 16-bit 
		 * number from [0, 2^16 - 1] in centimeters.
		 *
		 * @param filename   The file to write to as a png depth map image
		 *
		 * @return   Return zero on success, non-zero on failure.
		 */
		int writepng_depth(const std::string& filename) const;
};

#endif
