#ifndef NODE_CORNER_MAP_H
#define NODE_CORNER_MAP_H

/**
 * @file    node_corner_map.h
 * @author  Eic Turner <elturner@eecs.berkeley.edu>
 * @brief   Defines the node_corner::corner_map_t class
 *
 * @section DESCRIPTION
 *
 * This file contains the classes used to map the properties
 * of node corners in a model.  It adds to the node_corner
 * namespace.
 */

#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/node_corner.h>
#include <Eigen/Dense>
#include <set>
#include <map>

/**
 * This namespace contains all classes, structures, and functions used
 * to perform analysis on node corners.
 */
namespace node_corner
{
	/* the following classes are defined in this file */
	class corner_info_t;
	class corner_map_t;

	/* the following type definitions are useful for these classes */
	typedef std::map<corner_t, corner_info_t> ccmap_t;
	typedef std::set<corner_t>                cornerset_t;

	/**
	 * The corner_info_t class is used to store all faces
	 * in the model that touch this corner
	 */
	class corner_info_t
	{
		/* security */
		friend class corner_map_t;

		/* parameters */
		private:

			/**
			 * The set of nodes that touch this corner
			 */
			std::set<octnode_t*> nodes;

			/**
			 * The set of faces that touch this corner
			 */
			faceset_t faces;

			/**
			 * The set of edges (represented by another
			 * corner) that touch this corner.
			 *
			 * This member is only populated if populate_edges()
			 * is called on the corner_map_t.
			 */
			cornerset_t edges;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs empty object
			 */
			corner_info_t()
			{ /* don't need to do anything */ };

			/**
			 * Constructs object with given nodes
			 *
			 * Will Construct a corner_info_t object
			 * that contains the specified nodes
			 */
			corner_info_t(const std::set<octnode_t*>& ns)
				:	nodes(ns)
			{};

			/*-----------*/
			/* modifiers */
			/*-----------*/

			/**
			 * Adds neighboring nodes to this structure
			 *
			 * @param ns   The set of nodes to add
			 */
			inline void add(const std::set<octnode_t*>& ns)
			{ this->nodes.insert(ns.begin(), ns.end()); };

			/**
			 * Adds a node to this structure
			 *
			 * @param n   The node to add
			 */
			inline void add(octnode_t* n)
			{ this->nodes.insert(n); };

			/**
			 * Adds a set of faces to this structure
			 *
			 * @param fs   The set of faces to add
			 */
			void add(const faceset_t& fs);

			/**
			 * Adds a face to this structure
			 *
			 * @param f    The face to add
			 */
			void add(const node_face_t& f);

			/**
			 * Adds an edge (as a corner) to this structure
			 *
			 * @param c    The corner to add
			 */
			inline void add(const corner_t& c)
			{ this->edges.insert(c); };

			/*-----------*/
			/* accessors */
			/*-----------*/
			
			/**
			 * Retrieves the number of nodes in this structure
			 *
			 * @return   Number of nodes stored here
			 */
			inline size_t num_nodes() const
			{ return this->nodes.size(); };

			/**
			 * Check if a given node is in this structure
			 *
			 * @param n   The node to analyze
			 *
			 * @return    Returns true if n is in this structure
			 */
			inline bool contains(octnode_t* n) const
			{ return this->nodes.count(n) > 0; };

			/**
			 * Retrieves the number of faces in this structure
			 *
			 * @return   Number of faces stored here
			 */
			inline size_t num_faces() const
			{ return this->faces.size(); };

			/**
			 * Check if a given face is in this structure
			 *
			 * @param f   The face to analyze
			 *
			 * @return    Returns true if f is in this structure
			 */
			inline bool contains(const node_face_t& f) const
			{ return this->faces.count(f) > 0; };

			/**
			 * Beginning iterator for the face set
			 *
			 * @return   The begin iterator for the set of faces
			 */
			inline faceset_t::const_iterator begin_faces() const
			{ return this->faces.begin(); };

			/**
			 * End iterator fot the face set
			 *
			 * @return   The end iterator for the set of faces
			 */
			inline faceset_t::const_iterator end_faces() const
			{ return this->faces.end(); };
			
			/**
			 * Retrieves the number of edges in this structure
			 *
			 * @return   Number of edges stored here
			 */
			inline size_t num_edges() const
			{ return this->edges.size(); };

			/**
			 * Check if a given edge is in this structure
			 *
			 * The edge is represented by this corner and
			 * the argument corner.
			 *
			 * @param c   The other corner of the edge
			 *
			 * @return    REturns true iff c is in structure
			 */
			inline bool contains(const corner_t& c) const
			{ return this->edges.count(c) > 0; };

			/**
			 * Beginning iterator for the edge set
			 *
			 * @return   The begin iterator for the set
			 *           of corners that, along with this
			 *           corner, define edges.
			 */
			inline cornerset_t::const_iterator 
						begin_edges() const
			{ return this->edges.begin(); };

			/**
			 * Ending iterator for the edge set
			 *
			 * @return  The end iterator for set of edges
			 */
			inline cornerset_t::const_iterator end_edges() const
			{ return this->edges.end(); };

	};

