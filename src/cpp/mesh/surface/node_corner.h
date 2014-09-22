#ifndef NODE_CORNER_H
#define NODE_CORNER_H

/**
 * @file    node_corner.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Analyzes attributes of the corners of nodes in octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the node_corner::corner_t class, which describe 
 * the attributes of the octree at node corners.
 *
 * Note that the octree natively stores data at the center of nodes,
 * and in order to understand the value of the interpolated data at
 * the corners, you need this class.
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octtopo.h>
#include <mesh/surface/node_boundary.h>
#include <Eigen/Dense>
#include <iostream>
#include <set>

/**
 * This namespace contains all classes, structures, and functions used
 * to perform analysis on node corners.
 */
namespace node_corner
{
	/*-------------------*/
	/* Defined Constants */
	/*-------------------*/

	/**
	 * The following constant represents the number of corners
	 * per node.
	 *
	 * Since each node is a cube, it has eight corners.
	 */
	static const size_t NUM_CORNERS_PER_CUBE = 8;

	/**
	 * This indexing allows for a representation of each corner of 
	 * a given node.  The ordering of the indexing is meant to be
	 * consistent with the placement of child nodes of a parent
	 * node.
	 *
	 *
	 *    z
	 *    ^
	 *    .
	 *    .
 	 *    .    1 ________ 0                                         
 	 *    .    /|       /|                                           
 	 *    .  /  |     /  |                                          
 	 *   2 /_______ /    |                                          
 	 *    |     |  |3    |                                           
 	 *    |    5|__|_____|4                                         
 	 *    |    /   |    /                                          
 	 *    |  /     |  /                                          
 	 *    |/_______|/........................> x                 
 	 *   6          7                      
	 */
	inline Eigen::Vector3d get_corner_pos(const octnode_t* node,
						size_t corner_index)  
	{
		return (node->halfwidth 
				* relative_child_pos(corner_index))
				+ node->center;
	};

	/**
	 * The following value represents the number of edges attached
	 * at each corner of a cube.
	 *
	 * Based on geometry, this value is 3
	 */
	static const size_t NUM_EDGES_PER_CORNER = 3;

	/**
	 * This array represents the edges of a cube, by relating which
	 * corners share an edge with which other corners.
	 *
	 * This is a 2D array.  The first index represents a corner index.
	 * The row specified by this corner index will be 3 values long,
	 * representing the three neighboring indices for that corner.
	 *
	 * Also note that the ordering of these neighbors is such that,
	 * if you are looking from the outside in onto the cube, the
	 * edges are oriented counter-clockwise.
	 */
	static const size_t cube_edges[NUM_CORNERS_PER_CUBE][
					NUM_EDGES_PER_CORNER] = 
					{	{1, 4, 3},
						{0, 5, 2},
						{1, 6, 3},
						{0, 2, 7},
						{0, 7, 5},
						{1, 4, 6},
						{2, 5, 7},
						{3, 6, 4}	};

	/**
	 * This value indicates the number of corners per face of each node
	 *
	 * Since each face is a square, this value is four.  A square has
	 * four corners.  Ah ah ah.
	 */
	static const size_t NUM_CORNERS_PER_SQUARE = 4;

	/**
	 * This function is used to retrieve the corner indices of a face
	 *
	 * Given the face of a node, will retrieve the corners of that face.
	 * Note that the corners will be referenced in a counter-clockwise
	 * fashion going around the outside of the face.
	 *
	 * @param f   The face to analyze
	 * @param i   The index of the corner on the face.  Must be a value
	 *            between 0 and (NUM_CORNERS_PER_SQUARE-1).
	 *
	 * @return    The cube's corner index that was represented.
	 */
	inline size_t get_face_corner(octtopo::CUBE_FACE f, size_t i)
	{
		size_t fi;

		/* get value depending on face */
		fi = octtopo::NUM_FACES_PER_CUBE;
		switch(f)
		{
			case octtopo::FACE_ZMINUS:
				fi = 0;
				break;
			case octtopo::FACE_YMINUS:
				fi = 1;
				break;
			case octtopo::FACE_XMINUS:
				fi = 2;
				break;
			case octtopo::FACE_XPLUS:
				fi = 3;
				break;
			case octtopo::FACE_YPLUS:
				fi = 4;
				break;
			case octtopo::FACE_ZPLUS:
				fi = 5;
				break;
		}

		/* look up index for corner */
		static const size_t face_corners
				[octtopo::NUM_FACES_PER_CUBE]
				[NUM_CORNERS_PER_SQUARE] = 
				{	{7, 6, 5, 4},
					{3, 2, 6, 7},
					{2, 1, 5, 6},
					{0, 3, 7, 4},
					{1, 0, 4, 5},
					{0, 1, 2, 3}	};
		return face_corners[fi][i];
	}

	/*-------------------*/
	/* Node Corner Class */
	/*-------------------*/

	/**
	 * The node_corner_t class represents a corner in an octree
	 *
	 * A corner is the corner vertex of a node in the tree.  Since a 
	 * node is represented by a cube, each node is attached to eight 
	 * corners.  Since nodes can be next to each other, a corner can 
	 * be referenced by up to eight nodes.  The number of nodes that 
	 * reference a corner is dependent on the size of the nodes.
	 */
	class corner_t
	{
		/* parameters */
		private:

