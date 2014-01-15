#ifndef TRI_REP_H
#define TRI_REP_H

/* tri_rep.h:
 *
 * 	This file contains the definitions of functions
 * 	and classes that represent triangulations, and allow
 * 	for partitioning of the triangulation based on geometry
 * 	measurements.
 */

#include <set>
#include <map>
#include <vector>
#include "../structs/point.h"
#include "../structs/triple.h"
#include "../delaunay/triangulation/triangulation.h"

using namespace std;

/* the following are the classes defined in this file */
class tri_info_t;
class tri_rep_t;
class tri_edge_t;
class room_info_t;
class room_height_t;

/* the following represents a triangle within a triangulation,
 * including metadata and topographical connections. */
class tri_info_t
{
	/*** parameters ***/
	public:

	/* circumcircle of triangle */
	double rcc;
	vertex_t cc;

	/* neighbors of triangle */
	set<triple_t> neighs;

	/* local geometry information */
	bool is_local_max;

	/* When partitioning triangulations, we denote a root triangle.
	 * If the root is this triangle, but the triangle is not a local
	 * max, then we can consider this triangle as unclaimed.  If this
	 * triangle is a local max, then it's its own root.  If the root
	 * is another triangle, then this triangle is claimed by that
	 * partition. */
	triple_t root;

	/*** functions ***/
	public:

	/* constructors */
	tri_info_t();
	tri_info_t(const triple_t& t, triangulation_t& tri,
					set<triple_t>& interior);
	~tri_info_t();

	/* init:
	 *
	 * 	Initializes this tri_info to a default state.
	 */
	void init();

	/* init:
	 *
	 * 	Initializes this tri_info based on
	 * 	a given triangle in the specified triangulation.
	 *
	 * arguments:
	 *
	 * 	t -		The indices of the vertices of the triangle
	 * 			to characterize
	 * 	tri -		The triangulation that t resides in
	 * 	interior -	The interior triangles of tri
	 */
	void init(const triple_t& t, triangulation_t& tri, 
					set<triple_t>& interior);
	
	/* init:
	 *
	 *	Will initialize the info of the triangle t, given
	 *	the connectivity described in trirep.
	 */
	void init(const triple_t& t, tri_rep_t& trirep);
};

/* a trirep_t stores the tri info for all triangles in a triangulation.
 * This is different than a triangulation_t since it may be only a subset
 * of the convex delaunay triangulation of a set of points (for instance,
 * the interior triangles). */
class tri_rep_t
{
	/*** parameters ***/
	public:
	
	/* The original delaunay triangulation this topology is
	 * referencing.  tri must be properly constructed before
	 * this trirep is initialized.
	 *
	 * Any modification of this tri_rep_t object will NOT be
	 * represented in the tri parameter.
	 */
	triangulation_t tri;

	/* the triangles and their properties */
	map<triple_t, tri_info_t> tris;

	/* vertices of triangles, and the neighboring triangles */
	map<int, set<triple_t> > vert_map;

	/* the following denotes the room heights, and is populated
	 * once the room labeling processing is complete. 
	 *
	 * The map goes from room roots to height ranges */
	map<triple_t, room_height_t> room_heights;

	/*** functions ***/
	public:

	/* constructors */
	tri_rep_t();
	~tri_rep_t();

	/* initialization */

	/* init:
	 *
	 * 	
	 * 	Creates a mapping between triangles and tri_info_t for each
	 * 	interior triangle.
	 *
	 * arguments:
	 *
	 * 	interior -	The interior triangles of tri
	 */
	void init(set<triple_t>& interior);

	/* accessors */

	/* contains:
	 *
	 *	Returns true iff t is an interior triangle in
	 *	this representation.
	 */
	bool contains(triple_t& t);

	/* add:
	 *
	 * 	Will add the specified triangle to this representation.
	 *
	 * return value:
	 *
	 * 	Returns the iterator to this triangle, and a boolean
	 * 	denoting whether the insertion was successful.  This
	 * 	is the same return type as std::map::insert()
	 */
	pair<map<triple_t, tri_info_t>::iterator, bool> 
			add(const triple_t& t);

