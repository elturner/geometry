#ifndef DGRID_H
#define DGRID_H

/* dgrid.h:
 *
 * A volume-defining data-structure that stores the
 * boundary voxels between solid and empty space.  The
 * boundary voxels are considered solid.
 */

#include <map>
#include <set>
#include <vector>
#include "point.h"
#include "pose.h"
#include "../util/parameters.h"

using namespace std;

/* The voxel_t class defines the coordinates of a voxel
 * in space, and a sorting for these coordinates. */
class voxel_t
{
	/* parameters */
	public:

	/* the following parameters define the coordinates of
	 * this voxel.  These are the indices in 3-space of this
	 * voxel. */
	int x_ind;
	int y_ind;
	int z_ind;

	/* functions */
	public:

	/* constructors */

	/* the following returns garbage voxel */
	voxel_t() {};

	/* the following returns the voxel at the given indices */
	voxel_t(int xi, int yi, int zi);

	/* the following constructor returns the voxel at the given
	 * continuous coordinates, assuming the given voxel size */
	voxel_t(double x, double y, double z, double vs);

	/* destructor unnecessary */
	~voxel_t() {};
	
	/* overloaded operators */

	/* Since voxels are stored in a treemap, they must be sortable.
	 * This sorting is done by overloading the less-than operator */
	inline bool operator < (const voxel_t& rhs) const
	{
		/* check x-dimension */
		if(this->x_ind < rhs.x_ind)
			return true;
		if(this->x_ind > rhs.x_ind)
			return false;
	
		/* check y-dimension */
		if(this->y_ind < rhs.y_ind)
			return true;
		if(this->y_ind > rhs.y_ind)
			return false;

		/* check z-dimension */
		if(this->z_ind < rhs.z_ind)
			return true;
		return false;
	};
	inline bool operator == (const voxel_t& rhs) const
	{
		/* two voxels are at equal position iff each coordinate
		 * dimension is equal */
		return (this->x_ind == rhs.x_ind)
				&& (this->y_ind == rhs.y_ind)
				&& (this->z_ind == rhs.z_ind);
	};
	inline bool operator != (const voxel_t& rhs) const
	{
		/* two voxels are at equal position iff each coordinate
		 * dimension is equal */
		return (this->x_ind != rhs.x_ind) 
				|| (this->y_ind != rhs.y_ind)
				|| (this->z_ind != rhs.z_ind);
	};

	/* accessors */
	
	/* the following function sets the location of this voxel */
	void set(int xi, int yi, int zi);
	void set(double x, double y, double z, double vs);

	/* set_to_mirror:
	 *
	 *	Sets this voxel to be at the position of the 
	 *	neighboring voxel of v pointed to by face f.  That
	 *	is, this voxel will be the voxel on the other side
	 *	of the f'th face of v.
	 *
	 * arguments:
	 *
	 * 	v -	The voxel to mirror
	 * 	f -	The face of v to reflect across.
	 *
	 * return value:
	 *
	 * 	Returns the face of this voxel one whose other side is v.
	 * 	That is, if we call:
	 *
	 * 		f_b = b.set_to_mirror(a, f_a);
	 * 		f_c = c.set_to_mirror(b, f_b);
	 *
	 * 	Then f_c == f_a and a == c.
	 */
	int set_to_mirror(voxel_t& v, int f);

	/* geometry */

	/* intersects_segment:
	 *
	 * 	Returns true iff the specified segment intersects this
	 *	voxel at the specified face.
	 *
	 * arguments:
	 *
	 * 	p,s -	The endpoints of the segment to analyze
	 * 	f -	The face number to check
	 * 	vs -	The voxel-size to use in computations
	 *
	 * return value:
	 *
	 * 	Returns true iff intersection occurs
	 */
	bool intersects_segment_at_face(point_t& p, point_t& s,
						int f, double vs);

	/* get_center:
	 *
	 *	Sets the value of c to be the center of this voxel,
	 *	at the given voxel size.
	 *
	 * arguments:
	 *
	 * 	c -	Where to store the location of voxel center.
	 * 	vs -	The length of an edge of the voxel.
	 */
	void get_center(point_t& c, double vs);
};

/* a voxel_state_t is the value in the mapping at each voxel.  This
 * value is non-zero only at boundary solid voxels.  If non-zero, this
 * value also denotes which faces of the boundary voxel are connected
 * to inside 'empty' voxels. */
typedef unsigned char voxel_state_t;

