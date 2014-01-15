#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include "linkring.h"
#include "vertex.h"

/* triangulation.h:
 *
 * This file defines the triangulation structure
 * and corrresponding functions, which are used to
 * store and access a triangulation of points.
 *
 * The data-structure defined below is the one
 * defined by Blandford, Blelloch, Cardoze,
 * and Kadow.
 *
 * It uses the link-ring structure defined in
 * linkring.h
 */

/* the following defines are used by triangulation */
#define VERTICES_PER_TRIANGLE 3
#define GHOST_VERTEX 0

/* the following defines the struct
 * used to represent a triangulation */
typedef struct triangulation
{
	/* the number of vertices stored
	 * in this triangulation. */
	unsigned int num_verts;

	/* how many vertices this triangulation
	 * has allocated capacity for */
	unsigned int vert_cap;

	/* store vertex positions */
	vertex_t* vertices;

	/* for each vertex, store a link-ring */
	linkring_t* links;

	/* The index starting value is meant to
	 * represent solely the indexing scheme
	 * of the *.node file that was read */
	int starting_index;

	/* store the most recent triangle, for
	 * vertex localization purposes */
	unsigned int last_tri[VERTICES_PER_TRIANGLE];

} triangulation_t;

/* this returns a pointer to the vertex position
 * of the given vertex number */
#define TRI_VERTEX_POS(tri,v) ( ((v) == GHOST_VERTEX) ? NULL \
				: (tri)->vertices + VERT_NUM_2_IND(v) )

/* returns pointer to requested linkring */
#define TRI_GET_LINKRING(tri,v) ( ((v) > (tri)->num_verts) \
					? NULL : ((tri)->links + v) )

/* the following is used to compute an
 * array index given a vertex number.
 * These are not the same since the Ghost
 * vertex doesn't need to be stored in the
 * array */
#define VERT_NUM_2_IND(v) ((v)-1)
#define IND_2_VERT_NUM(i) ((i)+1)

/* tri_init:
 *
 * 	Initializes the specified triangulation
 * 	to be empty and valid. The empty triangulation
 * 	contains one vertex: the ghost vertex.
 *
 * arguments:
 *
 * 	tri -	The triangulation to initialize
 */
void tri_init(triangulation_t* tri);

/* tri_cleanup:
 *
 * 	Frees any allocated resources used by this tri.
 * 	All vertices and links within the tri will also
 * 	be freed.
 *
 * 	The result will be that the specified tri is still
 * 	valid, but empty.
 *
 * arguments:
 *
 * 	tri -	The tri to clean-up.
 */
void tri_cleanup(triangulation_t* tri);

/* tri_change_cap:
 *
 * 	Modify allocated space for this structure.  This is also
 * 	done automatically when adding vertices or triangles.
 *
 * arguments:
 *
 * 	tri -	Triangulation struct to modify
 * 	nc -	New capacity
 */
void tri_change_cap(triangulation_t* tri, unsigned int nc);

/* tri_add_vertex:
 *
 * 	Will add a vertex and return the new vertex index.
 *
 * arguments:
 *
 * 	tri -	Triangulation to modify
 * 	v -	Vertex to add.  A copy of this information is made.
 *
 * Return value:
 *
 * 	Returns (positive) vertex index on success, negative
 * 	value on failure.
 */
int tri_add_vertex(triangulation_t* tri, vertex_t* v);

/* tri_set_neighbors:
 *
 * 	Will modify the triangles represented in the given
 * 	triangulation so that the link set of vertex v is equal
 * 	to the ordered list neighs.
 *
 * arguments:
 *
 * 	tri -	Triangulation to modify
 *
 * 	v -	The index of the vertex to modify.  After this call,
 * 		v will be connected to a vertex w by a triangle if 
 * 		and only if w is in neighs.
 *
 * 	neighs -	Link-ring describing the neighbors of v.  They
 * 			must be listed in counter-clockwise order.  After
 * 			this call, the struct pointed to by neighs will
 * 			be modified so that it is empty.
 *
 * return value:
 *
 * 	Returns 0 if successful, non-zero if error occurred.
 */
int tri_set_neighbors(triangulation_t* tri, unsigned int v,
						linkring_t* neighs);

/* tri_get_apex:
 *
 * 	Given an edge defined by two vertex indices, will return the
 * 	third vertex v2 of the triangle (v0, v1, v2), where those
 * 	vertices are defined in counter-clockwise order.
 *
 * arguments:
 *
 * 	tri -	The triangulation to observe.
 * 	v0,v1 -	Vertices within tri
 *
 * return value:
 *
 * 	If successfull, will return non-negative vertex index of v2.
 * 	If failure (such as no edge v0 <-> v1 existing), will return
 * 	negative value.
 */
int tri_get_apex(triangulation_t* tri, unsigned int v0, unsigned int v1);

/* tri_get_directions:
 *
 *	Given a triangulation tri and a point in 2D space pos, will use
 *	the specified triangle in tri to help locate pos.  If the call
 *	is successful, will return whether pos is inside (v0,v1,v2), or
 *	if not, which direction it is from the triangle.
 *
 * arguments:
 *
 * 	tri -		Triangulation to analyze
 * 	start -		A starting position to traverse from
 * 	pos -		Position to locate
 * 	(v0,v1,v2) -	Triangle to use to locate pos
 *
 * return value:
 *
 * 	For i in {0,1,2}, returns i iff vertex is on other side of
 * 	triangle edge opposite vi.
 *
 * 	Returns 3 iff pos is inside the triangle (v0,v1,v2)
 * 
 * 	Returns negative value if error occurred.
 */
int tri_get_directions(triangulation_t* tri, vertex_t* start, 
		vertex_t* pos, unsigned int v0, unsigned int v1,
					unsigned int v2);

/* tri_locate:
 *
 * 	Will determine which triangle in the given triangulation
 * 	contains the vertex v.  Optionally given a starting triangle
 * 	to walk from.
 *
 * arguments:
 *
 *	tri -		The triangulation to analyze
 *	v -		The vertex to locate
 *	(s0,s1,s2) -	A starting triangle
 *	(v0,v1,v2) -	Where to store the triangle found
 *
 * return value:
 *
 * 	Returns 0 if successful, non-zero on failure.
 */
int tri_locate(triangulation_t* tri, vertex_t* v,
		unsigned int s0, unsigned int s1, unsigned int s2,
		unsigned int* v0, unsigned int* v1, unsigned int* v2);

/* tri_verify_delaunay:
 *
 * 	Will iterate through every triangle, and verify that
 * 	each interior edge is locally delaunay.
 *
 * arguments:
 *
 * 	tri -	Triangulation to verify.
 *
 * output:
 *
 * 	Returns 1 if delaunay, 0 otherwise.
 */
int tri_verify_delaunay(triangulation_t* tri);

/* tri_print_triang:
 *
 * 	Prints the triangulation structure for debugging purposes.
 */
void tri_print(triangulation_t* tri);

#endif
