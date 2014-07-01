#ifndef OCTTOPO_H
#define OCTTOPO_H

/**
 * @file octtopo.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The octtopo_t class is used for computing octree topology
 *
 * @section DESCRIPTION
 *
 * This file contains the octtopo_t class, which is used to provide
 * additional representations of octree and octnode topology.  Its
 * main purpose is to allow for relative neighbor linkages between
 * adjacent nodes.
 *
 * While this class is not part of the octree structure directly, it
 * can be provided with an octree to be initialized, and used to augment
 * an existing tree structure information.
 */

#include "octree.h"
#include "octnode.h"
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

/**
 * This namespace contains all classes and definitions for node topology
 */
namespace octtopo
{
	/* the following classes are defined in this file */
	class octneighbors_t;
	class octtopo_t;

	/*-------------------------------------------------------*/
	/*------------- topology helper structures --------------*/
	/*-------------------------------------------------------*/

	/* basic geometry constants */
	static const size_t NUM_FACES_PER_CUBE = 6;

	/**
	 * This enumurates the faces of an axis-aligned cube
	 *
	 * Each octnode can have neighbors on all six faces.  The
	 * relative positions of the nodes are defined in the octnode_t
	 * class.
	 */
	enum CUBE_FACE
	{
		FACE_ZMINUS,
		FACE_YMINUS,
		FACE_XMINUS,
		FACE_XPLUS,
		FACE_YPLUS,
		FACE_ZPLUS
	};

	/**
	 * This is a list of all faces on a cube, for easy iteration
	 */
	static const CUBE_FACE all_cube_faces[NUM_FACES_PER_CUBE]
		= { FACE_ZMINUS, FACE_YMINUS, FACE_XMINUS,
		    FACE_XPLUS,  FACE_YPLUS,  FACE_ZPLUS };

	/**
	 * This function is a look-up table for opposing faces on the cube
	 */
	static inline CUBE_FACE get_opposing_face(CUBE_FACE f)
	{
		/* return the opposing face */
		switch(f)
		{
			case FACE_ZMINUS:	return FACE_ZPLUS;
			case FACE_YMINUS:	return FACE_YPLUS;
			case FACE_XMINUS:	return FACE_XPLUS;
			case FACE_XPLUS:	return FACE_XMINUS;
			case FACE_YPLUS:	return FACE_YMINUS;
			case FACE_ZPLUS:	return FACE_ZMINUS;
		}

		/* will never get here */
		return f;
	};

	/*-----------------------------------------------------*/
	/*-------------- class declarations -------------------*/
	/*-----------------------------------------------------*/
	
	/**
	 * The octneighbors_t class represents neighbors of a single octnode
	 */
	class octneighbors_t
	{
		/* security */
		friend class octtopo_t;
	
		/* parameters */
		private:

			/**
			 * Stores the neighboring nodes to this node
			 */
			std::set<octnode_t*> neighs[NUM_FACES_PER_CUBE];
			
		/* functions */
		public:

			/*-----------*/
			/* modifiers */
			/*-----------*/

			/**
			 * Clears all neighbor info
			 */
			void clear();

			/**
			 * Adds a neighbor to this structure
			 *
			 * Given a neighbor and a face direction, will
			 * store the neighbor appropriately in this object.
			 *
			 * @param n   The neighboring node to store
			 * @param f   The face on which n neighbors
			 */
			inline void add(octnode_t* n, CUBE_FACE f)
			{
				/* ignore if pointer is null */
				if(n != NULL)
					this->neighs[f].insert(n);
			};

			/**
			 * Adds all neighbors to this structure
			 *
			 * Given a list of neighbors and a face direction,
			 * will store the neighbors appropriately in this
			 * object.
			 *
			 * @param ns  The neighboring nodes to store
			 * @param f   The face on which ns neighbor
			 */
			void add_all(const std::vector<octnode_t*>& ns,
			             CUBE_FACE f);

			/**
			 * Removes neighbor from this structure
			 *
			 * Given a neighbor node and a face, will
			 * remove this neighbor from this face.
			 *
			 * @param n   The node to remove
			 * @param f   The face on which n neighbors
			 *
			 * @return    Returns true if it was removed,
			 *            false if never there to begin with
			 */
			inline bool remove(octnode_t* n, CUBE_FACE f)
			{
				return (0 < this->neighs[f].erase(n));
			};

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Gets the neighbors for a particular face
			 *
			 * Will store all neighboring nodes for a face
			 * into the given list.
			 *
			 * Any elements in this list already will still
			 * exist after this call.
			 *
			 * @param f   The face to analyze
			 * @param ns  The vector to modify
			 */
			inline void get(CUBE_FACE f,
					std::vector<octnode_t*>& ns) const
			{ 
				ns.insert(ns.end(), 
					this->neighs[f].begin(), 
					this->neighs[f].end());
			};

