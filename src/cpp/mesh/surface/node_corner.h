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
#include <cmath>

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
	 * <pre>
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
	 * </pre>
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
			 * The x-coordinate of this corner's index
			 *
			 * In order to unique represent a corner
			 * value, we store its discretized position
			 * in units of half-resolutions of the tree.
			 */
			int x_ind;

			/**
			 * The y-coordinate of this corner's index
			 *
			 * In order to unique represent a corner
			 * value, we store its discretized position
			 * in units of half-resolutions of the tree.
			 */
			int y_ind;
			
			/**
			 * The z-coordinate of this corner's index
			 *
			 * In order to unique represent a corner
			 * value, we store its discretized position
			 * in units of half-resolutions of the tree.
			 */
			int z_ind;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs corner with invalid parameters
			 */
			corner_t() : x_ind(0), y_ind(0), z_ind(0)
			{};

			/**
			 * Constructs corner given another corner
			 *
			 * @param other   The other corner to copy
			 */
			corner_t(const corner_t& other)
				:	x_ind(other.x_ind), 
					y_ind(other.y_ind),
					z_ind(other.z_ind)
			{};

			/**
			 * Constructs corner to be equivalent to the given
			 * values
			 *
			 * Will set this corner to be equivalent to the
			 * 'ind' corner of the given node, where 'ind'
			 * is between 0 and (NUM_CORNERS_PER_CUBE-1).
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
			 * a unique representation for each corner.
			 *
			 * @param tree  The originating octree
			 * @param n     Any node that contains this corner
			 * @parma ind   The corner index in the given node
			 */
			inline void set(const octree_t& tree,
					octnode_t* n, size_t ind)
			{
				double res;

				/* get corner position in world coords */
				Eigen::Vector3d p = get_corner_pos(n, ind);

				/* get tree properties */
				p -= tree.get_root()->center;
				res = tree.get_resolution() * 0.5;

				/* get discretized coordinates */
				this->x_ind = (int) floor(p(0) / res);
				this->y_ind = (int) floor(p(1) / res);
				this->z_ind = (int) floor(p(2) / res);
			};

			/**
			 * Sets the value of this corner based on
			 * the input parameters
			 *
			 * @param tree   The originating tree
			 * @param f      The node face that contains
			 *               this corner
			 * @param ind    The index of the corner on
			 *               this face, in range 0 to
			 *               (NUM_CORNERS_PER_SQUARE-1)
			 */
			inline void set(const octree_t& tree,
					const node_face_t& f,
					size_t ind)
			{
				size_t node_corner_ind;
				octnode_t* node;
				octtopo::CUBE_FACE cf;

				/* determine which of the face's
				 * nodes are smaller */
				node = f.interior;
				cf = f.direction;
				if(f.exterior != NULL 
						&& f.exterior->halfwidth
						< f.interior->halfwidth)
				{
					node = f.exterior;
					cf = octtopo::get_opposing_face(
							f.direction);
				}

				/* get the node's corner index */
				node_corner_ind = get_face_corner(cf, ind);

				/* use the interior node of the face
				 * to look up the corner */
				this->set(tree, node, node_corner_ind);
			};

			/*----------*/
			/* geometry */
			/*----------*/

			/**
			 * Gets the global position of this corner
			 *
			 * @param tree   The originating octree for this
			 *               corner
			 * @param pos    Where to store the position vector
			 */
			inline void get_position(const octree_t& tree,
						Eigen::Vector3d& pos) const
			{ 
				double res;

				/* get the tree resolution */
				res = tree.get_resolution() * 0.5;

				/* set corner position based on tree */
				pos = tree.get_root()->center;
				pos(0) += this->x_ind * res;
				pos(1) += this->y_ind * res;
				pos(2) += this->z_ind * res;
			};

			/**
			 * Checks if this face occurs within the bounding
			 * box given.
			 *
			 * Given a bounding box of corners in 3D space,
			 * will check if this corner falls within the
			 * bounding box.
			 *
			 * @param min_c   The minimum corner of the bounds
			 * @param max_c   The maximum corner of the bounds
			 *
			 * @return    Returns true if this corner is within
			 *            the given bounds.
			 */
			bool within_bounds(const corner_t& min_c,
					const corner_t& max_c) const;

			/**
			 * Will update the bounds specified by the arguments
			 * to include this corner.
			 *
			 * Given some bounds, will modify the min and max
			 * components so that the bounds include the value
			 * of this corner
			 *
			 * @param min_c   The minimum corner of the bounds
			 * @param max_c   The maximum corner of the bounds
			 */
			void update_bounds(corner_t& min_c, 
					corner_t& max_c) const;

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
				this->x_ind = other.x_ind;
				this->y_ind = other.y_ind;
				this->z_ind = other.z_ind;

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
				/* the corners are equal if their
				 * discretized positions are equal */
				return ( (this->x_ind == other.x_ind)
					|| (this->y_ind == other.y_ind)
					|| (this->z_ind == other.z_ind) );
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
				/* sort by each coordiante*/
				if(this->x_ind < other.x_ind)
					return true;
				if(this->x_ind > other.x_ind)
					return false;
				if(this->y_ind < other.y_ind)
					return true;
				if(this->y_ind > other.y_ind)
					return false;
				return (this->z_ind < other.z_ind);
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
			 * @param tree  The originating octree
			 */
			void writeobj(std::ostream& os,
					const octree_t& tree) const;
	};
}
#endif
