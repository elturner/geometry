#ifndef FLOORPLAN_H
#define FLOORPLAN_H

/**
 * @file floorplan.h
 * @author Eric Turner
 *
 * @section DESCRIPTION
 *
 * This file defines classes used to define
 * a 2D floorplan with extruded height information.
 * A floorplan is composed of a set of rooms, where each
 * room is a 2D triangulation with a set floor and ceiling
 * height.
 */

#include <string>
#include <vector>
#include <set>

/**
 * all floorplan-specific classes are within the 'fp' namespace.
 */
namespace fp
{
	/* the following defines the classes that
	 * this file contains */
	class floorplan_t;
	class vertex_t;
	class edge_t;
	class triangle_t;
	class room_t;

	/**
	 * This class defines a full 2D floorplan of the environment
	 */
	class floorplan_t
	{
		/* parameters */
		public:

			/**
			 * A list of vertices composing the floorplan mesh
			 *
			 * A list of all vertices in this floorplan, all
			 * vertices are referenced by their indices
			 */
			std::vector<vertex_t> verts;

			/**
			 * A list of all triangles in the floorplan.
			 *
			 * All triangles are referenced by their indices
			 * in this list
			 */
			std::vector<triangle_t> tris;

			/**
			 * A list of all rooms.
			 *
			 * All rooms are referenced by their indices 
			 * in this list.
			 */
			std::vector<room_t> rooms;

			/**
			 * Resolution of floorplan, in meters.
			 *
			 * The following is some useful metadata about the
			 * floorplan.  This parameter estimates the
			 * floorplan's resolution, in meters.
			 */
			double res;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/
		
			/**
			 * Initializes empty floorplan
			 */
			floorplan_t();
			
			/**
			 * Frees all memory and resources
			 */
			~floorplan_t();

			/**
			 * Clears all data from floorplan.
			 */
			void clear();

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Imports floorplan information from .fp file.
			 *
			 * This function is implemented in 
			 * floorplan_input.cpp.  See this file for 
			 * file format information.
			 *
			 * @param filename      The file to parse
			 *
			 * @return       Returns zero on success,
			 *               non-zero on failure.
			 */
			int import_from_fp(const std::string& filename);

			/**
			 * Adds a copy of the specified vertex
			 *
			 * Adds vertex to this floorplan, keeping track 
			 * of connectivity information.
			 *
			 * Implemented in floorplan_input.cpp
			 *
			 * @param v   The element to add
			 */
			void add(const vertex_t& v);

			/**
			 * Adds a copy of the specified triangle
			 *
			 * Adds triangle to this floorplan, keeping track 
			 * of connectivity information.
			 *
			 * Vertex <-> Triangle connectivity will be recorded
			 * but no Triangle <-> Triangle connectivity will be
			 * computed.
			 *
			 * Implemented in floorplan_input.cpp
			 *
			 * @param t   The element to add
			 */	
			void add(const triangle_t& t);

			/**
			 * Adds a copy of the specified room
			 *
			 * Adds room to this floorplan, keeping track 
			 * of connectivity information.
			 *
			 * Implemented in floorplan_input.cpp
			 *
			 * @param r   The element to add
			 */
			void add(const room_t& r);

			/**
			 * Will map triangle <-> triangle neighborings.
			 *
			 * Will also compute vertex heights from room
			 * heights and store that information.
			 *
			 * Implemented in floorplan_input.cpp
			 */
			void map_neighbors();
			
			/**
			 * Will export an extruded mesh to the specified
			 * Wavefront OBJ file.
			 *
			 * Implemented in floorplan_output.cpp
			 *
			 * @param filename     Where to write the mesh.
			 *
			 * @return        Returns zero on success,
			 *                non-zero on failure.
			 */
			int export_to_obj(
				const std::string& filename) const;

			/*----------*/
			/* geometry */
			/*----------*/

			/**
			 * Computes all boundary edges of this mesh.
			 *
			 * Will add all boundary edges of this mesh to
			 * the specified list.  No specific ordering
			 * is guaranteed.
			 *
			 * Any existing elements in the input list will
			 * be erased.
			 *
			 * @param edges       Where to store the edges.
			 */
			void compute_edges(
				std::vector<edge_t>& edges) const;