	/* fill_polygonal_hole:
	 *
	 * 	Will populate the specified polygon with triangles.
	 * 	Assumes that this polygon is currently unrepresented
	 * 	in this triangulation.
	 *
	 * arguments:
	 *
	 * 	vs -	The circular list of vertices of the polygon
	 * 		to fill, in counter-clockwise order.
	 *
	 * 	root -	The root triangle to associate all created
	 * 		triangles with.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int fill_polygonal_hole(vector<int>& vs, const triple_t& root);

	/* pos:
	 *
	 * 	Returns the position of the specified vertex.
	 */
	inline point_t pos(int v)
	{
		vertex_t* vv;
	
		/* get vertex */
		vv = TRI_VERTEX_POS(&(this->tri), v);
	
		/* return position */
		return point_t(vv->pos[0], vv->pos[1]);
	};

	/* dist:
	 *
	 * 	Computes the distance between two vertices.
	 */
	double dist(int a, int b);

	/* remove:
	 *
	 * 	Removes the specified triangle from this representation.
	 */
	void remove(const triple_t& t);

	/* remove:
	 *
	 * 	Removes the specified vertex from this representation,
	 * 	removing any triangles that adjoing this vertex.
	 */
	void remove(int a);

	/* topology and geometry */

	/* compute_boundary_edges:
	 *
	 * 	Will determine the boundary edges, in counter-clockwise
	 * 	order, of a set of triangles.
	 *
	 * arguments:
	 *
	 *	edge_list -	Where to store the boundary edges.  The
	 *			length of this list will be 1 + the genus
	 *			of the polygon provided.  Each inner list
	 *			will be the vertices, ordered so that
	 *			counter-clockwise faces into the shape.
	 *
	 *	tris -		The set of triangles to analyze.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, and non-zero on failure.
	 */
	static int compute_boundary_edges(vector<vector<int> >& edge_list,
						set<triple_t>& tris);

	/* get_walls:
	 *
	 *	Computes the position of all boundary edges of this
	 *	triangulation.
	 *
	 * arguments:
	 *
	 * 	walls -	This will be modified.  It is a list of all edges.
	 * 		Each element represents one edge, oriented 
	 * 		counter-clockwise inward.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int get_walls(vector<edge_t>& walls);

	/* get_rooms:
	 *
	 * 	Populates the specified container with all the triangles
	 * 	in this triangulation, partitioned based on their room
	 * 	roots.
	 *
	 * 	Each element of the modified list will be a set of
	 * 	triangles, where the set corresponds to one room in
	 * 	the triangulation.
	 *
	 * arguments:
	 *
	 * 	rooms -	The container to populate with the room partitioning
	 */
	void get_rooms(vector<set<triple_t> >& rooms);

	/* orient_edge:
	 *
	 *	Returns true iff there exists a triangle in this topology
	 *	that has this edge, in this order.
	 *
	 *	Will optionally store this triangle in t if it exists.
	 */
	bool orient_edge(int a, int b);
	bool orient_edge(int a, int b, triple_t& t);

	/* room_edge:
	 *
	 * 	Checks if the specified edge separates rooms in this
	 * 	triangulation.
	 *
	 * arguments:
	 *
	 *	a,b -	The edge to analyze
	 *	t -	If this function returns true, t will be
	 *		the triangle that contains this edge.
	 *
	 * return value:
	 *
	 * 	Returns true iff a triangle contains this edge, and
	 * 	the other side of this edge is not a triangle with
	 * 	the same room.
	 */
	bool room_edge(int a, int b, triple_t& t);

	/* angle:
	 *
	 * 	Computes the angle <abc between the three given vertices.
	 *
	 * return value:
	 *
	 * 	Angle is computed in counter-clockwise fashion,
	 * 	and the return value will be in [-pi, pi].
	 *
	 *	If one or more vertex does not exist in this triangulation,
	 *	DBL_MAX is returned.
	 */
	double angle(int a, int b, int c);

	/* line_intersection:
	 *
	 *	Determines if the line segment a1->a2 intersects
	 *	the line b1->b2, based on vertex indices.
	 *
	 * return value:
	 *
	 * 	Returns true iff these line segments intersect.
	 */
	bool line_intersection(int a1, int a2, int b1, int b2);

	/* star_intersection:
	 *
	 * 	Checks if the star of the specified vertex intersects
	 * 	with the specified line segment.
	 *
	 * arguments:
	 *
	 * 	v -		The vertex whose star to analyze
	 * 	(a1,a2) -	The line segment to analyze
	 * 	to_ignore -	The set of vertices whose connections
	 * 			should not be checked.
	 *
	 * return value:
	 *
	 * 	Returns true iff the star intersects the line segment.
	 */
	bool star_intersection(int v, int a1, int a2, set<int>& to_ignore);