/* the following macros will determine if a particular face
 * is boundary face within a boundary voxel, based on the voxel_state_t.
 *
 * arguments:
 *
 * 	v -	The voxel state to analyze
 * 	i -	The face to analyze
 */
#define VOXEL_IS_FACE_BIT_INWARD(v, i)   ( ((v) >> (i)) & 1 )
#define VOXEL_GET_FACE_BIT(i)            ( 1 << (i) )
#define VOXEL_SET_FACE_BIT_INWARD(v, i)  ( (v) |=   VOXEL_GET_FACE_BIT(i)  )
#define VOXEL_SET_FACE_BIT_OUTWARD(v, i) ( (v) &= ~(VOXEL_GET_FACE_BIT(i)) )

/* The following values define the enumeration of each face value
 *
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
 */
#define VOXEL_FACE_YMINUS 0
#define VOXEL_FACE_XPLUS  1
#define VOXEL_FACE_YPLUS  2
#define VOXEL_FACE_XMINUS 3
#define VOXEL_FACE_ZMINUS 4
#define VOXEL_FACE_ZPLUS  5

/* The following denotes the state of a voxel that is not at the boundary */
#define VOXEL_STATE_NONBOUNDARY ((voxel_state_t) 0)

/* the following array defines the relative positions of each voxel
 * corner to one another, by giving the x,y,z displacement of each corner
 * from the min. corner of the voxel (i.e. corner #0) */
static const int voxel_corner_pos[NUM_CORNERS_PER_CUBE][NUM_DIMS] = {
/* corner #0 */	{0,0,0},
/* corner #1 */	{1,0,0},
/* corner #2 */	{1,1,0},
/* corner #3 */	{0,1,0},
/* corner #4 */	{0,0,1},
/* corner #5 */	{1,0,1},
/* corner #6 */	{1,1,1},
/* corner #7 */	{0,1,1}
};

/* the following array also specifies corner positions, but relative to
 * voxel faces.  Each row represents a face, with the entries being
 * the four corners of that face */
static const int 
	voxel_corner_by_face[NUM_FACES_PER_CUBE][NUM_VERTS_PER_SQUARE] = {
/* 0: face y-minus */ {0,1,5,4},
/* 1: face x-plus  */ {1,2,6,5},
/* 2: face y-plus  */ {2,3,7,6},
/* 3: face x-minus */ {0,4,7,3},
/* 4: face z-minus */ {0,3,2,1},
/* 5: face z-plus  */ {4,5,6,7}
};

/* the following array specifies which voxel faces intersect which corners.
 * Each row represents a corner, and the elements of that row denote
 * the indices of the faces that corner intersects. */
static const int voxel_face_by_corner[NUM_CORNERS_PER_CUBE]
				[NUM_FACES_PER_CORNER_PER_CUBE] = {
/* corner #0 */ {0,3,4},
/* corner #1 */ {0,1,4},
/* corner #2 */ {2,1,4},
/* corner #3 */ {2,3,4},
/* corner #4 */ {0,3,5},
/* corner #5 */ {0,1,5},
/* corner #6 */ {2,1,5},
/* corner #7 */ {2,3,5},
};

/* The following array allows for traversal between corners of a cube.
 * Each row reprents a corner of the cube.  The first three elements of
 * each row represnt the faces of the cube that are disjoint from this
 * corner.  The next three elements represent the corner on each of
 * those faces that connected via an edge to the corner in question. 
 *
 * So the first three elements are face numbers.  The next three elements
 * are corner numbers. */
static const int voxel_corner_traversal_table[NUM_CORNERS_PER_CUBE][6] = {
/*               faces   corners */
/* corner #0 */ {1,2,5,  1,3,4},
/* corner #1 */ {2,3,5,  2,0,5},
/* corner #2 */ {0,3,5,  1,3,6},
/* corner #3 */ {0,1,5,  0,2,7},
/* corner #4 */ {1,2,4,  5,7,0},
/* corner #5 */ {2,3,4,  6,4,1},
/* corner #6 */ {0,3,4,  5,7,2},
/* corner #7 */ {0,1,4,  4,6,3}
};

/* The dgrid class represents a carved grid in 3D space. It stores only
 * the solid boundary voxels around the carved volume.  Each voxel is a
 * cube of side-width vs. */
class dgrid_t
{
	/* parameters */
	public:
	
	/* vs represents the resolution of the grid.  Each voxel
	 * is a cube of this size.  The thinnest wall guaranteed to be
	 * represented in the grid must be of thickness greater than 2*vs */
	double vs; /* voxel size */

