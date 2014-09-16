#ifndef SHAPE_WRAPPER_H
#define SHAPE_WRAPPER_H

/**
 * @file   shape_wrapper.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Wrapper class around arbitrary shapes to find intersecting nodes
 *
 * @section DESCRIPTION
 *
 * This class is used to wrap around an arbitrary shape object.  It
 * overrides the "apply to leaf" function, in order to record the
 * information for each leaf it intersects.
 */

#include <geometry/octree/shape.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/octree.h>
#include <vector>
#include <Eigen/Dense>

class shape_wrapper_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The shape to mimick.  When set to a shape, this
		 * shape wrapper will have the same geometric 
		 * characteristics as the shape stored here.
		 */
		shape_t* shape;

	/* public parameters */
	public:

		/**
		 * This list of octdata elements is populated whenever
		 * an octree is analyzed
		 */
		std::vector<octdata_t*> data;

		/**
		 * This list of positions indicates the center positions
		 * of each leaf node that was intersected in the most
		 * recent analysis.
		 */
		std::vector<const Eigen::Vector3d*> centers;

		/**
		 * This list of halfwidths indicates the sizes of the
		 * leaf nodes that were intersected.
		 */
		std::vector<double> halfwidths;

	/* functions */
	public:

		/* constructors */

		/**
		 * Constructs default shape wrapper
		 */
		shape_wrapper_t()
		{ this->clear(); };

		/**
		 * Clears all info from this structure
		 */
		inline void clear()
		{
			this->shape = NULL;
			this->data.clear();
			this->centers.clear();
			this->halfwidths.clear();
		};

		/**
		 * Will find all leafs of the tree that are intersected
		 * by the given shape.
		 *
		 * Will apply the given shape to the tree, but rather
		 * than performing any action on the intersecting nodes,
		 * will just record which leafs were intersected.
		 *
		 * After this call, the parameters of this object will
		 * be populated with the analysis.
		 *
		 * @param s      The shape to use
		 * @param tree   The tree to analyze
		 */
		void find_in_tree(shape_t& s, octree_t& tree)
		{
			/* clear any existing data */
			this->clear();

			/* use the given shape */
			this->shape = &s;

			/* apply to tree */
			tree.find(*this);
		};

		/*----------------------*/
		/* overloaded functions */
		/*----------------------*/

		/**
		 * Retrieves the number of vertices that compose this line
		 *
		 * @return   The number of vertices in line
		 */
		inline unsigned int num_verts() const
		{
			/* wrap referenced shape */
			if(this->shape == NULL)
				return 0;
			return this->shape->num_verts();
		};
		
		/**
		 * Retrieves the i'th vertex of shape in 3D space
		 *
		 * @param i  The vertex index to retrieve
		 *
		 * @return   The i'th vertex
		 */
		inline Eigen::Vector3d get_vertex(unsigned int i) const
		{
			Eigen::Vector3d dummy(0,0,0);

			/* wrap referenced shape */
			if(this->shape == NULL)
				return dummy;
			return this->shape->get_vertex(i);
		};

		/**
		 * Tests intersection of this shape with an
		 * axis-aligned bounding box.
		 *
		 * @param c   The center of the box
		 * @param hw  The halfwidth of the box
		 *
		 * @return    Returns true if the box is intersected.
		 */
		inline bool intersects(const Eigen::Vector3d& c,
		                       double hw) const
		{
			if(this->shape == NULL)
				return false;
			return this->shape->intersects(c, hw);
		};


		/**
		 * This function will be called when intersected with
		 * a leaf in the octree.
		 *
		 * @param c     The center position of the leaf
		 * @param hw    The halfwidth of the leaf
		 * @param data  The leaf's data
		 *
		 * @return      The leaf's data
		 */
		inline octdata_t* apply_to_leaf(const Eigen::Vector3d& c,
		                                double hw,
		                                octdata_t* d)
		{
			/* store this info */
			this->centers.push_back(&c);
			this->halfwidths.push_back(hw);
			this->data.push_back(d);
			return d;
		};
};

#endif
