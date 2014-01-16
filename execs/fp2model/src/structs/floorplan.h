#ifndef FLOORPLAN_H
#define FLOORPLAN_H

/* floorplan.h:
 *
 * This file defines classes used to define
 * a 2D floorplan that describes a bounding shape
 * for the pointcloud being generated.
 */

#include <vector>
#include <set>
#include "../util/parameters.h"

using namespace std;

/* the following defines the classes that
 * this file contains */
class floorplan_t;
class vertex_t;
class edge_t;
class triangle_t;
class room_t;

/* the floorplan class defines a full 2D floorplan of the environment */
class floorplan_t
{
	/*** parameters ***/
	public:

	/* a list of all vertices in this floorplan  -- all
	 * vertices are referenced by their indices */
	vector<vertex_t> verts;

	/* a list of all triangles -- all triangles are
	 * referenced by their indices in this list */
	vector<triangle_t> tris;

	/* a list of all rooms -- all rooms are
	 * referenced by their indices in this list */
	vector<room_t> rooms;

	/* the following is some useful metadata about the floorplan */
	double res; /* estimate of floorplan's resolution, in meters */

	/*** functions ***/
	public:

	/* constructors */
	floorplan_t();
	~floorplan_t();

	/* clear:
	 *
	 * 	Clears all data from floorplan.
	 */
	void clear();

	/* import_from_fp:
	 *
	 * 	Imports floorplan information from .fp file.
	 *
	 *	Implemented in floorplan_input.cpp.  See this file for
	 *	file format information.
	 *
	 * arguments:
	 *
	 * 	filename -	The file to parse
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int import_from_fp(char* filename);

	/* add:
	 *
	 * 	Adds a copy of the specified vertex, triangle, or 
	 * 	room to this floorplan, keeping track of connectivity.
	 *
	 * 	Vertex <-> Triangle connectivity will be recorded
	 * 	but no Triangle <-> Triangle connectivity will be
	 * 	computed.
	 *
	 *	Implemented in floorplan_input.cpp
	 *
	 * argument:
	 * 
	 * 	The element to add
	 */
	void add(vertex_t& v);
	void add(triangle_t& t);
	void add(room_t& r);

	/* map_neighbors:
	 *
	 * 	Will map triangle <-> triangle neighborings.
	 *
	 *	Will also compute vertex heights from room heights.
	 *
	 * 	Implemented in floorplan_input.cpp
	 */
	void map_neighbors();

	/* compute_edges:
	 *
	 * 	Will add all boundary edges of this mesh to
	 * 	the specified list.  No specific ordering
	 * 	is guaranteed.
	 *
	 * arguments:
	 *
	 * 	edges -	Where to store the edges.  Any existing
	 * 		elements in this list will be erased.
	 */
	void compute_edges(vector<edge_t>& edges);

	/* compute_bounds:
	 *
	 * 	Computes the 2D bounds on this floorplan.
	 *
	 * arguments:
	 *
	 * 	min_x,
	 * 	min_y,
	 * 	max_x,
	 * 	max_y -	Where to store the bounds for this floorplan
	 */
	void compute_bounds(double& min_x, double& min_y,
				double& max_x, double& max_y);

	/* export_to_obj:
	 *
	 * 	Will export an extruded mesh to the specified
	 * 	Wavefront OBJ file.
	 *
	 *	Implemented in floorplan_output.cpp
	 *
	 * arguments:
	 *
	 * 	filename -	Where to write the mesh.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int export_to_obj(char* filename);
};

/* the vertex class references a geometric position and neighboring
 * triangles.  This is assumed to be a 2D floorplan, so the vertex position
 * is only in X and Y */
class vertex_t
{
	/*** parameters ***/
	public:

	/* vertex position (in meters) */
	double x;
	double y;
	double min_z;
	double max_z;
	
	int ind; /* index of this vertex */
	set<int> tri_neighs; /* neighboring triangles */