	/* in_triangle:
	 *
	 * 	Checks if a specified vertex is inside a triangle created
	 * 	by three other vertices.
	 *
	 * arguments:
	 *
	 * 	v -	The vertex to check for inside/outside test
	 * 	a,b,c -	The implied triangle
	 *
	 * return value:
	 *
	 * 	Returns true iff the specified vertex is inside the
	 * 	designated triangle.
	 */
	bool in_triangle(int v, int a, int b, int c);

	/* simplification and editing */

	/* collapse_edge:
	 *
	 *	Given a pair of vertices (a,b), will attempt to modify
	 *	this triangulation so that the edge (a,b) is collapsed
	 *	into the point a.  The vertex b will be deleted, and
	 *	any triangles connected to b will now connect to a.
	 *
	 *	Once this function is called, the tri parameter of
	 *	this class is invalidated, since this representation
	 *	has changed and may no longer be Delaunay.
	 *
	 * arguments:
	 *
	 *	a, b	-	Vertex indices of the endpoints of the
	 *			edge to collapse.
	 *
	 * return value:
	 *
	 * 	Returns zero if edge successfully collapsed.
	 * 	Returns positive if collapse was prevented by topology.
	 * 	Returns negative if error occurred attempting collapse.
	 */
	int collapse_edge(int a, int b);

	/* remove_boundary_vertex:
	 *
	 * 	Will remove vertex b from this triangulation.  If b
	 * 	is not connected to any boundary edges, then no
	 * 	processing is performed.
	 *
	 * 	This function may remove additional vertices in the
	 * 	process of retriangulating.
	 *
	 * arguments:
	 *
	 * 	b -		The index of the vertex to remove.
	 * 	verts_removed -	Any additioanl vertices that were removed
	 * 			will be identified here.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int remove_boundary_vertex(int b, set<int>& verts_removed);

	/* remove_interroom_columns:
	 *
	 * 	Will find all columns in the triangulation.  A column
	 * 	is a connected component of walls that is disjoint 
	 * 	from the other walls in the model.
	 *
	 * 	If this column occurs at the border of multiple rooms,
	 * 	and if it is smaller than the specified threshold (in
	 * 	perimeter), then it will be removed from the model.
	 *
	 * arguments:
	 *
	 * 	thresh -	A perimeter threshold used to classify
	 * 			the columns to remove.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int remove_interroom_columns(double thresh);

	/* processing */

	/* find_local_max:
	 *
	 * 	Given a fresh trirep mapping, will label triangles
	 * 	as local max if their circumcircles have a larger radius
	 * 	than any other circumcircle that theirs intersects.
	 */
	void find_local_max();

	/* flood_rooms:
	 *
	 * 	Assuming local max flags are set correctly (after having
	 * 	called find_local_max()), will associate all other
	 * 	triangles with one of the local maxima by flooding across
	 * 	triangle edges.
	 *
	 * 	The result will be a modification of the `root' fields
	 * 	in each tri_info_t.
	 */
	void flood_rooms();

	/* reset_roots:
	 *
	 * 	This resets the `root' field in each tri_info_t in this
	 * 	trirep.  Calling this function effectively undoes the
	 * 	computation of flood_rooms(), which is useful if the
	 * 	room labelings have changed.
	 */
	void reset_roots();

	/* unlabel_extra_rooms:
	 *
	 *	Assumes flood_rooms() has been called.
	 *
	 *	Will examine the boundries between the existing
	 *	flooded rooms.  If two rooms share a boundary that
	 *	is too big, then the smaller one is considered unnecessary
	 *	and should be merged with the larger one.
	 *
	 * return value:
	 * 	
	 * 	Returns the number of rooms unlabeled.  So if zero is
	 * 	returned, then no action was performed.
	 */
	int unlabel_extra_rooms();

	/* remove_unvisited_rooms:
	 *
	 *	Assumes that rooms have been labeled.
	 *
	 * 	Given a set of triangles that were visited by
	 * 	the path of the scanner, will remove find all
	 * 	rooms that contain no visited triangles.  All
	 * 	triangles within these rooms will be removed
	 * 	from this structure.
	 *
	 * arguments:
	 *
	 * 	visited -	The set of visited triangles.
	 *
	 * return value:
	 *
	 * 	Returns the number of rooms removed on success.
	 * 	Returns negative value on error.
	 */
	int remove_unvisited_rooms(set<triple_t>& visited);

