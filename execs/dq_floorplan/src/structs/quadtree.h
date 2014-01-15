#ifndef QUADTREE_H
#define QUADTREE_H

/* this file defines a quadtree structure.
 * The quadtree represents all of 2D space, and
 * the bounding box grows as more elements are added.
 */

#include <ostream>
#include <istream>
#include <set>
#include <vector>
#include "point.h"
#include "normal.h"
#include "parameters.h"

using namespace std;

/* these classes are defined in this file */
class quadtree_t;
class quadnode_t;
class quaddata_t;

/* defines the quadtree class */
class quadtree_t
{
	/*** parameters ***/
	private:

	/* root of the tree and its relative position */
	quadnode_t* root;

	/* the tree expands down some max depth */
	int max_depth;

	/*** function ***/
	public:

	/* constructors */
	quadtree_t();
	quadtree_t(double r); /* pass the max depth via grid res */
	~quadtree_t();

	/* accessors */

	/* set_resolution:
	 *
	 * 	Sets the resolution to be the argument.
	 * 	Will destroy any information in tree.
	 */
	void set_resolution(double r);

	/* get_resolution:
	 *
	 * 	Returns the resolution of the max depth of the tree.
	 */
	double get_resolution();

	/* clear:
	 *
	 * 	Clears all information from tree.  set_resolution
	 * 	must be called before adding more data.
	 */
	void clear();

	/* clone_from:
	 *
	 * 	Given a reference to another quadtree, will destroy
	 * 	information in this quadtree, and make a deep copy
	 * 	of the reference.
	 *
	 * arguments:
	 *
	 * 	other -	The other quadtree of which a deep copy will
	 * 		be made and stored in this quadtree.
	 */
	void clone_from(const quadtree_t& other);

	/* insert:
	 *
	 * 	Given a point in space, will update the data
	 * 	structure in the correct leaf node with this
	 * 	point.
	 *
	 * arguments:
	 *
	 * 	p -	The point to add to the structure
	 *
	 * return value:
	 *
	 * 	On success, returns pointer to the data p was incorporated
	 * 	into.
	 * 	On failure, returns NULL.
	 */
	quaddata_t* insert(point_t& p);

	/* insert:
	 *
	 * 	Given a point and its corresponding normal,
	 * 	will insert the point and update that quaddata's
	 * 	normal vector.
	 *
	 * return value:
	 *
	 * 	Returns the appropriate data on success.
	 * 	Returns NULL on error.
	 */
	quaddata_t* insert(point_t& p, normal_t& n);

	/* insert:
	 *
	 * 	Given a point, its normal, and its pose index, will
	 * 	insert these values into the appropriate node and
	 * 	return the corresponding data.
	 *
	 * return value:
	 *
	 * 	Returns the appropriate data on success.
	 * 	Returns NULL on error.
	 */
	quaddata_t* insert(point_t& p, normal_t& n, unsigned int pose_ind);

	/* retrieve:
	 *
	 * 	Given a point, will return the quaddata at
	 * 	the deepest node that contains that point.  If no data
	 * 	exists there, NULL is returned.
	 *
	 * return value:
	 *
	 * 	Returns the appropriate data on success.
	 * 	Returns NULL if p out of current range of tree.
	 */
	quaddata_t* retrieve(point_t& p);
	
	/* geometry */

	/* nearest_neighbor:
	 *
	 *	Returns the quaddata that is
	 *	closest to the given point.
	 *
	 *	If all nodes are empty, returns null.
	 */
	quaddata_t* nearest_neighbor(point_t& p);
	
	/* neighbors_in_range:
	 *
	 *	Will find all non-empty nodes within distance r
	 *	to the point p.
	 *
	 *	The resulting list of neighbors will not be sorted.
	 *
	 * arguments:
	 *
	 * 	p -		The query point.
	 *
	 * 	r -		The distance from p to check.
	 * 			If r < 0, will assume infinite range.
	 *
	 * 	neighs -	Will insert any relevent quaddatas into
	 * 			this structure.  It is not necessary
	 * 			for this structure to be empty upon
	 * 			call, new elements will be appended.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int neighbors_in_range(point_t& p, double r,
			vector<quaddata_t*>& neighs);

	/* raytrace:
	 *
	 * 	Will determine which data elements intersect
	 * 	the specified line segment, and add them to 
	 * 	the specified list.
	 *
	 * arguments:
	 *
	 * 	xings -	Where to store any data that intersect the line
	 * 		segment.
	 * 	a,b -	Two points that define a line segment to use
	 * 		for ray-tracing.
	 */
	void raytrace(vector<quaddata_t*>& xings, point_t& a, point_t& b);

