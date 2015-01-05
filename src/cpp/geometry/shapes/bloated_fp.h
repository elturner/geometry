#ifndef BLOATED_FP_H
#define BLOATED_FP_H

/**
 * @file   bloated_fp.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The shape object representing a floorplan with buffer distance
 *
 * @section DESCRIPTION
 *
 * This file defines the bloated_fp_t, which implements
 * the shape_t interface for octrees.  It is used to intersect
 * an entire floorplan with an octree.  Any nodes that are significantly
 * far away from the floorplan are removed from an internal list.
 *
 * The goal is to find all octree nodes that are 'explosions'.  That
 * is, are very far away from the floorplan, indicating they are outside
 * the building, which means they shouldn't be modeled.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <mesh/floorplan/floorplan.h>
#include <Eigen/Dense>
#include <iostream>
#include <set>

/**
 * This class defines a bloated floorplan in 3D space
 */
class bloated_fp_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * these values indicate the minimum floor height
		 * and maximum ceiling heights across all rooms in 
		 * this floorplan.
		 *
		 * These values will be further modified to include
		 * the specified buffer distance.
		 */
		double floor_height; /* units: meters */
		double ceiling_height; /* units: meters */

		/**
		 * The 2D bounding box for the floorplan
		 *
		 * Again, these values incorporate the buffer distance
		 * specified that 'bloats' this floorplan
		 */
		double bounds_x[2]; /* min, max */
		double bounds_y[2]; /* min, max */

		/**
		 * The following indicate the 'bloated' triangles of 
		 * the floorplan.  Each triangle has been modified
		 * so that its vertices are pushed by the buffer distance
		 * away from the centroid of the triangle.
		 *
		 * Each column represents one triangle in 2D:
		 *
		 *	px
		 *	py
		 *	qx
		 *	qy
		 *	rx
		 *	ry
		 *
		 * For triangle with vertices (p,q,r).
		 */
		Eigen::MatrixXd tris; /* dimensions: 6 x num triangles */

		/**
		 * This list stores any nodes that intersect
		 * with the bloated floorplan.
		 *
		 * This indicates nodes that are allowed to be included in
		 * interior of the carved model, since they are not 
		 * considered 'explosions'.
		 */
		std::set<octdata_t*> whitelist;

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
		bloated_fp_t();

		/**
		 * Initializes this shape from a floorplan room
		 *
		 * Given a floorplan to reference and a buffer distance, 
		 * will set the geometry of this shape to be a bloated
		 * version of the floorplan, covering more area by the
		 * given distance in all directions.
		 *
		 * @param f       The floorplan to reference
		 * @param buffer  The buffer distance to use (units: meters)
		 */
		void init(const fp::floorplan_t& f, double buffer);
		
		/**
		 * Initialize shape from floorplan with manual heights
		 *
		 * Will perform the same action as init(f,buffer), but
		 * will force the generate shape to have floor height
		 * of fh and ceiling height of ch.
		 *
		 * Note these values for the heights are before any 
		 * buffer is applied.
		 *
		 * @param f      The floorplan to reference
		 * @param fh     The floor height to use
		 * @param ch     The ceiling height to use
		 * @param buffer The buffer distance (units: meters)
		 */
		void init(const fp::floorplan_t& f,
		          double fh, double ch, double buffer);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Gives the number of vertices that represent this 3D shape
		 *
		 * The number of vertices returned will be twice the
		 * number of 2D vertices used to make up the floor plan,
		 * since there are equal number
		 * of vertices on the floor and the ceiling.
		 *
		 * Each triangle contains three vertices, so it contributes
		 * a total of 6 vertices in this count.
		 *
		 * @return   Returns number of 3D vertices in shape
		 */
		inline unsigned int num_verts() const
		{ return (6 * this->tris.cols()); };

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

		/**
		 * Retrieves the beginning iterator for the whitelist
		 */
		inline std::set<octdata_t*>::const_iterator begin() const
		{ return this->whitelist.begin(); };

		/**
		 * Retrieves the ending iterator for the whitelist
		 */
		inline std::set<octdata_t*>::const_iterator end() const
		{ return this->whitelist.end(); };

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
