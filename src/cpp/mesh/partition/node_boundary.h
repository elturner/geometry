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
#include <Eigen/Dense>
#include <ostream>
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
		std::map<node_face_t, node_face_info_t> faces;

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

		/**
		 * Exports a Wavefront OBJ file representing cliques in
		 * boundary graph
		 *
		 * Given an output file paht, will epxort the discovered
		 * cliques in the graph representation of the boundary
		 * faces each as a triangle.
		 *
		 * @param filename   The output file location
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writeobj_cliques(const std::string& filename) const;

	/* helper functions */
	private:

		/**
		 * Populates the boundary struct inside this object
		 *
		 * This function is called within the populate()
		 * function.
		 *
		 * @param topo   The topology to use
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate_boundary(const octtopo::octtopo_t& topo);

		/**
		 * Populates the faces struct inside this object
		 *
		 * This function is called within the populate()
		 * function.  populate_boundary() must be called first.
		 *
		 * @param topo   The original, full topology
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate_faces(const octtopo::octtopo_t& topo);

		/**
		 * Populates the linkages between faces
		 *
		 * Will compute which faces are neighbors to which other
		 * faces. populate_faces() must have already been called
		 * before calling this function.
		 *
		 * @param topo   The original, full topology
		 * @param node_face_map     A mapping from nodes to the 
		 *                          faces that make
		 *                          contact with those nodes.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate_face_linkages(const octtopo::octtopo_t& topo,
				const std::multimap<octnode_t*,
				node_face_t>& node_face_map);
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

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default constructor establishes invalid face
		 */
		node_face_t()
		{
			this->node = NULL;
			this->f = octtopo::FACE_ZMINUS;
		};

		/**
		 * Constructor provides parameters for face
		 *
		 * @param n   The originating node for this face
		 * @param ff  The side of the node that holds this face
		 */
		node_face_t(octnode_t* n, octtopo::CUBE_FACE ff)
		{
			this->node = n;
			this->f = ff;
		};

		/*----------*/
		/* geometry */
		/*----------*/

		/**
		 * Checks if this face shares an edge with another face
		 *
		 * To share an edge means that the faces are touching
		 * along a line defined by the intersection of the boundary
		 * of both faces.  This function call will check if such
		 * an intersection exists and if the faces fall along it.
		 *
		 * @param other   The other face to compare to
		 *
		 * @return        Returns true iff the faces share an edge
		 */
		bool shares_edge_with(const node_face_t& other) const;

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
 * Node faces are necessary for computing various boundary properties.
 *
 * Note that a face can be represented as a set of subfaces.  This is
 * because the entirety of the area of a face may not be touching the
 * boundary of the model, in which case we need to keep track of which
 * portions of the face are part of the boundary.
 */
class node_face_info_t
{
	/* security */
	friend class node_boundary_t;

	/* parameters */
	private:

		/* the face being represented.  This is stored as the
		 * originating node for this face and the side of the node
		 * that the face resides on.
		 *
		 * The originating node is always interior
		 */
		node_face_t face;

		/* the neighboring nodes of the originating node of this
		 * face.  Given the originating node and the neighboring
		 * node, one can reconstruct the boundary surface between
		 * them. 
		 *
		 * These should all be exterior nodes.  They are allowed
		 * to be null.
		 */
		std::vector<octnode_t*> exterior_nodes;

		/* this value represents the list of faces that are 
		 * connected in some way to this face */
		std::set<node_face_t> neighbors;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Clears all information from this structure.
		 */
		inline void clear()
		{
			this->face.node = NULL;
			this->exterior_nodes.clear();
			this->neighbors.clear();
		};

		/**
		 * Initializes this structure for a given face
		 *
		 * @param f   The face to initialize to
		 */
		inline void init(node_face_t& f)
		{
			/* reset the values of this structure */
			this->face = f;
			this->exterior_nodes.clear();
			this->neighbors.clear();
		};

		/**
		 * Will populate values of a subface based on given 
		 * exterior node.
		 *
		 * Given an exterior node and a cube face, will add a
		 * subface to this face info struct.
		 *
		 * @param n     The opposing node that shares this face
		 */
		inline void add(octnode_t* n)
		{ this->exterior_nodes.push_back(n); };

		/*----------*/
		/* analysis */
		/*----------*/

		/**
		 * Retrieve the number of subfaces represented
		 *
		 * @return   Returns the number of subfaces represented
		 *           in this info structure.
		 */
		inline size_t num_subfaces() const
		{ return this->exterior_nodes.size(); };

		/**
		 * Get the center position of the given subface
		 *
		 * Given the index of a subface, will determine the
		 * 3D center position of that subface.
		 * 
		 * @param i   The index of the subface to analyze
		 * @param p   Where to store the center position of subface
		 */
		void get_subface_center(size_t i, Eigen::Vector3d& p) const;

		/**
		 * Get the halfwidth of the given subface
		 *
		 * Given the index of a subface, will determine the
		 * halfwidth of that subface.
		 *
		 * The halfwidth represents half the length of one
		 * side of the square that represents the shape of
		 * the subface.
		 *
		 * @param i   The index of the subface to analyze
		 *
		 * @return    Returns the halfwidth of that subface
		 */
		double get_subface_halfwidth(size_t i) const;

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
			this->face = other.face;
			this->exterior_nodes.clear();
			this->exterior_nodes.insert(
				this->exterior_nodes.begin(),
				other.exterior_nodes.begin(),
				other.exterior_nodes.end());
			this->neighbors.clear();
			this->neighbors.insert(other.neighbors.begin(),
					other.neighbors.end());

			/* return the result */
			return (*this);
		};

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes face to a wavefront OBJ file stream
		 *
		 * @param os   The output stream to write to
		 */
		void writeobj(std::ostream& os) const;
};

#endif