	/* i/o */

	/* print:
	 *
	 * 	Prints a representation of this tree to the
	 * 	specified ascii stream.  The tree can be
	 * 	perfectly reconstructed by using this information.
	 *
	 * format:
	 *
	 * 	<max_depth>
	 * 	<root_halfwidth>
	 * 	<root_center_x> <root_center_y>
	 * 	<data1>
	 * 	<data2>
	 * 	...
	 *
	 * 	Where <datai> indicates a line that was printed by
	 * 	a quaddata_t struct.
	 */
	void print(ostream& os);

	/* parse:
	 *
	 *	Reads from the specified stream and creates the
	 *	tree described.  This will destroy any existing data.
	 *
	 *	Assumes the content of the stream is formatted in the
	 *	same manner as quadtree_t::print().
	 *
	 * return value:
	 *
	 *	Returns 0 on success, non-zero on failure.
	 */
	int parse(istream& is);

	/* debugging */

	/* print_points:
	 *
	 *	Prints a list of all the data locations,
	 *	separated by newlines.
	 *
	 * arguments:
	 *
	 * 	os -	The output stream to which to print
	 */
	void print_points(ostream& os);

	/* print_nodes:
	 *
	 *	Prints each node in this tree.  Each node
	 *	is represented by its own line in the following
	 *	format:
	 *
	 *	<x> <y> <halfwidth>
	 *
	 * arguments:
	 *
	 * 	os -	The output stream to which to print.
	 */
	void print_nodes(ostream& os);
};

/* defines the individual nodes of a quadtree */ 
class quadnode_t
{
	/*** parameters ***/
	public:

	/* each node has pointers to its children.
	 * These pointers being null implies this
	 * node is a leaf. 
	 *
	 *
	 *              |
	 *       1      |      0
	 *              |
	 * -------------+--------------
	 *              |
	 *       2      |      3
	 *              |
	 */
	quadnode_t* children[CHILDREN_PER_NODE];

	/* quadnodes have geometry, such as center position
	 * and size.  The position is relative to the origin
	 * of the tree. */
	point_t center;
	double halfwidth; /* distance from center to edge */

	/* each node also stored data elements */
	quaddata_t* data; /* only non-null for leaves */

	/*** functions ***/
	public:

	/* constructors */
	quadnode_t();
	quadnode_t(point_t& c, double hw);
	~quadnode_t();

	/* accessors */

	/* isleaf:
	 *
	 * 	Returns true iff this node is a leaf.
	 */
	bool isleaf();

	/* isempty:
	 *
	 * 	Returns true iff this node is empty.
	 */
	inline bool isempty();

	/* init_child:
	 *
	 * 	Will initialize the i'th child of this node,
	 * 	assuming it is not already initialized.
	 */
	void init_child(int i);

	/* clone:
	 *
	 * 	Allocates new memory that is a deep copy of
	 * 	this node and its subnodes.
	 */
	quadnode_t* clone();

	/* geometry */

	/* contains:
	 *
	 * 	Returns true iff the given point resides inside
	 * 	this node.
	 */
	inline bool contains(point_t& p);

	/* child_contains:
	 *
	 *	Returns which child would contain p.  Note
	 *	that p may be out of bounds of this node.  The
	 *	check only considers which quadrant p is in with
	 *	respect to the center of this node.
	 *
	 * return value:
	 * 	
	 * 	Returns the index of the child in this node that
	 * 	is closest to p.  This child may be null.
	 */
	inline int child_contains(point_t& p);

	/* intersects_line_segment:
	 *
	 * 	Returns true iff this node is intersected
	 * 	by the line segment defined by the two points
	 * 	given.
	 */
	bool intersects_line_segment(point_t& a, point_t& b);

	/* recursive calls */

