#ifndef CARVE_WEDGE_H
#define CARVE_WEDGE_H

/**
 * @file carve_wedge.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The carve_wedge_t class is used to represent a volume in 3D space,
 * and correlate the continuous mapping functions from a carve_map_t
 * to this volume, which allows a carve_map_t to be expressed over
 * a finite volume where it has the most impact.
 *
 * The wedge is formed from four carve_map_t objects, which all
 * contribute to the mapping inside the volume of the wedge.
 *
 * This wedge is also used as a shape that can intersect with octrees,
 * allowing for efficient insertion of carve maps into octrees.
 *
 * This file requires the Eigen framework.
 */

#include <geometry/carve/gaussian/carve_map.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <iostream>
#include <Eigen/Dense>

/* the following defines are used for this class */
#define NUM_MAPS_PER_WEDGE 4 /* number of scanpoints to make a wedge */
#define NUM_VERTICES_PER_WEDGE 6 /* number of vertices in polyhedron */

/**
 * This class represents a 3D shape formed by four scan points
 *
 * This shape originates from four scan points, two neighboring scans
 * from one frame, then the same indices in the successor frame.
 */
class carve_wedge_t : public shape_t
{
	/* parameters */
	private:

		/* the following are pointers to carve maps to
		 * use to define this shape.  This object is NOT
		 * responsible for the memory allocation of these
		 * maps, which is handled elsewhere. */
		carve_map_t* maps[NUM_MAPS_PER_WEDGE];

		/* the following define the world-coordinate vertices
		 * of this wedge.  These vertices are placed outside
		 * the mean-positions of the scans, so that the wedge
		 * includes not only the mean positions but an extra
		 * buffer to carve. */
		Eigen::Vector3d verts[NUM_VERTICES_PER_WEDGE];

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		/**
		 * Initializes empty wedge
		 */
		carve_wedge_t();

		/**
		 * Initialize this wedge with input carve maps
		 *
		 * Given four carve maps, will initialze the shape of
		 * this wedge.  Note that the wedge will be defined to
		 * be larger than the mean positions of the originating
		 * scans.  The buffer width of this increase is proportional
		 * to the standard deviation of the distributions provided,
		 * and scaled by the input buffer factor.
		 *
		 * @param a1   carve map indicating frame #j, point #i
		 * @param a2   carve map indicating frame #j, point #i+1
		 * @param b1   carve map indicating frame #j+1, point #i
		 * @param b2   carve map indicating frame #j+1, point #i+1
		 * @param nb   number of standard deviations of buffer
		 */
		void init(carve_map_t* a1, carve_map_t* a2,
		          carve_map_t* b1, carve_map_t* b2,
			  double nb);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Retrieves the number of vertices that compose this shape
		 *
		 * @return   The number of vertices in shape
		 */
		inline unsigned int num_verts() const
		{ return NUM_VERTICES_PER_WEDGE; };
		
		/**
		 * Retrieves the i'th vertex of shape in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{ return this->verts[i]; };

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Checks if this shape intersects a given axis-aligned box
		 *
		 * Given the parameters of an Axis-Aligned Bounding Box
		 * (AABB), will check if this wedge shape intersects the
		 * AABB.
		 *
		 * @param c       The center of the box, in 3D space
		 * @param hw      Half the width of an edge of the box
		 *
		 * @return    Returns true iff this intersects given box
		 */
		bool intersects(const Eigen::Vector3d& c, double hw) const;
		
		/**
		 * Helper function for intersection()
		 *
		 * Performs intersection test by representing the
		 * wedge as a set of line segments (interpolated within
		 * wedge volume), and performs a series of line/cube
		 * intersection tests.
		 */
		bool intersects_rays(const Eigen::Vector3d& c,
				double hw) const;
		
		/**
		 * Helper function for intersection()
		 *
		 * Performs intersection test by representing the
		 * wedge as a set of triangles and performing
		 * triangle/cube intersections.
		 */
		bool intersects_tris(const Eigen::Vector3d& c, 
				double hw) const;

		/**
		 * Applies this mapping to the given data element
		 *
		 * Given a data element, its location, and size, will
		 * compute the carve mapping for that location and store
		 * it in the given data element.  If the given data element
		 * is null, this function call will allocate a new element
		 * and return it.  If the input is not null, then it will
		 * be modified and the same element will be returned.
		 *
		 * @param c     The location of this data element in space
		 * @param hw    The half-width of this data element
		 * @param d     The data element to modify, can be null
		 *
		 * @return    Returns the modified data, or new data object.
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Prints debugging parameters about this wedge
		 *
		 * @param os  The output stream to print to
		 */
		void print_params(std::ostream& os) const;

		/**
		 * Will export this wedge to an OBJ file stream
		 *
		 * Will write out the triangles and vertices of this
		 * wedge to a Wavefront OBJ formatted file stream.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;

		/**
		 * Will export this wedge to a XYZ file stream
		 *
		 * Will write out the vertices of this wedge to
		 * a pointcloud file.
		 *
		 * @param os   The output stream to write to
		 */
		void writexyz(std::ostream& os) const;
};

#endif
