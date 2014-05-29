#ifndef FP_HORIZONTAL_H
#define FP_HORIZONTAL_H

/**
 * @file   fp_horizontal.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines fp_horizontal_t, describes fp floors/ceilings
 *
 * @section DESCRIPTION
 *
 * The fp_horizontal_t class is used to describe the geometry of a floor
 * or ceiling in an extruded floorplan mesh.  This horizontal is defined
 * as a polygon whose normal is vertical.  It describes the full (x,y)
 * extent of a room in the floorplan.
 *
 * This class is used to analyze the position of this geometry within
 * the context of a carving defined by an octree.  The octree carving
 * can be used to align the floorplan geometry to be consistent with the
 * carving geometry.
 */

#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <geometry/shapes/extruded_poly.h>
#include <Eigen/Dense>
#include <vector>

/**
 * The fp_horizontal_t class defines the shape of a floor or ceiling 
 */
class fp_horizontal_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The distance between the offset surface and the original
		 * surface of this horizontal.
		 */
		double offset_gap; 

		/**
		 * The shape of this surface can be described by a polygon
		 *
		 * Note that this 'extruded' poly is really represented
		 * as having the same floor and ceiling height, making
		 * it a truly 2D structure embedded in 3D space.
		 */
		extruded_poly_t shape;

		/**
		 * Original z position of this shape
		 */
		double z;

		/**
		 * The normal of this surface must be aligned with the
		 * z-axis, so this value determines if it's pointing
		 * in +z (true) or -z (false)
		 */
		bool norm_up;

		/**
		 * The cost value of the horizontal at the current offset
		 * position.  This value is computed based on how many
		 * exterior nodes the horizontal intersected.
		 */
		double offset_cost;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Initializes floor or ceiling based on given floorplan
		 *
		 * Given a floorplan and an edge within that floorplan,
		 * will define the geometry of this floor or ceiling 
		 * in 3D space.
		 *
		 * This call will also offset the surface from the
		 * surface described in the floorplan geometry by
		 * an offset gap of 'off' (measured in meters).
		 *
		 * @param f          The floorplan to use
		 * @param r          The index of the room to use
		 * @param isfloor    True if floor, false if ceiling
		 * @param off        The offset gap to use (units: meters)
		 */
		void init(const fp::floorplan_t& f, unsigned int ri,
		          bool isfloor, double off);

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Retrieves the final cost of the latest offset
		 *
		 * @return   Returns the computed cost after the last
		 *           call to octree.find(this)
		 */
		double get_offset_cost() const;

		/**
		 * Retrieves the normal vector of this surface
		 *
		 * Since this surface is guaranteed to be horizontal,
		 * then the normal vector only has one component (which
		 * is aligned with +z).
		 */
		inline double get_norm() const
		{ return (this->norm_up ? 1.0 : -1.0); };

		/*----------------------*/
		/* overloaded functions */
		/*----------------------*/

		/**
		 * Returns the number of vertices of this surface
		 *
		 * This corresponds to the number of 2D vertices of the
		 * floorplan room, times two, since this shape
		 * is represented with an extruded_poly_t.
		 *
		 * @return    Returns the number of vertices of shape
		 */
		unsigned int num_verts() const;
		
		/**
		 * Retrieves the i'th vertex of this horizontal in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		Eigen::Vector3d get_vertex(unsigned int i) const;
		
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
		 * This function will allow the shape to modify the
		 * data stored at leaf nodes that it intersects.  It
		 * will be given the current data element, and will
		 * return the modified data element.  If the input
		 * is null, this function will allocate a
		 * new octdata_t object to use.
		 *
		 * Typically, the return value should be the same as
		 * the input.
		 *
		 * @param c    The center position of leaf node
		 * @param hw   The half-width of leaf node
		 * @param d    The original data, can be null
		 *
		 * @return     Returns pointer to the modified data
		 */
		octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                         double hw, octdata_t* d);
};

#endif
