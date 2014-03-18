#ifndef SHAPE_H
#define SHAPE_H

/**
 * @file shape.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The shape_t class is a virtual interface that allows
 * for different shapes to be used to form an octree.  By
 * defining how a shape intersects an octnode, and what
 * happens when a shape is intersected, users can modify
 * the tree by carving with different implementations of
 * this interface.
 *
 * This class requires the Eigen framework
 */

#include "octdata.h"
#include <Eigen/Dense>

/**
 * The shape_t virtual interface
 */
class shape_t
{
	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Frees all memory and resources
		 */
		virtual ~shape_t() {};

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Retrieves the number of vertices that compose this shape
		 *
		 * @return   The number of vertices in shape
		 */
		virtual unsigned int num_verts() const =0;

		/**
		 * Retrieves the i'th vertex of shape in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		virtual Eigen::Vector3d get_vertex(unsigned int i) const =0;

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
		virtual bool intersects(const Eigen::Vector3d& c,
		                        double hw) const =0;

		/**
		 * Will be called on leaf nodes this shape intersects
		 *
		 * This function will allow the shape to modify the
		 * data stored at leaf nodes that it intersects.  It
		 * will be given the current data element, and should
		 * return the modified data element.  If the input
		 * is null, this function is expected to allocate a
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
		virtual octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                                 double hw,
		                                 octdata_t* d) =0;
};

#endif
