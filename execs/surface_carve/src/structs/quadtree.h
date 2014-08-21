#ifndef QUADTREE_H
#define QUADTREE_H

/* quadtree.h:
 *
 * This file defines the two-color quad-tree data structure, where
 * nodes can be refined down to integer-level widths
 */

#include <vector>
#include "../util/parameters.h"

using namespace std;

/* These defines represent the structure of the tree */
#define QUADTREE_CHILDREN_PER_NODE 4
#define QUADTREE_DIMENSIONS        2

/* quadtree subdivision scheme:
 *
 *	+-----+-----+
 *	|     |     |
 *	|  1  |  0  |
 *	+-----+-----+
 *	|     |     |
 *	|  2  |  3  |
 *	+-----+-----+
 */
static const int quadtree_child_arrangement[
			QUADTREE_CHILDREN_PER_NODE][
			QUADTREE_DIMENSIONS] = {
/* child #0: upper-right */ { 1, 1},
/* child #1: upper-left  */ {-1, 1},
/* child #2: lower-left  */ {-1,-1},
/* child #3: lower-right */ { 1,-1}
};

/* The following declarations represent the classes defined in this file */
class quadvert_t;
class quadtri_t;
class quadnode_t;
class quadtree_t;

/* this represents a discretized point in the quadtree's subspace */
class quadvert_t
{
	/**** properties ****/
	public:

	/* position of this vertex.  Note it must be a corner of
	 * the quadtree, so it has to be an integer */
	int x;
	int y;

	/**** functions ****/
	public:

	/* constructors */
	quadvert_t() { this->x = 0; this->y = 0; };
	quadvert_t(int xx, int yy) { this->set(xx,yy); };
	~quadvert_t() {};

	/* accessors */
	void set(int xx, int yy) { this->x = xx ; this->y = yy; };
};

/* this represents an element of the triangulation of a quadtree */
class quadtri_t
{
	/*** properties ***/
	public:

	/* the vertices of this triangle */
	quadvert_t v[NUM_VERTS_PER_TRI];

	/*** functions ***/
	public:

	/* constructors */
	quadtri_t() {};
	~quadtri_t() {};
};

/* This class represents a single node in the quadtree. */
class quadnode_t
{
	/*** parameters ***/
	public:

	/* Have pointers to all children.  If these pointers
	 * are all null, then this is a leaf node. */
	quadnode_t* children[QUADTREE_CHILDREN_PER_NODE];

	/* These parameters represent the geometry of the node,
	 * specifying the center point of the node, and the width
	 * of each side. */
	double x;
	double y;
	int s; /* the tree is defined so that the finest resolution is
		* integer level. */

	/* This is a two-color quadtree, so each node's value
	 * can be represented by a boolean */
	bool f;

	/*** functions ***/
	public:

	/* constructors */
	quadnode_t();
	quadnode_t(double xx, double yy, int ss);
	~quadnode_t();

	/* tree traversal */

	/* is_leaf:
	 *
	 * 	Returns true iff this node is a leaf.
	 */
	bool is_leaf();

	/* at_max_depth:
	 *
	 * 	Returns true iff this node is at the maximum
	 * 	allowable depth.
	 */
	bool at_max_depth();

	/* subdivide:
	 *
	 *	If this node is a leaf, will create children for
	 *	this node.  If this node is not a leaf, no action
	 *	is performed.
	 */
	void subdivide();

	/* is_inside:
	 *
	 * 	Given a 2D point, will return true iff that point
	 * 	is contained in this node.
	 */
	bool is_inside(double xx, double yy);

	/* set_leaf_value:
	 *
	 * 	Given a 2D point and a value, will set the leaf
	 * 	that contains this point to the given value.  Will
	 * 	subdivide the tree if necessary, so that the leaf is
	 * 	at the maximum depth.
	 */
	void set_leaf_value(double xx, double yy, bool ff);

	/* simplify:
	 *
	 * 	Recursively simplifies all children, possibly
	 * 	reducing the size of the tree while still representing
	 * 	the same space coloring.
	 */
	void simplify();

	/* get_neighbors_under:
	 *
	 * 	Fills the specified vector with the neighboring
	 * 	nodes to this one, intersected with the nodes
	 * 	underneath the argument.
	 *
	 * arguments:
	 *
	 * 	neighs -	Where to store the neighbors of this node
	 * 			that appear under q
	 *
	 * 	q -		The node under which to check for neighbors.
	 */
	void get_neighbors_under(vector<quadnode_t*>& neighs, 
						quadnode_t* q);

	/* edge_in_common:
	 *
	 * 	Given a neighboring node of this node, will compute
	 * 	the edge in common between the two nodes.
	 *
	 * arguments:
	 *
	 * 	x1,y1 -	Where to store the first vertex of this edge
	 * 	x2,y2 -	Where to store the second vertex of this edge
	 * 	q -	The neighbor to analyze.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int edge_in_common(int& x1, int& y1, int& x2, int& y2,
						quadnode_t* q);

	/* triangulate:
	 *
	 * 	Will recursively add the triangles of this node
	 * 	(and its children) to the list of triangles provided.
	 */
	void triangulate(vector<quadtri_t>& tris, quadnode_t* root);
};

/* This class represents the full tree */
class quadtree_t
{
	/*** parameters ***/
	private:

	/* The root node of the tree can also be viewed as a bounding
	 * box of the data.  Its center will be the origin. */
	quadnode_t root;

	public:

	/* The triangles that represent the meshing of this quadtree */
	vector<quadtri_t> triangles;

	/*** functions ***/
	public:

	/* constructors */
	quadtree_t();
	quadtree_t(int s); /* specify width of root node */
	~quadtree_t();

	/* fill_point:
	 *
	 * 	Will subdivide the tree, and set the leaves at this
	 * 	location to have a value of f.
	 */
	void fill_point(double x, double y, bool f);

	/* triangulate:
	 *
	 * 	Will populate the triangles vector with
	 * 	a mesh that corresponds to the leaves of
	 * 	this quadtree.
	 */
	void triangulate();
};


#endif