			/**
			 * Computes boundary edges for the given room
			 *
			 * Will add all boundary edges of the specified
			 * room to the given list.  No specific ordering
			 * is guaranteed.
			 *
			 * Any existing elements in this list will
			 * be erased.
			 *
			 * @param edges    Where to store the edges
			 * @param ri       The index of room to analyze
			 */
			void compute_edges_for_room(
					std::vector<edge_t>& edges,
					unsigned int ri) const;

			/**
			 * Computes the 2D bounds on this floorplan.
			 *
			 * Will find the 2D bounding box, and store the
			 * values in the specified input locations.
			 *
			 * @param min_x  Where to store min. x position
			 * @param min_y  Where to store min. y position
			 * @param max_x  Where to store max. x position
			 * @param max_y  Where to store max. y position
			 */
			void compute_bounds(double& min_x, double& min_y,
			             double& max_x, double& max_y) const;
	
			/**
			 * Computes the area (in square meters) of a room
			 *
			 * Given the room index, will return the floorspace
			 * area of that room.
			 *
			 * @param i   The room index to analyze
			 *
			 * @return    Returns the area in square meters
			 */
			double compute_room_area(unsigned int i) const;

			/**
			 * Computes area of full floorplan
			 *
			 * This will compute the area of all rooms and
			 * return the sum.  The value returned will be
			 * in units of square meters.
			 *
			 * @return   Returns the total area of the floorplan
			 */
			double compute_total_area() const;
	};

	/**
	 * This class represents a 2.5D vertex.
	 *
	 * The vertex class references a geometric position and neighboring
	 * triangles.  This is assumed to be a 2D floorplan, so
	 * the vertex position is only in X and Y
	 */
	class vertex_t
	{
		/* parameters */
		public:

			/* vertex position (in meters) */
			double x;
			double y;
			double min_z;
			double max_z;
	
			int ind; /* index of this vertex */
			std::set<int> tri_neighs; /* neighboring tris */

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/
			
			/**
			 * Initializes empty vertex structure
			 */
			vertex_t();

			/**
			 * Frees all memory and resources
			 */
			~vertex_t();

			/**
			 * Resets the values of this vertex.
			 */
			void clear();

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Sets this vertex's value to be equal to argument
			 *
			 * Will copy all information from rhs and store
			 * in this vertex object.
			 *
			 * @param rhs   The reference object to copy
			 *
			 * @return   Returns a copy of this vertex
			 */
			inline vertex_t operator = (const vertex_t& rhs)
			{
				/* copy all information */
				this->x = rhs.x;
				this->y = rhs.y;
				this->min_z = rhs.min_z;
				this->max_z = rhs.max_z;
				this->ind = rhs.ind;
				this->tri_neighs.clear();
				this->tri_neighs.insert(
					rhs.tri_neighs.begin(),
					rhs.tri_neighs.end());
				return (*this);
			};
	};

	/* this constant represents geometrical properties of edges */
	static const unsigned int NUM_VERTS_PER_EDGE = 2;

	/**
	 * an edge defines a connection between two vertices
	 */
	class edge_t
	{
		/* parameters */
		public:

			/* the indices of the connected vertices */
			int verts[NUM_VERTS_PER_EDGE];

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Initializes default edge object
			 */
			edge_t();

			/**
			 * Initializes edge between vertices #i and #j
			 */
			edge_t(int i, int j);
			
			/**
			 * Initializes this edge to the specified indices.
			 *
			 * @param i   The first vertex index to reference
			 * @param j   The other vertex index to reference
			 */
			inline void set(int i, int j)
			{
				verts[0] = i;
				verts[1] = j;
			};

			/**
			 * Returns the reverse of this edge.
			 *
			 * That is, it returns an edge that has the
			 * same verices, but points in the opposite
			 * direction.
			 */
			inline edge_t flip() const
			{
				edge_t e;
		
				/* flip edge */
				e.verts[0] = this->verts[1];
				e.verts[1] = this->verts[0];
		
				/* return it */
				return e;
			};

			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Provides a sorting for edges
			 *
			 * This sorting is useful for storing edges
			 * in sets or maps, which require comparison
			 * operators.
			 *
			 * @param rhs    The other edge to compare to
			 *
			 * @return  Returns true iff this edge less than rhs
			 */
			inline bool operator < (const edge_t& rhs) const
			{
				if(this->verts[0] < rhs.verts[0])
					return true;
				if(this->verts[0] > rhs.verts[0])
					return false;
				return (this->verts[1] < rhs.verts[1]);
			};

