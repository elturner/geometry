#ifndef STRUCTS_MESHER_H
#define STRUCTS_MESHER_H

/* mesher.h:
 *
 * Will convert from the boundary of
 * voxels to a triangulated mesh whose
 * elements are sized based on the planarity
 * of the geometry at that location.
 */

#include <vector>
#include <set>
#include "dgrid.h"
#include "normal.h"
#include "point.h"
#include "../util/parameters.h"

using namespace std;

/* the following are the classes defined in this file. */
class face_t;
class face_state_t;
class vertex_state_t;
class region_t;
class mesher_t;

/* the following class represents a face along the
 * voxel lattice in space. This face adjoins two
 * voxels. */
class face_t
{
	/*** parameters ***/
	public:

	/* This face adjoins two voxels,
	 * the following elements represents one of
	 * these voxels and which face index
	 * this face_t represents for it */
	voxel_t v;
	int f;

	/*** functions ***/
	public:

	/* constructors */
	face_t();
	face_t(voxel_t& vv, int ff);
	~face_t();
	
	/* set this face's parameters to the other face's parameters */
	void copy(face_t& other);

	/* swap this face's parameters with the other face's */
	void swap(face_t& other);

	/* comparisons */
	inline bool operator <  (const face_t& rhs) const
	{
		return (this->v < rhs.v) 
			|| (this->v == rhs.v && this->f < rhs.f);
	};
	inline bool operator == (const face_t& rhs) const
	{
		return (this->v == rhs.v && this->f == rhs.f);
	};
	inline bool operator != (const face_t& rhs) const
	{
		return (this->v != rhs.v || this->f != rhs.f);
	};

	/* geometry */

	/* get_center:
	 *
	 * 	Copies the location of the center of
	 * 	this face into the point object provided.
	 *
	 * 	Note that the center value will contain integer
	 * 	or half-integer coordinates.  It assumes voxel
	 * 	size is one.
	 *
	 * arguments:
	 *
	 * 	p -	Where to store the center point of this face
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int get_center(point_t& p) const;

	/* faces_outward:
	 *
	 * 	Returns false if the normal vector provided faces into
	 * 	the face.
	 *
	 * 	Returns false if the normal vector is orthogonal to the
	 * 	face's normal vector.
	 *
	 * 	Returns true if the normal vector provided points out of
	 * 	the face.
	 */
	bool faces_outward(normal_t& n);
};

/* This class represents the properties that a face_t can
 * have.  Since we are finding planar regions, for example,
 * each face needs to have a region identification number. */
class face_state_t
{
	/*** properties ***/
	public:

	/* The region identification number.  All faces with the
	 * same number are part of the same planar region. */
	int region_id;

	/* Neighborhood.  Any face that shares an edge with this
	 * face.  There should be exactly four such faces.
	 *
	 * 		    1
	 *		    |
	 *		2 --o-- 0
	 *		    |
	 *		    3
	 */
	face_t neighbors[NUM_EDGES_PER_SQUARE];

	/*** functions ***/

	/* constuctors */
	face_state_t();
	~face_state_t();
	void init();
};

/* The vertex state type defines attributes of a vertex in a mesh.
 * These vertices correspond to corners of voxels, though their
 * geometrical position may change. */
class vertex_state_t
{
	/*** properties ***/
	public:

	/* The position of a vertex in space is not discretized */
	point_t p;

	/* each vertex may be incident to any number of planar
	 * regions.  This list represents the indices of all
	 * these planar regions */
	set<int> reg_inds;

	/*** functions ***/

	/* constructors */
	vertex_state_t(voxel_t& v); /* the voxel defines the
					position of vert */
	~vertex_state_t();
};	

/* This class represents a set of faces that compose a region */
class region_t
{
	/*** parameters ***/
	public:

	/* the set of faces that make up this region */
	set<face_t> faces;

	/* Since a region represents a subset of a plane, that plane
	 * should be defined by a point and a normal vector */
	normal_t norm;
	point_t pos;

	/* since the faces on a region aren't all perfectly aligned with
	 * the plane of the region, there is some error of vertices away
	 * from the plane of the region.  The following is the maximum
	 * deviation from the plane defined above */
	double max_err;

	/* Each region also shares edges with other regions, so
	 * this set describes this region's neighbors, via indices */
	set<int> neighbors;

	/*** functions ***/
	public:

	/* constructors */
	region_t(face_t& seed);
	~region_t();

	/* initialization */

