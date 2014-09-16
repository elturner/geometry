#ifndef REGION_GROWING_H
#define REGION_GROWING_H

/* region_growing.h:
 *
 * Traverses over the triangles of a triangulation
 * in order to group each into regions based on planarity.
 */

#include <set>
#include "../structs/normal.h"
#include "../structs/triangulation.h"

using namespace std;

/* the following class represents an
 * edge in the triangulation, by specifying
 * a triangle and its edge number. */
class edge_t
{
	/* parameters */
	public:

	/* the edge connects two vertices in the triangulation */
	vertex_t* start;
	vertex_t* end;
	
	/* functions */
	public:

	/* constructors */
	edge_t();
	edge_t(vertex_t* s, vertex_t* e);
	~edge_t() {};

	/* set:
	 *
	 * 	Sets the endpoints of this edge.
	 */
	void init(vertex_t* s, vertex_t* e);

	/* get_reverse:
	 *
	 * 	Populate the argument
	 * 	with the reverse of this
	 * 	edge.
	 */
	void get_reverse(edge_t& e) const;

	/* operators */
	
	inline bool operator < (const edge_t& rhs) const
	{
		if(this->start < rhs.start)
			return true;
		if(this->start > rhs.start)
			return false;
		return (this->end < rhs.end);
	};
	inline bool operator == (const edge_t& rhs) const
	{
		return (this->start == rhs.start) && (this->end == rhs.end);
	};
	inline bool operator != (const edge_t& rhs) const
	{
		return (this->start != rhs.start) || (this->end != rhs.end);
	};
};

/* the following class represents a single planar region
 * composed of many triangles. */
class planar_region_t
{
	/* parameters */
	public:

	/* the average normal direction of all faces
	 * in this planar region */
	normal_t avg_norm;
	point_t avg_pos;

	/* a list of all triangles in this planar region */
	set<triangle_t*> tris;

	/* a list of all boundary edges of this region. This
	 * is a subset of the triangle edges in tris */
	set<edge_t> boundary;
	
	/* for convenience, store the total surface area of this
	 * region.  A valid of -1 indicates the area has not yet
	 * been calculated. */
	double my_area;

	/* functions */
	public:

	/* constructors */
	planar_region_t() { this->my_area = -1; };
	~planar_region_t() {};

	/* overloaded operators */
	planar_region_t& operator=(const planar_region_t& prt);

	/* grow_from_seed:
	 *
	 *	Finds all triangles in the same region as the given
	 *	triangle seed.
	 *
	 * arguments:
	 *
	 * 	seed -	The starting point
	 */
	void grow_from_seed(triangle_t* seed);

	/* add_boundary_edges:
	 *
	 * 	Given an ordered array of vertices, will add
	 * 	any edges that contain two boundary vertices.
	 *
	 * arguments:
	 *
	 * 	vs -	The array of vertices to check
	 * 	n -	The size of vs
	 */
	void add_boundary_edges(vertex_t** vs, int n);

	/* add_boundary_edge:
	 *
	 *	Given an edge, will include it in the boundary
	 *	of this region.
	 */
	void add_boundary_edge(const edge_t& e);

	/* area:
	 *
	 * 	Computes the sum of the areas of the triangles
	 * 	of this region.
	 */
	double area();
};

/* region_grow_all:
 *
 * 	Partitions all triangles into planar regions.
 *
 * arguments:
 *
 * 	rl -	Where to store the list of planar regions
 * 	tri -	The triangulation to partition.
 */
void region_grow_all(vector<planar_region_t>& rl, triangulation_t& tri);

/* region_grow_coalesce_small:
 *
 * 	Will examine the found planar regions, and check for
 * 	any that are considered too small.
 *
 * 	If a region is considered too small and is completely surrounded
 * 	by one other region, then the triangles of the smaller region
 * 	are merged into the larger region.
 *
 * arguments:
 *
 * 	rl -	The planar regions to examine and modify.
 */
void region_grow_coalesce_small(vector<planar_region_t>& rl);

/* region_grow_coalesce:
 *
 *	Will iterate over the given regions, and merge regions that are
 *	too small with larger neighbors.
 *
 *	Unlike region_grow_coalesce_small(), this may join regions that
 *	are not coplanar.  If region_grow_snap() is called after this
 *	call, it may dramatically change the geometry of the mesh.
 *
 * arguments:
 *
 * 	rl -	The list of regions to identify and modify
 *
 * 	min_reg_size -	The minimum number of triangles in a region for
 * 			it to be preserved in the list.
 */
void region_grow_coalesce(vector<planar_region_t>& rl, 
				unsigned int min_reg_size);

/* region_grow_snap_verts:
 *
 *	Finds all vertices that are on the boundary of multiple
 *	planar regions, and snaps their positions to the best fit
 *	for all regions involved.
 *
 * arguments:
 *
 * 	rl -	The planar regions to check, these should already be
 * 		grown using region_grow_all().
 *
 * 	tri -	The triangulation that was the input to the region growing
 * 		process, and contains the vertices to modify.
 */
void region_grow_snap_verts(vector<planar_region_t>& rl, 
						triangulation_t& tri);

/* region_grow_snap:
 *
 * 	Uses the specified regions to snap vertices to planar models. 
 *	
 * arguments:
 *
 * 	rl -	The regions to use
 */
void region_grow_snap(vector<planar_region_t>& rl);

/* color_by_region:
 *
 *	Changes vertex colors in tri to be coordinate with regions.
 *
 * arguments:
 *
 * 	rl -	The regions to use to color
 */
void color_by_region(vector<planar_region_t>& rl);

/* prune_invalid_triangles_from_regions:
 *
 * 	Removes references to triangles that are not in the specified
 * 	triangulation.
 *
 *	After computation, the triangulation will be indexed.
 * 
 * arguments:
 * 
 * 	tri -	The triangulation to consider.
 */
void prune_invalid_triangles_from_regions(vector<planar_region_t>& pl,
					triangulation_t& tri);

#endif