	/* add_room_labels_to_graph:
	 *
	 * 	Uses the rooms defined in this triangulation
	 * 	representation to label the vertices in the
	 * 	graph that spawned the original triangulation.
	 *
	 * return value:
	 *
	 * 	Returns number of rooms on success, negative on failure.
	 */
	int add_room_labels_to_graph();

	/* populate_room_heights:
	 *
	 * 	Will force all rooms to have single height ranges.
	 * 	These ranges are chosen by the median values of min
	 * 	and max z values for each given room.
	 */
	void populate_room_heights();
	
	/* debug */

	/* verify:
	 *
	 * 	Will check that the toplogy of this triangulation
	 * 	is consistant.
	 *
	 * return value:
	 *
	 * 	Returns zero if verification passed.  Returns non-zero
	 * 	if triangulation is degenerate in some way.
	 */
	int verify();

	/* color_by_room:
	 *
	 * 	Will specify a color for the given vertex (or triangle)
	 * 	based on the room labels.
	 *
	 * arguments:
	 *
	 * 	v -	The vertex index to analyze (or triangle to analyze)
	 * 	r,g,b -	Where to store the computed color.
	 */
	void color_by_room(const triple_t& t, int& r, int& g, int& b);
	void color_by_room(int v, int& r, int& g, int& b);

	/* print:
	 *
	 *	Prints this triangle representation to a OBJ file.
	 *
	 * arguments:
	 *
	 * 	filename -	Where to write file
	 *	tri -		The original triangulation
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int print(char* filename);

	/*** helper functions ***/
	protected:

	/* add_edge_to_room:
	 *
	 * 	Will add the characteristics of the given 
	 * 	edge e to the room r.
	 *
	 * 	Will check that this edge is a valid boundary
	 * 	to the specified room.
	 */
	void add_edge_to_room(tri_edge_t& e, room_info_t& r);
};

/* this class represents the edge between two triangles,
 * with directionality.  It stores the starting triangle, the
 * destination triangle, and the length of the edge they share. */
class tri_edge_t
{
	/*** parameters ***/
	public:

	triple_t start;
	triple_t end;
	double len_sq;

	/*** functions ***/
	public:

	/* constructors:
	 *
	 * 	The starting triangle has the apex counter-
	 * 	clockwise from i,j.  The ending triangle has the
	 * 	apex counter-clockwise from j,i.
	 */
	tri_edge_t();
	tri_edge_t(unsigned int i, unsigned int j, triangulation_t& tri);
	~tri_edge_t() {};

	/* operators */
	
	inline bool operator < (const tri_edge_t& rhs) const
	{		
		return ((this->len_sq) < (rhs.len_sq));
	};
	inline bool operator == (const tri_edge_t& rhs) const
	{
		return ((this->len_sq) == (rhs.len_sq));
	};
	inline bool operator != (const tri_edge_t& rhs) const
	{
		return ((this->len_sq) != (rhs.len_sq));
	};
	inline tri_edge_t operator = (const tri_edge_t& rhs)
	{
		this->start = rhs.start;
		this->end = rhs.end;
		this->len_sq = rhs.len_sq;
		return (*this);
	};
};

/* the following represents geometry information about
 * a room, which is a collection of triangles. */
class room_info_t
{
	/*** parameters ***/
	public:

	triple_t root; /* the root triangle of the room */
	double area; /* the sum of the area of the triangles in this room */
	map<triple_t, double> border_lengths; /* the length of the
						perimeter shared with 
						each neighboring room */

	/*** functions ***/
	public:

	/* constructors */
	room_info_t();
	room_info_t(triple_t& t);
	~room_info_t();

	/* add_triangle:
	 *
	 * 	Adds the triangle's area to the room's info.
	 */
	void add_triangle(const triple_t& t, triangulation_t& tri);
};

/* the following stores the range of heights for a room (that is, floor
 * height and ceiling height) */
class room_height_t
{
	/*** parameters ***/
	public:

	double min_z; /* height of floor */
	double max_z; /* height of ceiling */

	/*** functions ***/
	public:

	/* constructors */
	room_height_t() { this->min_z = this->max_z = 0.0; };
	room_height_t(double low, double high)
		{ this->min_z = low; this->max_z = high; };
	~room_height_t() {};
};

#endif