	/* find_center:
	 *
	 * 	Sets the parameter pos to the center position
	 * 	of this region.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int find_center();

	/* find_dominant_face:
	 *
	 * 	Returns the voxel face direction that best
	 * 	matches the normal direction of this region.
	 *
	 * 	In other words, uses the principle component of
	 * 	the normal to estimate a voxel face.
	 *
	 * return value:
	 *
	 * 	Returns voxel face define (e.g. VOXEL_FACE_XPLUS)
	 */
	int find_dominant_face();

	/* verify_normal:
	 *
	 *	Will potentially flip the direction of the normal
	 *	vector, based on analysis of the elements of the
	 *	faces set.
	 */
	void verify_normal();

	/* find_inf_radius:
	 *
	 *	Computes the radius of this region, from the
	 *	center point of this region, in the L-infinity
	 *	norm.
	 *
	 *	That is, will compute the size of a bounding cube
	 *	to this region.
	 *
	 * return value:
	 *
	 * 	Returns radius.  On error, returns negative value
	 */
	double find_inf_radius();

	/* height_of_point:
	 *
	 * 	Computes the signed height of the given point off of
	 * 	the plane of this region.
	 */
	double height_of_point(point_t& p);

	/* height_of_voxel:
	 *
	 *	Computes the signed height of the given voxel off of
	 *	the plane specified by this region.
	 */
	double height_of_voxel(voxel_t& v);
};

/* This class represents the surface of a voxelized volume.  This
 * differs from the dgrid_t class, since each face is stored explicitly */
class mesher_t
{
	/*** parameters ***/
	public:

	/* This structure represents all faces in this
	 * surface.  These faces should form a watertight
	 * boundary, and all be oriented so that the defining
	 * voxel is outside of the volume */
	map<face_t, face_state_t> graph; /* graph of faces */

	/* each region is represented by a subset of the faces */
	vector<region_t> regions;

	/* Each corner of a voxel may be connected to multiple regions.
	 * These corners will eventually become the vertices of a
	 * triangulation, so it is necessary to determine which regions
	 * a vertex is incident to. */
	map<voxel_t, vertex_state_t> verts;

	/*** functions ***/
	public:

	/* constructors */
	mesher_t();
	~mesher_t();

	/* init:
	 *
	 * 	initialization from existing dgrid
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int init(dgrid_t& dg);

	/* region_flood_fill:
	 *
	 * 	Once the mesher has been initialized with
	 * 	faces, this function will associate faces which
	 * 	form a single planar region with one another by
	 * 	performing a flood fill algorithm.
	 *
	 * 	This function assumes no region information has
	 * 	been defined yet.
	 */
	int region_flood_fill();

	/* coalesce_regions:
	 *
	 * 	Will combine the regions described in this mesher object,
	 * 	stopping only when there is no possible coalescing that
	 * 	will keep a region error under 1.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int coalesce_regions();

	/* coalesce_regions_lax:
	 *
	 * 	Will perform much the same operation as coalesce_regions(),
	 * 	but the metric for judging when to merge to regions is
	 * 	much looser.  Recommended to perform stricter coalescing,
	 * 	then only use this function if necessary afterwards.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int coalesce_regions_lax();

	/* reassign_degenerate_regions:
	 *
	 *	After coalescing, there may be some planar regions that
	 *	are in configurations that may become degenerate.
	 *
	 *	Example:  if a mostly vertical region is completely
	 *	surrounded by a mostly horizontal region.
	 *
	 *	This function will reduce the chance of such occurrances
	 *	by reassigning faces from some regions to others.
	 *
	 *	The number of regions will not change.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int reassign_degenerate_regions();

	/* coalesce_regions_small:
	 *
	 * 	Will find small regions, and attempt to merge them into
	 * 	larger neighboring regions.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int coalesce_regions_small();

	/* reassign_boundary_faces:
	 *
	 *	Will search through all boundary faces in this mesh,
	 *	and determine which region results in a smaller error
	 *	on the face.  If the face's current region is not the
	 *	smallest, will remove the face from that region and add
	 *	it to the better region.
	 *
	 *	Will do this iteratively until steady-state is reached.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int reassign_boundary_faces();

	/* compute_verts:
	 *
	 *	Given that regions have been defined for this
	 *	mesh, will store all face corners as vertices, recording
	 *	which regions are incident to each vertex.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int compute_verts();

	/* write_to_obj:
	 *
	 * 	Writes the faces to the specified obj file,
	 * 	coloring each face by region id.
	 *
	 * 	Useful for debugging.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int write_to_obj(char* filename);

	/*** helper functions ***/