			/**
			 * Gets all 'singleton' neighbors
			 *
			 * A singleton neighbor is one where there is
			 * exactly one neighbor on a given face.  Any
			 * non-leaf node should have all its neighbors
			 * as singletons (assuming the neighbors exist).
			 *
			 * Here, if a neighbor is non-singleton (either
			 * because there are multiple neighbors on a face
			 * or because there are no neighbors), then the
			 * face index is populated with NULL.
			 *
			 * @param ns    The array of neighbors to populate,
			 *              indexed by face.
			 */
			void get_singletons(
				octnode_t* ns[NUM_FACES_PER_CUBE]) const;

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Copies values from given neighbor object to this
			 *
			 * Will destroy any existing info in this object
			 * and replace it with a copy of the info in the
			 * given octneighbors_t object.
			 *
			 * @param other   The object to copy
			 *
			 * @return        A reference to the modified object
			 */
			octneighbors_t& operator = 
					(const octneighbors_t& other);
	};

	/**
	 * The octtopo_t class represents the octnodes' neighbor topology
	 */
	class octtopo_t
	{
		/* parameters */
		private:
	
			/**
			 * A mapping between octnodes and their neighbors
			 */
			std::map<octnode_t*, octneighbors_t> neighs;

		/* functions */
		public:

			/**
			 * Initializes this structure from the given octree
			 *
			 * Based on the given tree, will initialize all
			 * nodes' neighbor sets.
			 *
			 * @param tree   The tree to use to initialize
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int init(const octree_t& tree);


			/*-----------*/
			/* debugging */
			/*-----------*/

			/**
			 * Writes boundary faces to Wavefront OBJ format
			 *
			 * Given an output file path, will export the faces
			 * of internal nodes (as measured by 
			 * octdata->is_interior()) that border with external
			 * nodes (or null nodes).  The output will be
			 * formatted as a wavefront .obj file.
			 *
			 * @param os   The output file location
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int writeobj(const std::string& filename) const;
			
			/**
			 * Checks the invarients of the stored data
			 *
			 * Will iterate through all properties of the
			 * stored data, checking if any elements are
			 * inconsistent.  If an error is encountered,
			 * the details will be printed to cerr and 
			 * a unique value will be returned.
			 *
			 * @return    Returns zero on success, non-zero on
			 *            failure.
			 */
			int verify() const;

		/* helper functions */
		private:

			/**
			 * Recursively initializes neighbors of children
			 *
			 * Given an octnode with valid neighbors in the
			 * this->neighs map, will generate neighbor mappings
			 * for the children of this node recursively.
			 *
			 * @param node   The node to analyze
			 */
			void init_children(octnode_t* node);

			/**
			 * Retrieves nodes at child level given parent node
			 *
			 * Given a node, will retrieve pointers to its
			 * children.  If the parent node is a leaf, then
			 * all child pointers will refer to the original
			 * parent node.  If the parent node is null, then
			 * the children will also be null.
			 *
			 * @param cs   Where to store the child pointers
			 * @param p    The parent node
			 */
			static void get_children_of(
				octnode_t* cs[CHILDREN_PER_NODE], 
				octnode_t* p);

			/**
			 * Removes all elements that do not represent leafs
			 *
			 * Will remove all neighbor objects from this
			 * object's map that are not associated with leaf
			 * nodes in the given tree.  This saves space,
			 * since often we only care about the neighbor
			 * relations on leaf nodes.
			 *
			 * @return   Returns zero on success, non-zero on
			 *           failure.
			 */
			int remove_nonleafs();
	
			/*-----------*/
			/* debugging */
			/*-----------*/
	
			/**
			 * Writes a single node face to the OBJ stream
			 *
			 * Given an output stream to a wavefront OBJ file,
			 * will export a single voxel face to that stream.
			 *
			 * The face will be written counter-clockwise into
			 * the node.
			 *
			 * @param os       The output stream to write to
			 * @param n        The node whose face should be 
			 *                 written
			 * @param f        The face to write
			 * @param inside   Specifies whether to orient the
			 *                 face counter-clockwise into
			 *                 or out of the node.
			 */
			void writeobjface(std::ostream& os, octnode_t* n,
			                  CUBE_FACE f, bool inside) const;
	};
}

#endif
