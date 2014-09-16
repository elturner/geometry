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
#include <utility>
#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <set>

/* the following classes are defined in this file */
class node_boundary_t;
class node_face_t;
class node_face_info_t;

/* the following typedefs are used for these classes */
typedef std::multimap<octnode_t*, node_face_t>  nodefacemap_t;
typedef std::map<node_face_t, node_face_info_t> facemap_t;
typedef std::set<node_face_t>                   faceset_t;

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
		 * A mapping from octnodes to faces
		 *
		 * This mapping represents, for each octnode,
		 * which faces abut that node.  The node can be
		 * either interior or exterior.  Since multiple
		 * faces can abut each node, this must be stored
		 * as a multimap.
		 */
		nodefacemap_t node_face_map;

		/**
		 * The boundary faces and their topology
		 *
		 * The set of faces is populated using the
		 * boundary nodes.  This mapping gives information
		 * about each node face, such as what adjoining faces
		 * it touches.
		 */
		facemap_t faces;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Default constructor for node boundary object.
		 */
		node_boundary_t();

		/**
		 * Frees all memory and resources.
		 */
		~node_boundary_t();

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
		{
			this->node_face_map.clear();
			this->faces.clear();
		};

		/**
		 * Returns an iterator to the beginning of the set of faces
		 */
		inline facemap_t::const_iterator begin() const
		{ return this->faces.begin(); };

		/**
		 * Returns an iterator to the end of the set of faces
		 */
		inline facemap_t::const_iterator end() const
		{ return this->faces.end(); };
		

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Retrieves a faces that neighbor a node
		 *
		 * Given an octnode, will retrieve all faces
		 * that either abut the given node or abut a neighbor
		 * of the given node.
		 * 
		 * Note that this casts a wide net if all you want the
		 * faces that actually touch the node, and should be
		 * treated as a superset of that.
		 *
		 * Any values that were stored in nfs before this call
		 * will remain in nfs.
		 *
		 * @param topo       The octree topology
		 * @param node       The node to analyze
		 * @param nfs        The neighboring face set to modify
		 *
		 * @return      Returns zero on success, non-zero on failure
		 */
		int get_nearby_faces(const octtopo::octtopo_t& topo,
			octnode_t* node, faceset_t& nfs) const;

		/**
		 * Given a face, will retrieve iterators to neighbor set
		 *
		 * Given a face, this function call will return a 
		 * pair of iterators <begin, end>, which will allow
		 * for iteration over the neighbors of the given face.
		 *
		 * @param f   The face to analyze
		 *
		 * @return    The start/end pair of iterators to f's neighs
		 */
		std::pair<faceset_t::const_iterator,
			faceset_t::const_iterator>
				get_neighbors(const node_face_t& f) const;

		/**
		 * Finds all faces that are abutting the given node
		 *
		 * Given an octnode in the tree, will find all computed
		 * node faces that are abutting the specified node.  This
		 * list is returned as an iterator pair, so that one can
		 * iterate from the first value to the last value in order
		 * to access all faces.
		 *
		 * @param node   The octnode to analyze
		 *
		 * @return   Returns start/end pair of iterators for
		 *           the retrieved faces.
		 */
		inline std::pair<nodefacemap_t::const_iterator,
			nodefacemap_t::const_iterator>
				find_node(octnode_t* node) const
		{ return this->node_face_map.equal_range(node); };

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
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate_face_linkages(const octtopo::octtopo_t& topo);
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
		/* the originating nodes for this face */
	
		/**
		 * This is the originating node for this face on
		 * the interior side.  This value can never be null
		 * for a valid face.  This node should always be
		 * interior, and should neighbor the exterior node.
		 */
		octnode_t* interior; 

		/**
		 * This is the originating node for this face on
		 * the exterior side.  This value may be null, which
		 * indicates the interior node is adjacent to the
		 * octree bounds.  If non-null, this node should be
		 * exterior and a neighbor to the interior node.
		 */
		octnode_t* exterior;

		/**
		 * The following parameter stores the direction on
		 * the cube of the interior node that faces the
		 * exterior node.
		 *
		 * This value can be regenerated using the topology,
		 * but it is useful to cache it here as well.
		 */
		octtopo::CUBE_FACE direction;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Default constructor establishes invalid face
		 */
		node_face_t()
			:	interior(NULL),
				exterior(NULL),
				direction(octtopo::FACE_ZMINUS)
		{};

		/**
		 * Constructor provides parameters for face
		 *
		 * @param in   The originating interior node for this face
		 * @param ex   The originating exterior node for this face
		 * @param dir  The direction from in to ex
		 */
		node_face_t(octnode_t* in, octnode_t* ex,
					octtopo::CUBE_FACE dir)
			: 	interior(in), 
				exterior(ex), 
				direction(dir)
		{};

		/**
		 * Constructor by copying another node face object
		 */
		node_face_t(const node_face_t& other)
			: 	interior(other.interior), 
				exterior(other.exterior), 
				direction(other.direction)
		{};

		/**
		 * Initializes this face
		 *
		 * @param in   The originating interior node for this face
		 * @param ex   The originating exterior node for this face
		 * @param dir  The direction from in to ex
		 */
		inline void init(octnode_t* in, octnode_t* ex,
				octtopo::CUBE_FACE dir)
		{
			this->interior  = in;
			this->exterior  = ex;
			this->direction = dir;
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

		/**
		 * Get the center position of the face
		 *
		 * Will determine the 3D center position of the face.
		 * Note that this position will be aligned with the grid,
		 * and does not take any isosurface computation into
		 * account.
		 * 
		 * @param p   Where to store the center position of face
		 */
		void get_center(Eigen::Vector3d& p) const;

		/**
		 * Computes the normal vector of this face
		 *
		 * Will compute the normal vector of the face,
		 * and store it in the provided argument
		 *
		 * @param n  Where to store the normal vector of this face
		 */
		inline void get_normal(Eigen::Vector3d& n) const
		{ octtopo::cube_face_normals(this->direction, n); };

		/**
		 * Get the halfwidth of the face
		 *
		 * The halfwidth represents half the length of one
		 * side of the square that represents the shape of
		 * the face.
		 *
		 * @return    Returns the halfwidth of that subface
		 */
		double get_halfwidth() const;

		/**
		 * Computes the surface area of this face
		 *
		 * The surface area is computed by first computing
		 * the halfwidth of the face.
		 *
		 * @return   Returns the area of this face
		 */
		double get_area() const;

		/*------------------------*/
		/* isosurface computation */
		/*------------------------*/

		/**
		 * Will compute the planrity estimate for this face
		 *
		 * Will use the originating octnodes of this
		 * face to compute the planarity estimate.
		 * A planarity estimate is a value between 0 and 1, where
		 * 1 is perfectly planar and 0 is not planar at all.
		 *
		 * @return    Returns the planarity estimate, in [0,1]
		 */
		 double get_planarity() const;

		/**
		 * Computes the position of the center of this face,
		 * assuming alignment with the local isosurface.
		 *
		 * Rather than using this face's built-in get_center()
		 * function, which aligns the center with the octree
		 * grid, this function will compute the center based
		 * on the underlying octdata in order to put this
		 * face on the isosurface of the probability distribution.
		 *
		 * @param p    Where to store the center position
		 */
		 void get_isosurface_pos(Eigen::Vector3d& p) const;

		/**
		 * Computes the variance of this face's position along the
		 * normal direction of this face.
		 *
		 * This function will
		 * determine what the positional variance is for
		 * this face's center point, along the normal of the face.
		 * This computation uses the octdata values in the
		 * originating octnodes that are used to represent this
		 * face.
		 *
		 * @return      Returns the variance in f's center position
		 *              along f's normal vector.
		 */
		 double get_pos_variance() const;

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies information from the given node into this object
		 */
		inline node_face_t& operator = (const node_face_t& other)
		{
			this->interior  = other.interior;
			this->exterior  = other.exterior;
			this->direction = other.direction;
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
			if(this->interior < other.interior)
				return true;
			if(this->interior > other.interior)
				return false;
			if(this->exterior < other.exterior)
				return true;
			if(this->exterior > other.exterior)
				return false;
			return (this->direction < other.direction);
		};

		/**
		 * Checks equality between node faces
		 */
		inline bool operator == (const node_face_t& other) const
		{
			return ((this->interior == other.interior) 
				&& (this->exterior == other.exterior)
				&& (this->direction == other.direction));
		};

		/**
		 * Checks inequality between node faces
		 */
		inline bool operator != (const node_face_t& other) const
		{
			return ((this->interior != other.interior)
				|| (this->exterior != other.exterior)
				|| (this->direction != other.direction));
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

		/**
		 * Writes face to OBJ file stream, colored by value
		 *
		 * Given a scalar value in the range [0,1], will generate
		 * geometry for this face colored to reflect the value.
		 *
		 * @param os  The output stream to write to
		 * @param v   The value to use
		 */
		void writeobj(std::ostream& os, double v) const;

		/**
		 * Writes face to a wavefront OBJ file stream, with color
		 *
		 * Given the RGB color, will export the geometry
		 * of this face to the given obj output stream.
		 *
		 * @param os   The output stream to write to
		 * @param r    The red component of color
		 * @param g    The green component of color
		 * @param b    The blue component of color
		 */
		void writeobj(std::ostream& os, int r, int g, int b) const;
};

/**
 * This class represents the topology of a face
 *
 * Node faces are necessary for computing various boundary properties.
 *
 * A node face can be adjacent to many other faces, which is represented
 * by this object.
 */
class node_face_info_t
{
	/* security */
	friend class node_boundary_t;

	/* parameters */
	private:

		/* this value represents the list of faces that are 
		 * connected in some way to this face */
		faceset_t neighbors;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Default constructor
		 */
		node_face_info_t()
		{ this->clear(); };

		/**
		 * Constructs this info from the given info object
		 */
		node_face_info_t(const node_face_info_t& other)
			: neighbors(other.neighbors.begin(), 
					other.neighbors.end())
		{};

		/**
		 * Clears all information from this structure.
		 */
		inline void clear()
		{ this->neighbors.clear(); };

		/*-----------*/
		/* accessors */
		/*-----------*/
		
		/**
		 * Returns the beginning iterator to the neighbors set
		 *
		 * @return   The begin iterator to this->neighbors
		 */
		inline faceset_t::const_iterator begin() const
		{ return this->neighbors.begin(); };

		/**
		 * Returns the end iterator to the neighbors set
		 *
		 * @return   The end iterator to this->neighbors
		 */
		inline faceset_t::const_iterator end() const
		{ return this->neighbors.end(); };

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
			this->neighbors.clear();
			this->neighbors.insert(other.neighbors.begin(),
					other.neighbors.end());

			/* return the result */
			return (*this);
		};
};

#endif