	/* the voxels stored are those solid boundary voxels, which are
	 * stored sparsely in a treemap */
	map<voxel_t, voxel_state_t> voxels;

	/* The following set also defines voxel properties.  This one
	 * dictates which voxels contain points from the original input
	 * pointcloud.  The act of populating of this voxel is an optional
	 * operation, so it may be empty. */
	set<voxel_t> points;

	/* member functions */
	public:

	/* constructor, initializes grid where every location in space is
	 * solid.  The argument v represents voxel-size to use. */
	dgrid_t() {}; /* default constructor gives garbage voxel size */
	dgrid_t(double v);
	~dgrid_t() {}; /* stl-containers can destruct themselves */

	/* accessors */

	/* init:
	 *
	 * 	Initializes a completely solid grid with specified voxel
	 * 	size.
	 */
	void init(double v);

	/* clear:
	 *
	 * 	Clear all information from this grid.
	 */
	void clear();

	/* populate_points_from_xyz:
	 *
	 * 	Uses the specified pointcloud file to populate the
	 * 	voxels located in the points field.
	 *
	 * arguments:
	 *
	 * 	filename -		The *.xyz formatted file to read the
	 * 				point-cloud information from.
	 * 	
	 * 	pl -			The pose-list associated with this
	 * 				pointcloud.
	 *
	 * 	range_limit_sq -	The square of the maximum distance
	 * 				a point can be from its 
	 * 				corresponding pose for it to be
	 * 				included in the voxelized points
	 * 				set.  Any points with a greater
	 * 				range than this will be ignored.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int populate_points_from_xyz(char* filename, vector<pose_t>& pl, 
						double range_limit_sq);

	/* get_voxel_state:
	 *
	 * 	Returns the state at the current voxel.  A state of
	 * 	0 indicates that this voxel is either inside or outside,
	 * 	but not a border voxel.
	 *
	 * arguments:
	 *
	 * 	v -	The location of the voxel of which to return the
	 * 		state
	 *
	 * return value:
	 *
	 * 	Returns the state value at the specified voxel.
	 */
	voxel_state_t get_voxel_state(voxel_t& v);

	/* The following represent carving functions, which are used
	 * to denote subsets of the voxelized space as empty, or 'inside'*/

	/* carve_voxel:
	 *
	 * 	Carves a single voxel in the dgrid.  Will only have an
	 * 	effect if the specified voxel is a boundary voxel.  Thus
	 * 	this operation is only valid for interior or boundary
	 * 	voxels.
	 *
	 * arguments:
	 *
	 * 	v -	The location of the voxel of which to carve.
	 */
	void carve_voxel(voxel_t& v);

	/* fill_voxel:
	 *
	 * 	Fills a single voxel in the dgrid, converting it from
	 * 	an empty 'inside' voxel to a solid voxel.  Note that filling
	 * 	must be done incrementally, so it is only valid to fill
	 * 	voxels that are neighboring boundary voxels.
	 *
	 * arguments:
	 *
	 * 	v -	The voxel location to fill.  v must be neighboring
	 * 		a boundary voxel, and must be an interior voxel.
	 */
	void fill_voxel(voxel_t& v);

	/* carve_segment:
	 *
	 * 	Carves all voxels that are intersected by the line segment
	 * 	p to s.  Since carving is incremental, the point p must be
	 * 	contained in either an internal or boundary voxel.
	 *
	 * arguments:
	 *
	 * 	p, s -	The endpoints of the segment to carve.  p must be
	 * 		contained in an inside or boundary voxel for
	 * 		successful carving.
	 *
	 * 	force -	If true, will carve the segment without any regard
	 * 		to the contents of this->points.  If false, will
	 * 		only carve the portion of the segment from p
	 * 		up to the first voxel that intersects a point-voxel
	 * 		(exclusive).
	 */
	void carve_segment(point_t& p, point_t& s, bool force);

	/* remove_outliers:
	 *
	 *	Examines the final grid, checking for grid cells that
	 *	are isolated or determined to be outliers.
	 * 
	 * 	Any outliers that are found are removed.
	 */
	void remove_outliers();

	/* write_to_obj:
	 *
	 * 	Exports this grid to disk, in the form of a naively
	 * 	constructed object file.
	 *
	 * 	This function useful for debugging.
	 *
	 * arguments:
	 *
	 * 	filename -	Where to write file.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int write_to_obj(char* filename);

	/* helper functions */
	protected:

	/* set_voxel_state:
	 *
	 * 	Records the voxel at location v to be state s.
	 */
	void set_voxel_state(voxel_t& v, voxel_state_t s);
};

#endif
