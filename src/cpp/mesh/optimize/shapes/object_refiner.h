#ifndef OBJECT_REFINER_H
#define OBJECT_REFINER_H

/**
 * @file object_refiner.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief This class is used to selectively refine data in an octree
 *
 * @section DESCRIPTION
 *
 * This class is used to remove data elements in an octree that correspond
 * to objects.  This requires the octree to have already imported floorplan
 * information.  The motivation for removing these data elements is so
 * those locations can be recarved at a finer resolution.
 */

#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octdata.h>
#include <geometry/octree/shape.h>
#include <Eigen/StdVector>
#include <Eigen/Dense>
#include <iostream>
#include <string>
#include <vector>
#include <map>

/* the following classes are defined in this file */
class object_refiner_t;
class node_location_t;

/**
 * Identifies and refines areas of the tree that represent objects
 *
 * The object_refiner_t class will intersect with all nodes, identify
 * nodes that represent objects in the environment, and remove those
 * sections of the tree so that they can be recarved at a finer resolution
 * later.
 */
class object_refiner_t : public shape_t
{
	/* parameters */
	private:

		/**
		 * The amount to increase an octree's depth at object nodes
		 *
		 * This class will refine an octree's geometry at the
		 * location of object nodes.  This value indicates how
		 * many levels of the octree to increase at these locations.
		 */
		unsigned int object_depth_increase;

		/**
		 * The input chunklist file to reference
		 *
		 * This file contains the list of chunks that were used
		 * to populate the octree.  This acts as the reference of
		 * chunk locations in order to determine which chunks to
		 * recarve.
		 */
		chunk::chunklist_reader_t chunklist;

		/**
		 * The input list of wedges to reference
		 *
		 * This file contains the list of wedges that are
		 * referenced by the chunks.  These wedges will be
		 * the shapes used to recarve the octree.
		 */
		wedge::reader_t wedges;

		/**
		 * The input list of carvemaps to reference
		 *
		 * This file represents the carvemaps associated with
		 * the original scans of the system.  These are referenced
		 * by the wedge shapes and define the scan geometry used
		 * to carve the octree.
		 */
		cm_io::reader_t carvemaps;

		/**
		 * A list of node locations to modify
		 *
		 * For each data element we remove, we want to keep
		 * track of the floorplan room it came from.  This
		 * list represents the locations in the environment
		 * that were modified, and what room they reside in.
		 */
		std::vector<node_location_t> nodes;

	/* functions */
	public:

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Initializes this object with the appropriate input files
		 *
		 * Will initialize the parameters of this object, including
		 * loading the specified files.
		 *
		 * @param inc_depth         The increase in depth to 
		 *                          enact at object nodes
		 * @param chunklistfile     The input .chunklist file
		 * @param wedgefile         The input .wedge file
		 * @param cmfile            The input .carvemap file
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int init(unsigned int inc_depth,
		         const std::string& chunklistfile,
		         const std::string& wedgefile,
		         const std::string& cmfile);

		/**
		 * Refines nodes of the given octree that represent objects
		 *
		 * Will manipulate the given octree by identifying the
		 * location of objects in the environment, and recarving
		 * these locations at a finer resolution.  Objects are
		 * defined as exterior nodes that are contained within
		 * the volume of the floorplan.  This call must occur
		 * on an octree that has been merged with a floorplan.
		 *
		 * @param tree   The octree to refine
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int refine(octree_t& tree);

		/*---------------------*/
		/* container accessors */
		/*---------------------*/

		// TODO make better container for nodes

		/**
		 * Clears all values from this structure
		 */
		inline void clear()
		{ this->nodes.clear(); };

		/*----------------------*/
		/* overloaded functions */
		/*----------------------*/

		/**
		 * Returns the number of vertices of this shape
		 *
		 * Since this object will always intersect any existing
		 * node, it doesn't need any explicit vertices, so
		 * this function will return zero.
		 *
		 * @return    Returns the number of vertices
		 */
		inline unsigned int num_verts() const
		{ return 0; };
		
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

		/*---------------------*/
		/* debugging functions */
		/*---------------------*/

		/**
		 * Will export the found posiitons to the Wavefront
		 * OBJ formatted stream.
		 *
		 * @param os   Where to write the data
		 */
		void writeobj(std::ostream& os) const;
};

/**
 * Identifies the location and size of a node in the tree
 *
 * Objects of this class represent the persistent data of nodes
 * in the octree that will be refined.
 */
class node_location_t
{
	/* parameters */
	private:
		
		/**
		 * The room id index of this node
		 */
		int room_id;

		/**
		 * The center position of the node
		 */
		double x, y, z;

		/**
		 * The halfwidth of this node
		 */
		double hw;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs default object
		 */
		node_location_t()
		{
			/* default values */
			this->room_id = -1;
			this->x = this->y = this->z = this->hw = 0;
		};

		/**
		 * Constructs object with specified parameters
		 *
		 * @param rid   The room id of this node
		 * @param c     The center position of this node
		 * @param h     The halfwidth of this node
		 */
		node_location_t(int rid,const Eigen::Vector3d& c,double h)
		{ this->set(rid,c,h); };

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Sets the parameter values of this object
		 *
		 * @param rid   The room id of this node
		 * @param c     The center position of this node
		 * @param h     The halfwidth of this node
		 */
		inline void set(int rid, const Eigen::Vector3d& c, double h)
		{
			/* set the values */
			this->room_id = rid;
			this->x = c(0); this->y = c(1); this->z = c(2);
			this->hw = h;
		};

		/**
		 * Retrieves room id of this object
		 */
		inline int get_room_id() const
		{ return this->room_id; };

		/**
		 * Retrieves the center position of this object
		 */
		inline Eigen::Vector3d get_center() const
		{
			/* return coordinates as vector */
			Eigen::Vector3d c(this->x, this->y, this->z);
			return c;
		};

		/**
		 * Retrieves the halfwidth of this object
		 */
		inline double get_halfwidth() const
		{ return this->hw; };
};

#endif
