#ifndef NODE_SET_H
#define NODE_SET_H

/**
 * @file   node_set.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Represents a subset of nodes from an octree
 *
 * @section DESCRIPTION
 *
 * This file defines the node_set_t class, which is used to
 * represent a subset of nodes from an octree, along with
 * relavant functions that analyze or modify these nodes.
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <string>
#include <set>

/**
 * The node_set_t class is used to represent a subset of nodes from a tree
 */
class node_set_t
{
	/* parameters */
	private:

		/**
		 * The set of nodes being represented
		 *
		 * This set contains a list of node pointers, which
		 * are defined in an octree.  Note that the octree is
		 * responsible for all memory management of nodes, NOT
		 * this class.
		 */
		std::set<octnode_t*> nodes;

	/* functions */
	public:

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Clears all nodes from this set
		 *
		 * After this call, this object will
		 * represent the empty set of nodes.
		 */
		inline void clear()
		{ this->nodes.clear(); };

		/**
		 * Checks if this set contains a node
		 *
		 * @param n   The node to check
		 *
		 * @return    Returns true iff n is in this set
		 */
		inline bool contains(octnode_t* n) const
		{ return (this->nodes.count(n) > 0); };

		/**
		 * Returns the size of this container
		 *
		 * @return   Returns the number of elements in this set
		 */
		inline size_t size() const
		{ return (this->nodes.size()); };

		/**
		 * Adds a node to this set
		 *
		 * @param n   The node to add to set
		 */
		inline void add(octnode_t* n)
		{ this->nodes.insert(n); };

		/**
		 * Removes a node from this set
		 *
		 * @param n   The node to remove from set
		 */
		inline void remove(octnode_t* n)
		{ this->nodes.erase(n); };

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Sets the contents of this node set to be that given
		 *
		 * Given node set other, will copy the contents of
		 * other into this node set, discarding any values contained
		 * in this set before this call.
		 *
		 * @param other    The set to copy
		 *
		 * @return         Returns reference to modified set
		 */
		inline node_set_t& operator = (const node_set_t& other)
		{
			/* copy contents */
			this->clear();
			this->nodes.insert(other.nodes.begin(),
			                   other.nodes.end());
			
			/* return reference to the modified set */
			return *this;
		};

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes objects described by this set to mesh file
		 *
		 * Will export the described nodes to the specified
		 * Wavefront OBJ file.
		 *
		 * @param filename  Where to write the file
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int writeobj(const std::string& filename) const;
};

#endif
