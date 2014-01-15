#ifndef INSERTION_H
#define INSERTION_H

#include "triangulation/triangulation.h"

/* insertion.h
 *
 *	This file contains functions used
 *	to perform incremental triangle and vertex
 *	insertion on a triangulation.  Each time a vertex
 *	is inserted (which is to say there exist triangles
 *	containing that vertex), this package will guarantee
 *	that all triangles after the insertion are Delaunay.
 */

/* begin_triangulation:
 *
 * 	Given a triangulation that contains N vertices, but no
 * 	triangles, will insert two vertices (plus the ghost vertex)
 * 	so that there are two triangles.
 *
 * 	After this step, insert_vertex() can be called to incrementally
 * 	insert vertices.
 *
 * arguments:
 *
 * 	tri -	A triangulation containing all vertices but no triangles.
 *
 * return value:
 *
 *	Return 0 on success, non-zero on failure.
 */
int begin_triangulation(triangulation_t* tri);

/* insert_vertex:
 *
 * 	Given a partial triangulation, will insert the vertex
 * 	specified by vertex number v.
 *
 * arguments:
 *
 * 	tri -	The partial triangulation to modify
 * 	v -	The vertex number of the vertex to insert.  Note
 * 		that v must not be a part of any triangles.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int insert_vertex(triangulation_t* tri, unsigned int v);

/* search_circumcircles:
 *
 * 	Given a point vp, will search for all triangles that contain
 * 	vp in their circumcircles.  vp is assumed to reside inside
 * 	triangle (f0,f1,f2) in the triangulation tri.
 *
 * arguments:
 *
 *	tri -		The triangulation to search
 *	
 *	vp -		The vertex to analyze
 *	
 *	lrt -		Where to store the result.  Before a call to
 *			this function, lrt should be an initialized to
 *			contain (f0,f1,f2), which should be the vertices
 *			of the triangle that contains vp (listed in
 *			counter-clockwise order).  After the call, it will
 *			contain the vertex numbers of the border of the
 *			cavity within tri that vp induces.  These vertices
 *			will be listed in counter-clockwise order.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int search_circumcircles(triangulation_t* tri, vertex_t* vp, 
					linkring_t* lrt);

/* search_outer_edge:
 *
 * 	Given a point vp, that lies outside the triangulation tri (i.e.,
 * 	inside a triangle with the ghost vertex), will determine which
 * 	boundary edges of the triangulation face vp.
 *
 * 	The initial boundary edge should be listed in lrt, so that
 * 	the lrt is length 2, and (lrt(0), lrt(1)) represents the edge
 * 	as traversed clockwise along the boundary of the triangulation.
 *
 * arguments:
 *
 * 	tri -	The triangulation to search
 * 	vp -	The vertex to analyze
 * 	lrt -	Where to store the two vertices that make up the outer edge
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int search_outer_edge(triangulation_t* tri, vertex_t* vp, linkring_t* lrt);

#endif