	/* find_neighbors_for:
	 *
	 * 	Records the neighboring faces for the given face.
	 *
	 * arguments:
	 *
	 * 	f -	Face to analyze
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int find_neighbors_for(face_t& f);
	
	/* compute_neighbors_of:
	 *
	 *	Given the index of a region in this mesh, will compute
	 *	what other regions are neighboring r, and will store
	 *	those indices in regions[r].neighbors.
	 *
	 *	Assumes the boundary_faces set has been initialized, and
	 *	that all faces point to correct regions.
	 *
	 * arguments:
	 *
	 *	r -	The index of the region to analyze.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int compute_neighbors_of(int r);

	/* coalesce_properties:
	 *
	 * 	Will compute the characteristics of the resulting region
	 * 	if the two input regions were coalesced.  Specifically, will
	 * 	determine the position and norm of the best-fit plane, and
	 * 	determine the maximum error if that plane were used.
	 *
	 *	Assumes input regions have well defined center points,
	 *	norms, faces sets, and boundary_faces sets.
	 *
	 * arguments:
	 *
	 * 	ra, rb -	The regions to analyze
	 * 	center -	Where to store the center point of best-fit
	 * 			plane
	 * 	norm -		The normal of the best-fit plane to the
	 * 			union of the two regions
	 * 	err -		The maximum distance error of an element
	 * 			to the best-fit plane.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int find_combined_properties(region_t& ra, region_t& rb, 
					point_t& center, normal_t& norm,
					double& err);

	/* merge_regions:
	 *
	 *	Merges the two regions r1 and r2 into a new region,
	 *	with the planar attributes provided.
	 *
	 * arguments:
	 *
	 * 	r1, r2 -	The indices of the two regions to merge
	 *
	 * 	p, norm, err -	The attributes of the plane of the merged
	 * 			region.  These should have been calculated
	 * 			using find_combined_properties().
	 *
	 * return value:
	 *
	 * 	On success, returns the index of the merged region.
	 * 	On failure, returns negative value.
	 */
	int merge_regions(int r1, int r2,
			point_t& p, normal_t& norm, double err);

	/* face_is_degenerate:
	 *
	 * 	Returns non-negative iff the region labels for this 
	 * 	face and its neighbors result in a degenerate condition.
	 *
	 * return value:
	 *
	 * 	If the face is degenerate, returns the index of the
	 * 	region to add this face to.
	 *
	 * 	If the face is not degenerate, returns negative value.
	 */
	int face_is_degenerate(face_t& f);

	/* face_is_boundary:
	 *
	 *	Returns true iff the given face is a boundary face
	 *	for its region.
	 */
	bool face_is_boundary(face_t& f);

	/* project_vertex:
	 *
	 *	Given a vertex, will reposition it so that it best
	 *	aligns with the planar regions referenced.
	 *
	 *	This projection is bounded, in that the position of
	 *	the vertex will be perturbed no more than one grid
	 *	spacing.
	 *
	 * return value:
	 *
	 * 	Returns Non-negative number on success, denoting how
	 * 	many planes the point was projected onto.
	 *
	 * 	Returns negative value on failure.
	 */
	int project_vertex(vertex_state_t& v);
	
	/* point_axis_projected_to:
	 *
	 * 	Given a direction, will project the center point of
	 * 	this face onto the 2D subspace along the direction
	 * 	provided.
	 *
	 * arguments:
	 *
	 * 	u,v -	Where to store the 2D coordinates in the
	 * 		projected subspace
	 *
	 *	p -	The point to project.
	 *
	 * 	fn -	The voxel face number that represents the
	 * 		subspace (this can be one of six axis-aligned
	 * 		signed directions).
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int point_axis_projected_to(double& u, double& v,
						point_t& p, int fn); 

	/* undo_point_axis_projection:
	 *
	 *	Given the 2D discretized coordinates of a point on
	 *	a axis-aligned subspace, will position that point in
	 *	discretized 3D space.
	 *
	 * arguments:
	 *
	 * 	p -	Where to store the discretized 3D result
	 *
	 * 	u,v -	The 2D coordintes on the specified subspace
	 *
	 * 	fn,c -	The direction and position that defines the
	 * 		axis-aligned subspace.  u,v are defined so
	 * 		that c is their origin.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int undo_point_axis_projection(voxel_t& p, int u, int v,
						int fn, voxel_t& c);

	/* undo_plane_projection:
	 *
	 *	Given a voxel in space, will find the point p on
	 *	the planar region r that results from moving vp in
	 *	the fn direction.
	 *
	 * arguments:
	 *
	 * 	p -	Where to store the projected point onto the plane
	 * 	
	 * 	vp -	The discretized point to project
	 * 	
	 * 	r -	Index of the region that defines the plane on which 
	 * 		to project.
	 *
	 *	fn -	The voxel face number that defines the direction
	 *		of projection.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int undo_plane_projection(point_t& p, voxel_t& vp, 
					int r, int fn);
};

#endif