	/* insert:
	 *
	 * 	Will insert the given point into this node
	 * 	or one of its children.  Will force the
	 * 	insertion to the specified relative depth,
	 * 	creating new children if necessary.
	 *
	 * arguments:
	 *
	 *	p -	The point to add to the tree.
	 *
	 *	d -	The relative depth at which to add
	 *		this point.  Will add to the current
	 *		level if d=0.
	 *
	 * return value:
	 *
	 * 	Returns a pointer to the leaf node that p was
	 * 	added to.  Returns NULL on error.
	 */
	quaddata_t* insert(point_t& p, int d); 

	/* retrieve:
	 *
	 * 	Will return the leaf node that contains the
	 * 	given point.
	 *
	 * arguments:
	 *
	 * 	p -	The query point.
	 *
	 * return value:
	 *
	 * 	Returns node that is a leaf and contains p.
	 * 	Returns NULL on error.
	 */
	quadnode_t* retrieve(point_t& p);

	/* nearest_neighbor:
	 *
	 *	Will compare this node and its children to their
	 *	proximity to point p compared to the best quadnode
	 *	seen so far.
	 *
	 *	If this node or one of its decendents are closer to
	 *	p, then the value of best_so_far will be updated.
	 *
	 * arguments:
	 *
	 * 	best_so_far -	If null, will insert a non-empty
	 * 			decendent that is close to p.
	 * 			If non-null, will only insert
	 * 			decendents that are closer than the
	 * 			current value is to p.
	 *
	 * 	p -		The query point.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int nearest_neighbor(quaddata_t** best_so_far, point_t& p);	

	/* nodes_in_range:
	 *
	 * 	Searches this node and all its decendants for all
	 * 	quaddatas that fall within distance r of p.
	 *
	 * 	Stores qualifying pointers in provided list.
	 *
	 * arguments:
	 *
	 * 	p -		The query point
	 * 	r -		The distance to query point.  If r < 0,
	 * 			will use infinite range.
	 * 	neighs -	The list to modify with candidate
	 * 			quaddatas
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int nodes_in_range(point_t& p, double r,
			vector<quaddata_t*>& neighs);

	/* raytrace:
	 *
	 *	Will ray trace between the points provided.  If
	 *	any data under this node intersect this line segment,
	 *	they will be inserted into the specified list.
	 *
	 * arguments:
	 *
	 * 	xing -	Where to store data that intersect the line segment
	 * 	a,b -	The endpoints of a line segment to raytrace.
	 *
	 */
	void raytrace(vector<quaddata_t*>& xings, point_t& a, point_t& b);

	/* i/o */

	/* print:
	 *
	 * 	Prints the data under this node to the
	 * 	specified ascii stream.
	 */
	void print(ostream& os);

	/* debugging */
	
	/* print_points:
	 *
	 *	Prints a list of all the data locations,
	 *	separated by newlines.
	 *
	 * arguments:
	 *
	 * 	os -	The output stream to which to print
	 */
	void print_points(ostream& os);

	/* print_nodes:
	 *
	 *	Prints each node in this tree.  Each node
	 *	is represented by its own line in the following
	 *	format:
	 *
	 *	<x> <y> <halfwidth>
	 *
	 * arguments:
	 *
	 * 	os -	The output stream to which to print.
	 */
	void print_nodes(ostream& os);
};

/* this class represents the data that are stored in the nodes of
 * the quad tree.  This is only interesting at the leaves. */
class quaddata_t
{
	/*** security ***/
	friend class quadtree_t;

	/*** parameters ***/
	public:

	point_t average; /* the average position */
	int num_points; /* number of points added */
	set<unsigned int> pose_inds;
	normal_t norm;

	/*** helper parameters ***/
	private:
	point_t sum_pos; /* the sum of all points added */

	/*** functions ***/
	public:

	/* constructors */
	quaddata_t();
	~quaddata_t();

	/* accessors */

	/* add:
	 *
	 * 	Incorporates a point into this data structure,
	 * 	updating fields.
	 */
	void add(point_t& p);

	/* clone:
	 *
	 * 	Allocates new memory that is a deep clone of
	 * 	this data object.
	 */
	quaddata_t* clone();

	/* i/o */

	/* print:
	 *
	 * 	Prints the information in this struct to the
	 * 	given stream.
	 *
	 *	Does NOT print a new line.
	 *
	 * format:
	 *
	 * 	<x> <y> <nx> <ny> <num_pts> <num_poses> <pose1> <pose2> ...
	 */
	void print(ostream& os);
};

#endif
