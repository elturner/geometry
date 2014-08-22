#ifndef NODE_BOUNDARY_H
#define NODE_BOUNDARY_H

/**
 * @file node_boundary.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Classes used to define boundary nodes in octrees
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to formulate the set of boundary
 * nodes in a given octree.  Boundary nodes are nodes that are labeled
 * interior, but are adjacent to exterior nodes.  The "is_interior()"
 * function of octdata objects is used to determine if the nodes
 * are interior or exterior.
 */

#include <geometry/octree/octtopo.h>
#include <mesh/partition/node_set.h>
#include <string>
#include <vector>
#include <map>

/* the following classes are defined in this file */
class node_boundary_t;
class node_face_t;
class node_face_info_t;

/**
 * The node_boundary_t class can compute the subset of nodes
 * that represent the boundary.
 *
 * Boundary nodes are interior nodes that are adjacent to the
 * exterior sections of the tree.
 */
class node_boundary_t
{
	/* parameters */
	private:

		/**
		 * The boundary nodes and their topology
		 *
		 * The node set is populated by interior nodes
		 * that border exterior nodes.  These are defined
		 * as the boundary nodes of the octree.
		 *
		 * this topology represents the connections for 
		 * just the boundary nodes of the octree. it is a subset
		 * of the topology for the whole set of leaf nodes.
		 */
		octtopo::octtopo_t boundary;

		/**
		 * The boundary faces and their topology
		 *
		 * The set of faces is populated using the
		 * boundary nodes.  This mapping gives information
		 * about each node face, such as what adjoining faces
		 * it touches.
		 */
		std::multimap<node_face_t, node_face_info_t> faces;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Generates a set of boundary nodes from an octree topology
		 *
		 * This call will populate this boundary object.
		 *
		 * @param topo   The octree topology to use to populate this
		 *               object.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate(const octtopo::octtopo_t& topo);

		/**
		 * Clears all info stored in this boundary.
		 *
		 * This will clear any stored boundary information.
		 * To repopulate the boundary, call populate().
		 */
		inline void clear()
		{ this->boundary.clear(); };

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Determines the nodes of the tree adjacent to the
		 * specified node that are also boundary nodes.
		 *
		 * Will find the neighboring boundary nodes of the
		 * given node, which is assumed to be a boundary node.
		 *
		 * Any existing data in the neighs container will be
		 * cleared.
		 *
		 * If the given node is not a boundary node,
		 * then the neighs container will remain empty.
		 *
		 * @param node     The node to analyze
		 * @param neighs   Where to store the neighboring 
		 *                 boundary nodes.
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int get_boundary_neighbors(octnode_t* node,
				std::vector<octnode_t*>& neighs) const;

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Exports a Wavefront OBJ file representing the boundary
		 *
		 * Given an output file path, will export the faces
		 * of internal nodes (as measured by 
		 * octdata->is_interior()) that border with external
		 * nodes (or null nodes).  The output will be
		 * formatted as a wavefront .obj file.
		 *
		 * @param filename   The output file location
		 *
		 * @return     Returns zero on success, non-zero
		 *             on failure.
		 */
		int writeobj(const std::string& filename) const;
};

/**
 * This class represents a face of a node
 *
 * This class is expressed by the minimal amount of information
 * necessary to uniquely identify a particular face of a specific node.
 */
class node_face_t
{
	/* parameters */
	public:
		/* the originating node for this face */
		octnode_t* node;
		octtopo::CUBE_FACE f; /* the face of node this represents */

	/* functions */
	public:

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies information from the given node into this object
		 */
		inline node_face_t& operator = (const node_face_t& other)
		{
			this->node = other.node;
			this->f = other.f;
			return (*this);
		};

		/**
		 * Determines ordering of node faces.
		 *
		 * Ordering is necessary for insertion into sorted
		 * structures, such as sets or maps.
		 */
		inline bool operator < (const node_face_t& other) const
		{
			if(this->node < other.node)
				return true;
			if(this->node > other.node)
				return false;
			return (this->f < other.f);
		};

		/**
		 * Checks equality between node faces
		 */
		inline bool operator == (const node_face_t& other) const
		{
			return ((this->node == other.node) 
					&& (this->f == other.f));
		};

		/**
		 * Checks inequality between node faces
		 */
		inline bool operator != (const node_face_t& other) const
		{
			return ((this->node != other.node)
					|| (this->f != other.f));
		};
};

/**
 * This class represents the face of a node
 *
 * Node faces are necessary for computing various boundary properties
 */
class node_face_info_t
{
	/* parameters */
	public:

		/* center position of the face */
		double x;
		double y;
		double z;

		/* faces are always squares.  This
		 * value represents half the length
		 * of one side */
		double halfwidth;

		/* this value represents the list of faces that are 
		 * connected in some way to this face */
		std::set<node_face_t> neighbors;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Will populate values of face based on the given node.
		 *
		 * Given a node and a cube face, will populate all
		 * the parameters of this face.
		 *
		 * @param n     The node whose face this is
		 * @param ff    The face of the node that this represents
		 */
		void init(octnode_t* n, octtopo::CUBE_FACE ff);

		/**
		 * Initializes the face info based on the given face
		 *
		 * @param face  The face to use to initialize
		 */
		inline void init(const node_face_t& face)
		{ this->init(face.node, face.f); };

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies parameters from given node_face_t to this one
		 */
		inline node_face_info_t& operator = (
				const node_face_info_t& other)
		{
			/* copy values */
			this->x         = other.x;
			this->y         = other.y;
			this->z         = other.z;
			this->halfwidth = other.halfwidth;
			this->neighbors.clear();
			this->neighbors.insert(other.neighbors.begin(),
					other.neighbors.end());

			/* return the result */
			return (*this);
		};
};

#endif