			/**
			 * A node that contains this corner
			 *
			 * Of all the nodes that contain this corner,
			 * this node should have the smallest halfwidth.
			 * If multiple nodes have the same minimum
			 * halfwidth, this should be the node that minimizes
			 * the value of 'index'.
			 *
			 * With the above conditions, the (node, index)
			 * pair should be unique for each corner.
			 */
			octnode_t* node;

			/**
			 * The index of this corner in node.
			 *
			 * Value must be between 0 and 
			 * (NUM_CORNERS_PER_CUBE-1)
			 */
			size_t index;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs corner with invalid parameters
			 */
			corner_t() : node(NULL), index(0)
			{};

			/**
			 * Constructs corner given another corner
			 *
			 * @param other   The other corner to copy
			 */
			corner_t(const corner_t& other)
				:	node(other.node), index(other.index)
			{};

			/**
			 * Constructs corner to be equivalent to the given
			 * values
			 *
			 * Note that the input values may not actually be
			 * the ones stored, since we want to store the
			 * node and index that gives a unique representation
			 * for each corner.
			 *
			 * @param tree  The originating octree
			 * @param node  Any node that contains this corner
			 * @param ind   The corner index in the given node
			 */
			corner_t(const octree_t& tree,
					octnode_t* node, size_t ind)
			{ this->set(tree, node, ind); };
				
			/*-----------*/
			/* modifiers */
			/*-----------*/

			/**
			 * Sets the value of this corner based on the
			 * input parameters
			 *
			 * Note that the input values may not actually
			 * be the ones stored, since we want to store the
			 * node and index that gives a unique representation
			 * for each corner.
			 *
			 * @param tree  The originating octree
			 * @param n     Any node that contains this corner
			 * @parma ind   The corner index in the given node
			 */
			void set(const octree_t& tree,
					octnode_t* n, size_t ind);

			/**
			 * Sets the value of this corner based on the
			 * input parameters
			 *
			 * Note that the input values may not actually
			 * be the ones stored, since we want to store the
			 * node and index that gives a unique representation
			 * for each corner.
			 *
			 * In order to set the value of this corner, all
			 * nodes that touch this corner must be identified.
			 * This call will save those nodes to the specified
			 * set.  Note that this structure will be cleared
			 * during this call.
			 *
			 * @param tree    The originating octree
			 * @param n       Any node that contains this corner
			 * @param ind     The corner index in the given node
			 * @param neighs  All neighboring nodes to this
			 *                corner
			 */
			void set(const octree_t& tree,
					octnode_t* n, size_t ind,
					std::set<octnode_t*> neighs);

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Checks if this corner is set to a valid value
			 *
			 * @return    Returns true if the corner is valid
			 */
			inline bool isvalid() const
			{ return (this->node != NULL); };

			/*----------*/
			/* geometry */
			/*----------*/

			/**
			 * Gets the global position of this corner
			 *
			 * @return   Returns position of corner
			 */
			inline Eigen::Vector3d get_position() const
			{ 
				/* this function is defined above */
				return get_corner_pos(this->node, 
						this->index);
			};

			/**
			 * Checks if the given node contains this corner
			 *
			 * Note that this will return true for parent
			 * nodes as well as leaf nodes.  If this corner
			 * is in the interior of the node, it will still
			 * return true.
			 *
			 * @param n   The node to analyze
			 *
			 * @return   Returns true iff node contains corner
			 */
			bool is_contained_in(const octnode_t* n) const;

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Will copy the given corner into this one
			 *
			 * @param other   The other corner to copy
			 *
			 * @return   Returns the modified corner
			 */
			inline corner_t& operator = (const corner_t& other)
			{
				/* copy the values */
				this->node = other.node;
				this->index = other.index;

				/* return the result */
				return (*this);
			};

			/**
			 * Checks if two corners are equal
			 *
			 * @param other    The other corner to compare to
			 *
			 * @return    Returns true iff the two corners are 
			 *            equal
			 */
			inline bool operator == (
						const corner_t& other) const
			{
				/* Because corners are constructed to
				 * only allow unique representations of
				 * each corner, then we can just compare the
				 * parameters.
				 *
				 * Note that there are many other parameter
				 * combinations that could be used to 
				 * express the given corner, but because
				 * we've limited how each corner is 
				 * expressed, we can take advantage of this
				 * speedup here.
				 */
				return ( (this->node == other.node) 
					&& (this->index == other.index) );
			};

			/**
			 * Performs a sorting comparison for the corners
			 *
			 * This comparison is useful when sorting corners,
			 * which is a necessary operation when inserting 
			 * them into a map or set.
			 *
			 * @param other   The other corner to compare to
			 *
			 * @return   Returns true iff this < other
			 */
			inline bool operator < (const corner_t& other) const
			{
				/* sort by node pointer, then by index */
				if(this->node < other.node)
					return true;
				if(this->node > other.node)
					return false;
				return (this->index < other.index);
			};

			/*-----------*/
			/* debugging */
			/*-----------*/

			/**
			 * Exports this corner to a Wavefront OBJ file
			 *
			 * This function is useful for debugging.  It
			 * will export this corner's position to
			 * the specified OBJ file stream.
			 *
			 * @param os    Where to write the OBJ geometry
			 */
			void writeobj(std::ostream& os) const;
	};
}
#endif
