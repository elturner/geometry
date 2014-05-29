#ifndef EXTRUDED_POLY_H
#define EXTRUDED_POLY_H

/**
 * @file extruded_poly.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the extruded_poly_t, which implements
 * the shape_t interface for octrees.  It is used to intersect
 * a room from an extruded floor plan with an octree.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <mesh/floorplan/floorplan.h>
#include <iostream>
#include <Eigen/Dense>

/**
 * This class defines an extruded oriented polygon in 3D space
 */
class extruded_poly_t : public shape_t
{
	/* parameters */
	private:

		/* this represents the global index of the room, which is
		 * unique across all rooms across all floorplans */
		int room_index; /* index starts at zero, neg => invalid */

		/* these values indicate the floor and ceiling height
		 * of the room, which are useful for fast culling when
		 * determining intersections */
		double floor_height; /* units: meters */
		double ceiling_height; /* units: meters */

		/* the list of 3D vertices that represent the floor
		 * vertices.  To generate ceiling vertices, take a floor
		 * vertex and add (ceiling_height - floor_height) to the
		 * z-component */
		Eigen::MatrixXd verts; /* dimensions: 3 x num verts */

		/* the following denote the topology of the room,
		 * which is represented by a list of 2D triangles
		 * making up the floor plan. */
		Eigen::MatrixXi tris; /* dimensions: 3 x num triangles */

		/* The list of boundary edges of this triangulation */
		Eigen::MatrixXi edges; /* dimensions: 2 x num edges */

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
		
		/**
		 * Initializes empty extruded polygon
		 */
		extruded_poly_t();

		/**
		 * Initializes this shape from a floorplan room
		 *
		 * Given a floorplan and a room to reference, will
		 * set the geometry of this shape to be that of
		 * the room.
		 *
		 * The value gi should be a globally unique index of the
		 * room, which may be different than ri if there are 
		 * multiple floor plans in play, which can happen if a 
		 * building has multiple levels.
		 *
		 * The value ri given should be the local index of the
		 * room within the f structure.
		 *
		 * @param f     The floorplan to reference
		 * @param gi    The global index of room
		 * @param ri    The room index to use
		 */
		void init(const fp::floorplan_t& f,
		          unsigned int gi, unsigned int ri);
		
		/**
		 * Initialize shape from floorplan with manual heights
		 *
		 * Will perform the same action as init(f,gi,ri), but
		 * will force the generate shape to have floor height
		 * of fh and ceiling height of ch.
		 *
		 * @param f     The floorplan to reference
		 * @param gi    The global index of room
		 * @param ri    The room index to use
		 * @param fh    The floor height to use
		 * @param ch    The ceiling height to use
		 */
		void init(const fp::floorplan_t& f,
		          unsigned int gi, unsigned int ri,
		          double fh, double ch);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Gives the number of vertices that represent this 3D shape
		 *
		 * The number of vertices returned will be twice the
		 * number of 2D vertices used to make up the floor plan
		 * of the originating room, since there are equal number
		 * of vertices on the floor and the ceiling.
		 *
		 * @return   Returns number of 3D vertices in shape
		 */
		inline unsigned int num_verts() const
		{ return (2 * this->verts.cols()); };

		/**
		 * Retrieves the i'th vertex in the shape
		 *
		 * Will return the position of the specified vertex
		 * in 3D space.
		 *
		 * @param i   The index of the vertex to return
		 *
		 * @return    Returns the i'th vertex's position
		 */
		Eigen::Vector3d get_vertex(unsigned int i) const;

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Checks if this shape intersects an octnode
		 *
		 * By checking this shape against the parameters
		 * of an axis-aligned bounding box, should determine
		 * if the 3D shape intersects the volume of the box.
		 *
		 * @param c    The center of the box
		 * @param hw   The half-width of the box
		 *
		 * @return   Returns true iff the shape intersects the box
		 */
		bool intersects(const Eigen::Vector3d& c, double hw) const;
		
		/**
		 * Will be called on leaf nodes this shape intersects
		 *
		 * Will set this octdata to intersect the room represented
		 * by this shape. The return value will be the same as the
		 * input.
		 *
		 * @param c    The center position of leaf node
		 * @param hw   The half-width of leaf node
		 * @param d    The original data, can be null
		 *
		 * @return     Returns pointer to the modified data d
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Exports this shape to a Wavefront OBJ file stream
		 *
		 * Will generate lines of a wavefront .obj formatted
		 * file that represent this shape.
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;
};

#endif