			/**
			 * Checks for equality among edges
			 *
			 * Edges must not only have the same vertices,
			 * but also be in the same order/direction, to
			 * be considered equal.
			 *
			 * @param rhs   The other edge to compare
			 *
			 * @return    Returns true iff edges are equal
			 */
			inline bool operator == (const edge_t& rhs) const
			{
				return (this->verts[0] == rhs.verts[0]
					&& this->verts[1] == rhs.verts[1]);
			};
		
			/**
			 * Checks if edges are not equal
			 *
			 * @param rhs   The other edge to compare
			 *
			 * @return   Returns true iff edges are not equal
			 */
			inline bool operator != (const edge_t& rhs) const
			{
				return (this->verts[0] != rhs.verts[0]
					|| this->verts[1] != rhs.verts[1]);
			};

			/**
			 * Assigns rhs value to this edge
			 *
			 * Will copy information from rhs to be stored
			 * in this edge
			 *
			 * @param rhs   The edge to copy
			 *
			 * @return   Returns a copy of this edge
			 */
			inline edge_t operator = (const edge_t& rhs)
			{
				unsigned int i;
			
				for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
					this->verts[i] = rhs.verts[i];

				return (*this);
			};
	};

	/* The following values indicate geometric constants of triangles */
	static const unsigned int NUM_VERTS_PER_TRI = 3;
	static const unsigned int NUM_EDGES_PER_TRI = 3;

	/**
	 * This class represents a triangle object
	 *
	 * The triangle class references the indices of three vertices
	 * and those of neighboring triangles
	 */
	class triangle_t
	{
		/* parameters */
		public:

			/* the vertices that define this triangle */
			int verts[NUM_VERTS_PER_TRI]; 

			/* the neighboring triangles, with each neighbor
			 * opposite the corresponding vertex */
			int neighs[NUM_EDGES_PER_TRI];

			/* the index of this triangle */
			int ind;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/
			
			/**
			 * Constructs a default triangle.
			 */
			triangle_t();

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Retrieves the specified edge
			 *
			 * Returns the edge referenced by the argument
			 * index position.
			 *
			 * @param ni   The index of the edge to return
			 *
			 * @return   Returns the edge in question.
			 */
			edge_t get_edge(unsigned int ni) const;

			/*----------*/
			/* topology */
			/*----------*/

			/**
			 * Updates neighbor information of triangles
			 *
			 * Checks if this triangle is neighbors with
			 * the other triangle.  If so, will update
			 * the neighbor information of each triangle
			 * accordingly.
			 *
			 * Implemented in floorplan_input.cpp
			 * 
			 * @param other   The neighboring triangle
			 *
			 * @return     Returns true iff they are neighbors.
			 */
			bool make_neighbors_with(triangle_t& other);
	};

	/**
	 * Definition of the room class, which represents a set of triangles
	 */
	class room_t
	{
		/* parameters */
		public:

			/* the set of triangles that are in this room */
			std::set<int> tris;

			/* the index of this room */
			int ind;

			/* the floor and ceiling heights of this room */
			double min_z;
			double max_z;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/
			
			/**
			 * Initializes empty room
			 */
			room_t();

			/**
			 * Frees all memory and resources
			 */
			~room_t();
	
			/**
			 * Resets the info in this struct.
			 */
			void clear();
	
			/*-----------*/
			/* operators */
			/*-----------*/

			/**
			 * Copies infromation from rhs into this room
			 *
			 * @param rhs  The other room to copy from
			 *
			 * @return   Returns the updated room
			 */
			inline room_t operator = (const room_t& rhs)
			{
				/* copy all values */
				this->min_z = rhs.min_z;
				this->max_z = rhs.max_z;
				this->ind = rhs.ind;
				this->tris.clear();

				/* reinsert each triangle index */
				this->tris.insert(rhs.tris.begin(),
				                  rhs.tris.end());

				/* return updated room */
				return (*this);
			};
	};
}

#endif