	/*** functions ***/
	public:

	/* constructors */
	vertex_t();
	~vertex_t();

	/* clear:
	 *
	 * 	Resets the values of this vertex.
	 */
	void clear();

	/* operators */

	inline vertex_t operator = (const vertex_t& rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->min_z = rhs.min_z;
		this->max_z = rhs.max_z;
		this->ind = rhs.ind;
		this->tri_neighs.clear();
		this->tri_neighs.insert(rhs.tri_neighs.begin(),
					rhs.tri_neighs.end());
		return (*this);
	};
};

/* an edge defines a connection between two vertices */
class edge_t
{
	/*** parameters ***/
	public:

	/* the indices of the connected vertices */
	int verts[NUM_VERTS_PER_EDGE];

	/*** functions ***/
	public:

	/* constructors */
	edge_t();
	edge_t(int i, int j);
	~edge_t();

	/* set:
	 *
	 * 	Initializes this edge to the specified indices.
	 */
	inline void set(int i, int j)
	{
		verts[0] = i;
		verts[1] = j;
	};

	/* flip:
	 *
	 * 	Returns the reverse of this edge.
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

	/* operators */

	inline bool operator < (const edge_t& rhs) const
	{
		if(this->verts[0] < rhs.verts[0])
			return true;
		if(this->verts[0] > rhs.verts[0])
			return false;
		return (this->verts[1] < rhs.verts[1]);
	};
	inline bool operator == (const edge_t& rhs) const
	{
		return (this->verts[0] == rhs.verts[0]
				&& this->verts[1] == rhs.verts[1]);
	};
	inline bool operator != (const edge_t& rhs) const
	{
		return (this->verts[0] != rhs.verts[0]
				|| this->verts[1] != rhs.verts[1]);
	};
	inline edge_t operator = (const edge_t& rhs)
	{
		int i;
		
		for(i = 0; i < NUM_VERTS_PER_EDGE; i++)
			this->verts[i] = rhs.verts[i];

		return (*this);
	};
};

/* the triangle class references the indices of three vertices
 * and those of neighboring triangles */
class triangle_t
{
	/*** parameters ***/
	public:

	/* the vertices that define this triangle */
	int verts[NUM_VERTS_PER_TRI]; 

	/* the neighboring triangles, with each neighbor
	 * opposite the corresponding vertex */
	int neighs[NUM_EDGES_PER_TRI];

	/* the index of this triangle */
	int ind;

	/*** functions ***/
	public:

	/* constructors */
	triangle_t();
	~triangle_t();

	/* accessors */

	/* get_edge:
	 *
	 * 	Returns the edge referenced by the argument
	 * 	index position.
	 */
	edge_t get_edge(int ni) const;

	/* topology */

	/* make_neighbors_with:
	 *
	 * 	Checks if this triangle is neighbors with
	 * 	the other triangle.  If so, will update
	 * 	the neighbor information of each triangle accordingly.
	 *
	 *	Implemented in floorplan_input.cpp
	 *
	 * return value:
	 *
	 * 	Returns true iff they are neighbors.
	 */
	bool make_neighbors_with(triangle_t& other);
};

/* definition of the room class, which represents a set of triangles */
class room_t
{
	/*** parameters ***/
	public:

	/* the set of triangles that are in this room */
	set<int> tris;

	/* the index of this room */
	int ind;

	/* the floor and ceiling heights of this room */
	double min_z;
	double max_z;

	/*** functions ***/
	public:

	/* constructors */
	room_t();
	~room_t();
	
	/* clear:
	 *
	 * 	Resets the info in this struct.
	 */
	void clear();
	
	/* operators */

	inline room_t operator = (const room_t& rhs)
	{
		this->min_z = rhs.min_z;
		this->max_z = rhs.max_z;
		this->ind = rhs.ind;
		this->tris.clear();
		this->tris.insert(rhs.tris.begin(), rhs.tris.end());
		return (*this);
	};
};

#endif
