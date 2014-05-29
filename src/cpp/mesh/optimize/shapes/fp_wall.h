#ifndef FP_WALL_H
#define FP_WALL_H

/**
 * @file   fp_wall.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines the fp_wall_t class, describes fp geometry
 *
 * @section DESCRIPTION
 *
 * The fp_wall_t class is used to describe the geometry of a wall in an
 * extruded floorplan mesh.  This wall is defined as a rectangle whose
 * normal is horizontal.  It originates from two 2D wall samples within
 * the floorplan.
 *
 * This class is used to analyze the position of this geometry within
 * the context of a carving defined by an octree.  The octree carving
 * can be used to align the floorplan geometry to be consistent with the
 * carving geometry.
 */

#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <Eigen/Dense>

/* the following defines are used by this shape */
#define NUM_VERTS_PER_RECT 4 /* a wall geometry is defined by a rectangle */

/**
 * The fp_wall_t class defines the shape of a single wall in a floorplan
 */
class fp_wall_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The distance between the offset surface and the original
		 * surface of this wall.
		 */
		double offset_gap; 

		/**
		 * The edge of the floorplan that this geometry represents
		 */
		fp::edge_t edge;

		/**
		 * The line segment that defines the projection of the
		 * original wall surface onto the xy-plane.
		 */
		Eigen::Vector2d edge_pos[fp::NUM_VERTS_PER_EDGE];

		/**
		 * The line segment that defines the project of the
		 * offset version of this wall.  The offset is used to
		 * represent a surface parallel to the wall.
		 */
		Eigen::Vector2d offset_edge_pos[fp::NUM_VERTS_PER_EDGE];

		/**
		 * Normal vector of edge
		 */
		Eigen::Vector2d norm;

		/**
		 * This vector represents the unit vector tangent
		 * to the edge defining this wall
		 */
		Eigen::Vector2d tangent;
		
		/**
		 * The length of the edge
		 */
		double length;

		/**
		 * The minimum vertical position of the wall
		 *
		 * This defines the height of the floor that is
		 * connected to this wall.
		 */
		double min_z;

		/**
		 * The maximum vertical position of the wall
		 *
		 * This defines the height of the ceiling that is
		 * connected to this wall.
		 */
		double max_z;

		/**
		 * The cost value of the wall at the current offset
		 * position.  This value is computed based on how many
		 * exterior nodes the wall intersected.
		 */
		double offset_cost;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Initializes this wall position based on given floorplan
		 *
		 * Given a floorplan and an edge within that floorplan,
		 * will define the geometry of this wall shape in 3D space.
		 *
		 * @param f    The floorplan to use
		 * @param e    The edge of this floorplan to use as a wall
		 */
		void init(const fp::floorplan_t& f, const fp::edge_t& e);

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Sets the offset position of this surface
		 *
		 * This call will reset the geometry associated with
		 * the offset of this surface and any stored information
		 * about previous offsets.
		 *
		 * @param off   The offset gap to use (units: meters)
		 */
		void set_offset(double off);

		/**
		 * Retrieves the final cost of the latest offset
		 *
		 * @return   Returns the computed cost after the last
		 *           call to octree.find(this)
		 */
		double get_offset_cost() const;

		/**
		 * Retrieves the normal vector of this wall
		 */
		inline Eigen::Vector2d get_norm() const
		{ return this->norm; };

		/*----------------------*/
		/* overloaded functions */
		/*----------------------*/

		/**
		 * Returns the number of vertices of this wall
		 *
		 * Since a wall is defined to be a rectangle, this
		 * will always return four vertices.
		 *
		 * @return    Returns the number of vertices of this wall
		 *            (which will always be 8).
		 */
		inline unsigned int num_verts() const
		{ return NUM_VERTS_PER_RECT; };
		
		/**
		 * Retrieves the i'th vertex of this wall in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		Eigen::Vector3d get_vertex(unsigned int i) const;
		
		/**
		 * Checks if this wall intersects an octnode
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
