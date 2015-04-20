#ifndef QUADTREE_H
#define QUADTREE_H

/* this file defines a quadtree structure.
 * The quadtree represents all of 2D space, and
 * the bounding box grows as more elements are added.
 */

/* includes */
#include <ostream>
#include <istream>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include "Point2D.h"

/* namespaces */
using namespace std;

/* defines */
#define NUM_DIMS 2
#define CHILDREN_PER_NODE 4

/* these classes are defined in this file */
class quadtree_t;
class quadnode_t;
class quaddata_t;

/* defines the quadtree class */
class quadtree_t
{
	/*** parameters ***/
	//private:
public:
	double res;

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
	quadtree_t(const quadtree_t& other);
	
	/* operators */

	/*
		quadtree_t& operator=(const quadtree_t& other)
	*/
	inline quadtree_t& operator=(const quadtree_t& other)
	{
		if(&other != this)
		{
			this->clone_from(other);	
		}
	    return *this;
	};

	/* accessors */

	/* set_resolution:
	 *
	 * 	Sets the resolution to be the argument.
	 * 	Will destroy any information in tree.
	 */
	void set_resolution(double r);

	/* clear:
	 *
	 * 	Clears all information from tree.  set_resolution
	 * 	must be called before adding more data.
	 */
	void clear();

	/* empty:
	 *
	 * Returns true if the quadtree has no points 
	 */
	inline bool empty() const
	{
		return (!root);
	};

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
	quaddata_t* insert(Point2D& p);

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
	quaddata_t* insert(Point2D& p, size_t imgid, double score);

	/* retrieve:
	 *
	 * 	Given a point, will return the quadnode
	 * 	that is the leaf that contains that point.
	 *
	 * return value:
	 *
	 * 	Returns the appropriate data on success.
	 * 	Returns NULL if p out of current range of tree.
	 */
	quaddata_t* retrieve(Point2D& p) const;
	
	/* geometry */
			
	/* ray_trace:
	 *
	 * Ray traces through the quadtree until it finds an occluder to the
	 * line segment that goes from p1 to p2.
	 *
	 * arguments:
	 *
	 *	p1 -	The start point of the ray
	 *
	 *	p2 -	The end point of the ray
	 *
	 * return value:
	 *
	 *	Returns the pointer to the node that was intersected.  If no
	 *	intersection occured then the return value will be null
	 */
	quadnode_t* ray_trace(Point2D& p1, Point2D& p2);

	/* ray trace and find all */
	void raytrace(vector<quaddata_t*>& xings,
     	Point2D& a, Point2D& b);

	/* generate the boxes that should exist between the two points */
	void trace_and_insert(vector<quaddata_t*>& xings,
     	Point2D& a, Point2D& b);

	/* i/o */

	void print(std::ostream& os);



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
	Point2D center;
	double halfwidth; /* distance from center to edge */

	/* each node also stored data elements */
	quaddata_t* data; /* only non-null for leaves */

	/*** functions ***/
	public:

	void raytrace(vector<quaddata_t*>& xings,
     	Point2D& a, Point2D& b);

	/* constructors */
	quadnode_t();
	quadnode_t(Point2D& c, double hw);
	~quadnode_t();

	/* accessors */

	/* isleaf:
	 *
	 * 	Returns true iff this node is a leaf.
	 */
	bool isleaf() const;

	/* isempty:
	 *
	 * 	Returns true iff this node is empty.
	 */
	inline bool isempty() const;

	/* clone:
	 *
	 * 	Allocates new memory that is a deep copy of
	 * 	this node and its subnodes.
	 */
	quadnode_t* clone();

	/* init_child:
	 *
	 * 	Will initialize the i'th child of this node,
	 * 	assuming it is not already initialized.
	 */
	void init_child(int i);

	/* geometry */

	/* contains:
	 *
	 * 	Returns true iff the given point resides inside
	 * 	this node.
	 */
	inline bool contains(Point2D& p);

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
	inline int child_contains(Point2D& p) const;

	/*
		bool intersects_line_segment(Point2D& a, Point2D& b)

		returns if the line formed by tracing between a and b intersect
		this node
	*/
	bool intersects_line_segment(Point2D& a, Point2D& b);
	
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
	quaddata_t* insert(Point2D& p, int d); 

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
	quaddata_t* retrieve(Point2D& p) const;

	void trace_and_insert(vector<quaddata_t*>& xings,
     	Point2D& a, Point2D& b, int depth, int maxdepth);

	void print(std::ostream& os);

};

/* this class represents the data that are stored in the nodes of
 * the quad tree.  This is only interesting at the leaves. */
class quaddata_t
{
	/*** security ***/
	friend class quadtree_t;

	/*** parameters ***/
	public:

	/* here is an std map between image id and scores */
	std::map<size_t, double> data;

	Point2D pos;

	/*** functions ***/
	public:

	/* constructors */
	quaddata_t();
	~quaddata_t();

	/* clone:
	 *
	 * 	Allocates new memory that is a deep clone of
	 * 	this data object.
	 */
	quaddata_t* clone();

};

#endif
