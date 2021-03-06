#ifndef TRIANGULATION_H
#define TRIANGULATION_H

/* triangulation.h:
 *
 * This file defines structs used
 * for the triangulation of the generated
 * surface from grid carving */

#include <map>
#include <set>
#include <vector>
#include "dgrid.h"
#include "../util/parameters.h"

using namespace std;

/* The following descriptors are used for the
 * vertices, edges, and faces of a cube, respectively:
 *
 *
 *                                              z
 *         7 ________ 6           _____6__      ^      ________
 *         /|       /|         7/|       /|     |    /|       /|
 *       /  |     /  |        /  |     /5 |     |  /  5     /  |
 *   4 /_______ /    |      /__4____ /    10    |/_______2/    |
 *    |     |  |5    |     |    11  |     |     |     |  |   1 |
 *    |    3|__|_____|2    |     |__|__2__|     | 3   |__|_____|
 *    |    /   |    /      8   3/   9    /      |    /   |    /
 *    |  /     |  /        |  /     |  /1       |  /     4  /
 *    |/_______|/          |/___0___|/          |/_0_____|/________> x
 *   0          1                 
 *
 *
 * Note that voxels are cube corners.  Vertices are cube edges.
 */

class voxelface_t;
class vertex_t;
class triangle_t;
class triangulation_t;

/* This class represents the face between two voxels, which is a
 * unique edge in the lattice used by marching cubes.  Every
 * vertex generated by marching cubes will fall on such a face. */
class voxelface_t
{
	/* parameters */
	public:

	int x_ind; /* voxel index */
	int y_ind; /* voxel index */
	int z_ind; /* voxel index */
	unsigned char facenum; /* voxel face.  Can be: 0, 3, 4 */

	/* functions */
	public:

	/* constructors */
	voxelface_t();
	voxelface_t(int xi, int yi, int zi, unsigned char fn);
	~voxelface_t() {};

	/* overloaded operators */
	friend bool operator <  (const voxelface_t& lhs, 
					const voxelface_t& rhs);
	friend bool operator == (const voxelface_t& lhs, 
					const voxelface_t& rhs);
};

/* This class defines a vertex */
class vertex_t
{
	/* parameters */
	public:

	/* position in 3D space */
	double x;
	double y;
	double z;
	
	/* A unique hashcode for this vertex
	 * can be defined using these values. */
	voxelface_t hash; /* a unique identifier for vertex location */
	unsigned int index; /* a more compact unique id for export */

	/* optionally have color */
	unsigned char red;
	unsigned char green;
	unsigned char blue;

	/* a list of all triangles that contain this vertex */
	vector<triangle_t*> mytris;

	/* this boolean denotes if this triangle is shared by
	 * multiple regions */
	bool boundary;

	/* functions */
	public:

	/* constructor:
	 *
	 * 	Creates vertex based on hash h and voxel size vs.
	 */
	vertex_t(voxelface_t& h, dgrid_t& g);
	~vertex_t() {};

	/* init_pos:
	 *
	 * 	Computes the 3D position of this vertex in
	 * 	continuous space from the hash struct.
	 *
	 * arguments:
	 *
	 * 	g -	original voxel grid that formed this vertex.
	 */
	void init_pos(dgrid_t& g);
};

/* This class defines a triangle */
class triangle_t
{
	/* parameters */
	public:

	/* pointers to three vertices that make up this triangle.
	 * Stored in counter-clockwise order*/
	vertex_t* v[NUM_VERTS_PER_TRI];

	/* pointers to neighboring triangles (which each share
	 * two vertices with this triangle).  Triangle t[i]
	 * neighbors on the edge opposite v[i]. */
	triangle_t* t[NUM_EDGES_PER_TRI];

	/* index allows a triangle to know place in list */
	int index;

	/* the following value represents how many of this triangle's
	 * neighbors are in the same planar region as this triangle */
	int region_id; /* the id of the planar region that contains this */
	int region_neigh_count; /* # of neighbors in the same region */

	/* functions */
	public:

	/* constructors */
	triangle_t(vertex_t** vs, int ind);
	~triangle_t() {};

	/* shortest_edge:
	 *
	 * 	Returns the index of the shortest edge of this triangle.
	 */
	int shortest_edge();

	/* shares_edge_with:
	 * 
	 * 	Returns true iff this triangle shares exactly two
	 * 	vertices with the specified triangle argument.
	 */
	bool shares_edge_with(triangle_t* other);

	/* area:
	 *
	 * 	Computes the area of this triangle.
	 */
	double area();

	/* print:
	 *
	 * 	Useful for debugging, this function will print
	 * 	info about this triangle.
	 */
	void print();
};

/* This class defines a full triangulation in 3D space of a 2D manifold */
class triangulation_t
{
	/* parameters */
	public:

	/* vertices are stored in a map to prevent duplicates */
	map<voxelface_t, vertex_t*> vertices;

	/* triangles are stored in a list */
	vector<triangle_t*> triangles;

	/* functions */
	public:

	/* constructors */
	triangulation_t();
	~triangulation_t();

	/* generate:
	 *
	 * 	Populates the triangulation using the voxel
	 * 	grid g.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int generate(dgrid_t& g);

	/* map_neighbors:
	 *
	 * 	Assigns neighbor pointers for each triangle
	 * 	in this triangulation.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int map_neighbors();

	/* index_vertices:
	 *
	 *	Assigns unique indices to each vertex for easy
	 *	export.  These index values are not used internally.
	 */
	void index_vertices();

	/* index_triangles:
	 *
	 * 	Assigns unique indices to each triangle for
	 * 	easy export.
	 */
	void index_triangles();

	/* get_cube_description:
	 *
	 * 	Returns the description of the cube whose minimum corner
	 * 	is specified.
	 *
	 * 	If the voxel v is not the smallest boundary voxel, then
	 * 	the cube description returned will be 0.
	 *
	 * 	A cube description is an integer whose i'th bit is 1
	 * 	if and only if the i'th corner of the cube is solid.
	 *
	 * arguments:
	 *
	 * 	mc -	The location of the min. corner of the
	 * 		cube to analyze.
	 *
	 * 	v -	The generating voxel. Must be one of
	 * 		the corners of the cube.  Will only return
	 * 		a valid description if v is the smallest
	 * 		boundary corner.
	 *
	 * 	g -	The grid to use to get the cube description.
	 *
	 * return value:
	 * 
	 *	Returns cube description (which is potentially zero)
	 */
	int get_cube_description(voxel_t& mc, voxel_t& v, dgrid_t& g);

	/* do_cube:
	 *
	 * 	Performs marching cubes on the specified cube, and adds
	 * 	the generated vertices/triangles to the triangulation.
	 *
	 * arguments:
	 *
	 *	cube_description -	The integer representation of the
	 *				corners of the desired.  Each bit 
	 *				0/1 denotes solid/empty.
	 *
	 *	min_corner -		The voxel located at the min 
	 *				corner of this cube (Corner #0).
	 *
	 *	g -			Original voxel grid that
	 *				is used to triangulate.
	 *				(size of voxels is also the
	 *				size of the cube).
	 */
	void do_cube(int cube_description, voxel_t& min_corner, dgrid_t& g);

	/* reset_vertex_pos:
	 *
	 * 	Resets the positions of all vertices in this triangulation.
	 *
	 * arguments:
	 *
	 * 	g -	original voxel grid
	 */
	void reset_vertex_pos(dgrid_t& g);
};

#endif
