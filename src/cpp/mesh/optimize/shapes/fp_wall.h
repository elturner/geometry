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
		 * The offset gap to use to compute gradient
		 * should be roughly sqrt(2) * resolution
		 */
		double offset_gap; 

		/**
		 * The edge of the floorplan that this geometry represents
		 */
		fp::edge_t edge;

		/**
		 * This flag indicates whether this shape represents the
		 * original position of the wall or an offset position
		 * for gradient-computation purposes.
		 */
		bool use_offset;

		/**
		 * The line segment that defines the projection of this
		 * wall onto the xy-plane.
		 */
		Eigen::Vector2d edge_pos[fp::NUM_VERTS_PER_EDGE];

		/**
		 * The line segment that defines the project of the
		 * offset version of this wall.  The offset is used to
		 * represent a surface parallel to the wall in order
		 * to compute gradient values.
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
		 * The sum of all scalar values that intersected the
		 * non-offset (original) wall surface.
		 */
		double scalar_sum_orig[fp::NUM_VERTS_PER_EDGE];

		/**
		 * Number of nodes that contributed to analysis
		 * of the non-offset (original) surface.
		 */
		unsigned int num_nodes_orig;

		/**
		 * The sum of all scalar values that intersected the
		 * offset wall surface.
		 */
		double scalar_sum_offset[fp::NUM_VERTS_PER_EDGE];

		/**
		 * Number of nodes that contributed to analysis
		 * of the offset surface.
		 */
		unsigned int num_nodes_offset;

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
		 * @param r    The offset gap to use (units: meters)
		 * @param f    The floorplan to use
		 * @param e    The edge of this floorplan to use as a wall
		 */
		void init(double r, const fp::floorplan_t& f,
		          const fp::edge_t& e);

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Sets the flag whether or not to use the offset surface
		 *
		 * By default, this geometry is defined to use the wall
		 * surface.  However, we also need to insert the geometry
		 * of a slight offset surface, in order to compute
		 * the portion of the gradient along the normal vector
		 * of the wall.  This call will set which of these two
		 * surfaces to use
		 *
		 * @param f  If true, calls to octree.find() use original
		 *           surface.  If false, calls to octree.find()
		 *           use offset surface.
		 */
		inline void toggle_offset(bool f)
		{ this->use_offset = f; };

		/**
		 * Computes force vector this wall exerts on each vertex
		 *
		 * This function should be called after this shape was
		 * inserted twice into the octree (once with offset=true,
		 * and once with offset=false).
		 *
		 * This will yield the force vectors exerted on the vertices
		 * by this wall based on the carved probabilities of the
		 * octree that was used.
		 *
		 * Note that no step size has been included in this
		 * computation, so the force vector may need to be scaled
		 * accordingly.
		 *
		 * @param f0   Where to store force vector for vertex #0
		 * @param f1   Where to store force vector for vertex #1
		 */
		void compute_forces(Eigen::Vector2d& f0, 
		                    Eigen::Vector2d& f1);

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