	/**
	 * The corner_map_t contains a mapping between all corners
	 * of interest and their populated corner_info_t objects
	 */
	class corner_map_t
	{
		/* parameters */
		private:

			/**
			 * This represents a mapping between corners
			 * and their information structs
			 */
			ccmap_t corners;

		/* functions */
		public:

			/*-----------*/
			/* modifiers */
			/*-----------*/

			/**
			 * Clears all info from this map
			 */
			inline void clear()
			{ this->corners.clear(); };

			/** 
			 * Adds the given node to this map
			 *
			 * Will add all corners of the given node,
			 * and associate this node with each of those
			 * corners.
			 *
			 * @param tree  The originating tree for n
			 * @param n     The node to add to this map
			 */
			void add(const octree_t& tree, octnode_t* n);

			/**
			 * Adds all leaf nodes to this map
			 *
			 * Will recursively search through the given
			 * tree, and add ALL leaf nodes to this map.
			 *
			 * @param tree  The tree to add
			 */
			void add_all(const octree_t& tree);

			/**
			 * Adds the given face to this map
			 *
			 * Will add all corners of the given face,
			 * and associate this face with each of those
			 * corners.
			 *
			 * @param tree  The originating tree for f
			 * @param f     The face to add to this map
			 */
			void add(const octree_t& tree,
					const node_face_t& f);

			/**
			 * Adds the given face to this map
			 *
			 * Will add all corners of the given face,
			 * as well as corners of neighboring faces
			 * that intersect with this face, to this structure.
			 * It will also associate this face with each
			 * corner.
			 *
			 * @param tree   The originating tree for f
			 * @param f      The face to add to this map
			 * @param neighs The neighbors of this face
			 */
			void add(const octree_t& tree,
					const node_face_t& f,
					const node_face_info_t& neighs);

			/**
			 * Adds all faces in the given boundary struct
			 *
			 * Will iterate through the node boundary
			 * structure provided, adding all stored faces
			 * to this mapping.
			 *
			 * @param tree       The originating tree
			 * @param boundary   The boundary faces for tree
			 */
			void add(const octree_t& tree,
					const node_boundary_t& boundary);

			/*------------*/
			/* processing */
			/*------------*/

			/**
			 * Populates all edges in this map
			 *
			 * This should be called only after all faces have
			 * been added to the structure, since it uses 
			 * existing faces and corners in the computation
			 *
			 * This call will populate the corner_info_t.edges
			 * member for each element in the map.
			 *
			 * @param tree   The originating octree
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.
			 */
			int populate_edges(const octree_t& tree);

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Retrieves the number of corners in this map
			 *
			 * @return   Number of stored corners
			 */
			inline size_t num_corners() const
			{ return this->corners.size(); };

			/**
			 * Returns the beginning iterator to the corners map
			 *
			 * @return  The start of the corners map iterator
			 */
			inline ccmap_t::const_iterator begin() const
			{ return this->corners.begin(); };

			/**
			 * Returns the end iterator to the corners map
			 *
			 * @return  The end of the corners map iterator
			 */
			inline ccmap_t::const_iterator end() const
			{ return this->corners.end(); };

			/**
			 * Gets the iterators for the neighboring nodes
			 * of the given corner.
			 *
			 * Given a corner that is stored in this map,
			 * will perform the look-up to find all
			 * octnodes that touch this corner.  The return
			 * value is the begin/end iterator pair to
			 * cycle through these nodes.
			 *
			 * @param c   The corner to analyze
			 *
			 * @return    Returns the iterators to the
			 *            neighboring nodes for this corner
			 */
			std::pair<std::set<octnode_t*>::const_iterator,
				std::set<octnode_t*>::const_iterator>
					get_nodes_for(const corner_t& c) 
						const;

			/**
			 * Gets the iterators for the neighboring faces
			 * of the given corner.
			 *
			 * Given a corner that is stored in this map,
			 * will perform the look-up to find all
			 * node_face_t that touch this corner.  The
			 * return value is the begin/end iterator pair
			 * to cycle through these faces.
			 *
			 * @param c    The corner to analyze
			 *
			 * @return     Returns the iterators to the
			 *             neighboring faces of this corner
			 */
			std::pair<faceset_t::const_iterator,
				faceset_t::const_iterator>
					get_faces_for(const corner_t& c)
						const;

			/**
			 * Gets the iterators for the neighboring edges
			 * of the given corner.
			 *
			 * Given a corner that is stored in this map,
			 * will perform the look-up to find all
			 * the corner_t objects that share an edge with
			 * the given corner.  The return value is the
			 * begin/end iterator pair to cycle through these
			 * edges.
			 *
			 * @param c   The corner to analyze
			 *
			 * @return    Returns iterators to neighboring
			 *            edges of this corner.
			 */
			std::pair<cornerset_t::const_iterator,
				cornerset_t::const_iterator>
					get_edges_for(const corner_t& c)
						const;

		/* helper functions */
		private:

			/**
			 * Recursively adds all leaf nodes under the
			 * given node to this mapping.
			 *
			 * Will search through all of the given node's
			 * subnodes, adding all leaf nodes to the map.
			 *
			 * @param tree  The originating tree
			 * @param node  The node to add recursively
			 */
			void add_all(const octree_t& tree, octnode_t* node);
	};
}

#endif
