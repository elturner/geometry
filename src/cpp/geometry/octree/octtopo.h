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
#include <vector>
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
		FACE_ZMINUS =  0,
		FACE_YMINUS =  1,
		FACE_XMINUS =  2,
		FACE_XPLUS  =  3,
		FACE_YPLUS  =  4,
		FACE_ZPLUS  =  5,
	};

	/**
	 * This function is a look-up table for opposing faces on the cube
	 */
	static inline CUBE_FACE get_opposing_face(CUBE_FACE f)
	{ return (NUM_FACES_PER_CUBE - 1 - f); };

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
			std::vector<octnode_t*> neighs[NUM_FACES_PER_CUBE];
			
		/* functions */
		public:

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
				this->neighs[f].push_back(n);
			};
	
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
					std::vector<octnode*>& ns) const
			{ 
				ns.insert(ns.end(), 
					this->neighs[f].begin(), 
					this->neighs[f].end());
			};
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
			 */
			void init(const octree_t& tree);

			// TODO
	};
}

#endif
