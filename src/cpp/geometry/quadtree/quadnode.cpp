#include "quadnode.h"
#include <geometry/quadtree/quaddata.h>
#include <geometry/shapes/linesegment_2d.h>
#include <geometry/poly_intersect/poly2d.h>
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <float.h>

/**
 * @file    quadnode.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   A node in the quadtree structure
 *
 * @section DESCRIPTION
 *
 * This file contains the quadnode_t class, which is used to
 * represent a single node in a quadtree.  A quadnode also contains
 * pointers to all its subnodes.
 */

using namespace std;
using namespace Eigen;

/*--------------------*/
/* QUADNODE FUNCTIONS */
/*--------------------*/

quadnode_t::quadnode_t()
{
	size_t i;

	/* set default values */
	this->halfwidth = -1;
	this->data = NULL;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		this->children[i] = NULL;
}

quadnode_t::quadnode_t(const Eigen::Vector2d& c, double hw)
{
	size_t i;

	/* set default values */
	this->center = c;
	this->halfwidth = hw;
	this->data = NULL;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		this->children[i] = NULL;
}

quadnode_t::~quadnode_t()
{
	size_t i;

	/* free children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
		{
			delete (this->children[i]);
			this->children[i] = NULL;
		}
	
	/* free data */
	if(this->data != NULL)
	{
		delete (this->data);
		this->data = NULL;
	}
}
	
bool quadnode_t::isleaf() const
{
	size_t i;

	/* check if any children aren't null */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
			return false;

	/* all children are null, must be leaf */
	return true;
}
	
bool quadnode_t::isempty() const
{
	size_t i;

	/* a node is empty if all of its children are
	 * empty and it has no data */

	/* check for data */
	if(this->data != NULL)
		return false;

	/* check children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
			return false; /* child not empty */

	/* all children are empty and no data at this level */
	return true;
}

void quadnode_t::init_child(size_t i)
{
	double chw;
	Vector2d cc;

	/* check if bad argument */
	if(i >= CHILDREN_PER_QUADNODE)
	{
		cerr << "[quadnode::init_child]\tGiven invalid"
				" child index: " << i << endl;
		return;
	}

	/* first, make sure i'th child doesn't exist yet */
	if(this->children[i] != NULL)
	{
		/* child already exists, do nothing */
		return;
	}

	/* set default geometry for i'th child */
	chw = this->halfwidth / 2; /* child is half the size of parent */
	cc = this->child_center(i); /* center of i'th child */

	/* create new node */
	this->children[i] = new quadnode_t(cc, chw);
}
	
quadnode_t* quadnode_t::clone() const
{
	quadnode_t* c;
	size_t i;

	/* allocate a new node */
	c = new quadnode_t(this->center, this->halfwidth);

	/* make a copy of the data */
	if(this->data != NULL)
		c->data = this->data->clone();

	/* copy children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
			c->children[i] = this->children[i]->clone();

	/* return result */
	return c;
}

int quadnode_t::contains(const Eigen::Vector2d& p) const
{
	Vector2d diff;
	double d;
	
	/* check that p is within cube around center of this node */
	diff = (p - this->center);
	d = diff.cwiseAbs().maxCoeff();
	if(d > this->halfwidth)
		return -1; /* point not in this node */

	/* check which quadrant this point is in */
	if(diff(0) >= 0)
	{
		/* x is positive */
		if(diff(1) >= 0)
			return 0; /* upper right */
		return 3; /* lower right */
	}

	/* x is negative */
	if(diff(1) >= 0)
		return 1; /* upper left */
	return 2; /* lower left */
}

bool quadnode_t::intersects(const linesegment_2d_t& line) const
{
	Vector3d pos;

	/* set this node's position in "3d" space */
	pos(0) = this->center(0);
	pos(1) = this->center(1);
	pos(2) = 0.0;

	/* test intersections */
	return line.intersects(pos, this->halfwidth);
}
		
Eigen::Vector2d quadnode_t::child_center(size_t i) const
{
	Vector2d cc;
	double chw;

	/* child is half the size of parent */
	chw = this->halfwidth / 2; 
	
	/* set position based on child number */
	switch(i)
	{
		case 0: /* upper right */
			cc(0) = this->center(0) + chw;
			cc(1) = this->center(1) + chw;
			break;

		case 1: /* upper left */
			cc(0) = this->center(0) - chw;
			cc(1) = this->center(1) + chw;
			break;

		case 2: /* lower left */
			cc(0) = this->center(0) - chw;
			cc(1) = this->center(1) - chw;
			break;

		case 3: /* lower right */
			cc(0) = this->center(0) + chw;
			cc(1) = this->center(1) - chw;
			break;

		default:
			/* case can never happen */
			cerr << "[quadnode::child_center]\tERROR: something"
			     << " has gone terribly wrong... i = " << i 
			     << endl;
			break;
	}

	/* return the computed position */
	return cc;
}
		
Eigen::Vector2d quadnode_t::corner_position(size_t i) const
{
	Vector2d p;

	/* the i'th child center is halfwidth/2 away, while
	 * the corner position is in the same direction, just
	 * twice as far away. */
	p = this->child_center(i);

	/* get the corner position from the child position */
	p = 2*(p - this->center) + this->center;
	
	/* return the corner position */
	return p;
}
		
int quadnode_t::edge_in_common(Eigen::Vector2d& a, Eigen::Vector2d& b,
					quadnode_t* other,
					double res) const
{
	double mx[2]; /* min/max bounds for this node in x */
	double my[2]; /* min/max bounds for this node in y */
	double ox[2]; /* min/max bounds for other node in x */
	double oy[2]; /* min/max bounds for other node in y */

	/* compute bounds on nodes */
	mx[0] = this->center(0)  - this->halfwidth;
	mx[1] = this->center(0)  + this->halfwidth;
	my[0] = this->center(1)  - this->halfwidth;
	my[1] = this->center(1)  + this->halfwidth;
	ox[0] = other->center(0) - other->halfwidth;
	ox[1] = other->center(0) + other->halfwidth;
	oy[0] = other->center(1) - other->halfwidth;
	oy[1] = other->center(1) + other->halfwidth;

	/* check four possible sides for neighbor */
	if(abs(mx[1] - ox[0]) < res)
	{
		/* vertical edge going up */
		a(0) = b(0) = mx[1];
		a(1) = max(my[0], oy[0]);
		b(1) = min(my[1], oy[1]);
	}
	else if(abs(mx[0] - ox[1]) < res)
	{
		/* vertical edge going down */
		a(0) = b(0) = mx[0];
		a(1) = min(my[1], oy[1]);
		b(1) = max(my[0], oy[0]);
	}
	else if(abs(my[1] - oy[0]) < res)
	{
		/* horizontal edge, going left */
		a(1) = b(1) = my[1];
		a(0) = min(mx[1], ox[1]);
		b(0) = max(mx[0], ox[0]);
	}
	else if(abs(my[0] - oy[1]) < res)
	{
		/* horizontal edge, going right */
		a(1) = b(1) = my[0];
		a(0) = max(mx[0], ox[0]);
		b(0) = min(mx[1], ox[1]);
	}
	else
	{
		/* not neighbors! */
		return -1;
	}

	/* success */
	return 0;
}
		
void quadnode_t::subdivide(double xs[2], double ys[2], int d)
{
	double myx[2];
	double myy[2];
	Vector2d cc;
	size_t i;
	double chw;

	/* check if we've reached the desired resolution */
	if(d <= 0)
		return; /* don't need to divide any further */

	/* check if the input geometry intersects with where
	 * each child node would exist */
	chw = this->halfwidth / 2;
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
	{
		/* get hypothetical center of child, and use it to
		 * compute the child's bounding box */
		cc = this->child_center(i);
		myx[0] = cc(0) - chw; /* min-x */
		myx[1] = cc(0) + chw; /* max-x */
		myy[0] = cc(1) - chw; /* min-y */
		myy[1] = cc(1) + chw; /* max-y */

		/* check for intersection */
		if(!(poly2d::aabb_in_aabb(myx, myy, xs, ys)))
			continue;

		/* intersection occurs at this child, make
		 * sure it exists */
		this->init_child(i);

		/* recurse to this child */
		this->children[i]->subdivide(xs, ys, d-1);
	}
}
		
bool quadnode_t::simplify()
{
	size_t i;
	bool all_simple;

	/* check for base case, which occurs if node is a leaf
	 * with no data elements */
	if(this->data == NULL && this->isleaf())
		return true; /* already simplified */

	/* check if this node has data.  We cannot simplify data nodes */
	if(this->data != NULL)
		return false; /* can't simplify */

	/* check if all children are simplified */
	all_simple = true;
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
	{
		if(this->children[i] == NULL)
			all_simple = false;
		else
			all_simple &= this->children[i]->simplify();
	}

	/* check if we can simplify this node */
	if(!all_simple)
		return false; /* can't simplify */

	/* delete all children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
	{
		delete this->children[i];
		this->children[i] = NULL;
	}

	/* successfully simplified */
	return true;
}
		
void quadnode_t::get_neighbors_under(std::vector<quadnode_t*>& neighs,
						quadnode_t* parent,
						double err) const
{
	double myx[2];
	double myy[2];
	double px[2];
	double py[2];
	size_t i;

	/* check edge cases */
	if(this == parent || parent == NULL)
		return; /* don't do anything */

	/* get bounding box for this node */
	myx[0] = this->center(0) - this->halfwidth; /* min-x */
	myx[1] = this->center(0) + this->halfwidth; /* max-x */
	myy[0] = this->center(1) - this->halfwidth; /* min-y */
	myy[1] = this->center(1) + this->halfwidth; /* max-y */
	
	/* get bounding box for parent */
	px[0] = parent->center(0) - parent->halfwidth; /* min-x */
	px[1] = parent->center(0) + parent->halfwidth; /* max-x */
	py[0] = parent->center(1) - parent->halfwidth; /* min-y */
	py[1] = parent->center(1) + parent->halfwidth; /* max-y */

	/* check if parent is actually a leaf (this is the base case) */
	if(parent->isleaf())
	{
		/* can only be a neighbor if the two nodes abut */
		if(poly2d::aabb_pair_abut(myx, myy, px, py, err))
			neighs.push_back(parent);
		
		/* we're done here */
		return;
	}

	/* since 'parent' is not a leaf, we need to check for both
	 * abutting or overlapping */
	if(!(poly2d::aabb_in_aabb(myx, myy, px, py))
		&& !(poly2d::aabb_pair_abut(myx, myy, px, py, err)) )
		return; /* must be disjoint */

	/* if got here, then recurse to parent's children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		this->get_neighbors_under(neighs, parent->children[i], err);
}

quaddata_t* quadnode_t::insert(const Vector2d& p, const Vector2d& n,
				double w, int d)
{
	int i;

	/* verify that node contains p */
	i = this->contains(p);
	if(i < 0)
	{
		cerr << "[quadnode::insert]\tError!  "
		     << "Got to node that doesn't contain"
		     << " the point! d = " << d << endl
		     << "\tnode center: " << this->center.transpose() 
		     << endl
		     << "\tnode hw: " << this->halfwidth << endl
		     << "\tp: " << p.transpose() << endl << endl;
		return NULL;
	}

	/* check if base case reached */
	if(d <= 0)
	{
		/* incorporate point to this node's data */
		if(this->data == NULL)
			this->data = new quaddata_t();
		this->data->add(p, n, w);
		return this->data;
	}

	/* make sure child exists */
	if(this->children[i] == NULL)
		this->init_child(i);

	/* continue insertion */
	return this->children[i]->insert(p, n, w, d-1);
}
	
const quadnode_t* quadnode_t::retrieve(const Vector2d& p) const
{
	int i;

	/* get appropriate child */
	i = this->contains(p);
	if(i < 0)
		return NULL;

	/* check if child exists */
	if(this->children[i] == NULL)
		return this; /* no child, return current node */
	return this->children[i]->retrieve(p); /* recurse through child */
}

quadnode_t* quadnode_t::retrieve(const Vector2d& p)
{
	int i;

	/* get appropriate child */
	i = this->contains(p);
	if(i < 0)
		return NULL;

	/* check if child exists */
	if(this->children[i] == NULL)
		return this; /* no child, return current node */
	return this->children[i]->retrieve(p); /* recurse through child */
}

int quadnode_t::nearest_neighbor(quaddata_t** best_so_far,
			const Vector2d& p) const
{
	size_t i;
	int ret;
	double d, d_best;
	quadnode_t* b;

	/* first, check if best_so_far points to a null value */
	if( (*best_so_far) == NULL )
	{
		/* check if empty */
		if(this->isempty())
			return -1; /* can't find anything */
	
		/* check if leaf */
		if(this->isleaf())
		{
			/* since this node is all we got, it's the
			 * best by default */
			*best_so_far = this->data;
			return 0; /* this is all we can do */
		}
		
		/* initialize */
		d = d_best = DBL_MAX;
		b = NULL;
	
		/* determine the non-empty child that 
		 * is closest to p */
		for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
			if(this->children[i] != NULL
				&& !(this->children[i]->isempty()))
			{
				/* get distance of child to p */
				d = (p - this->children[i]->center)
							.squaredNorm();
				if(d < d_best)
				{
					b = this->children[i];
					d_best = d;
				}
			}
	
		/* recurse with best guess */
		if(b == NULL)
			return -2; /* ERROR! all children empty?! */
		ret = b->nearest_neighbor(best_so_far, p);
		if(ret)
			return -3; /* error on recursion */
		
		/* verify estimate exists */
		if((*best_so_far) == NULL)
			return -4;
	}

	/* now that we have an approximate solution, check the distance */
	d_best = (p - (*best_so_far)->average).squaredNorm();

	/* if this node is non-empty leaf, check against data */
	if(this->isleaf())
	{
		/* check if this node could compete with best
		 * so far */
		if(!(this->isempty()))
		{
			/* compare distances */
			d = (p - this->data->average).squaredNorm();
			if(d < d_best)
			{
				/* our best guess is now this node */
				*best_so_far = this->data;
			}
		}
		
		/* since this is a leaf, can't do anything more */
		return 0;
	}

	/* current node is NOT a leaf, so proceed */
	d_best = sqrt(d_best); /* easier to take distance rather
					than squared-distance at
					this place in the code */

	/* now that we have an approximate solution, check children
	 * for better possibilites.  Of course, no need to recurse
	 * into the child that contains best_so_far */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
	{
		/* only search non-null children */
		if(this->children[i] == NULL)
			continue;

		/* check if this child contains current best */
		if(this->children[i]->contains((*best_so_far)->average)>=0)
			continue; /* don't bother searching */

		/* check if this child intersects the circle
		 * of distance d_best from p.
		 * If not, then there's no way that it
		 * or its descendants will change the estimate. */
		d = (p - this->children[i]->center).lpNorm<Infinity>();
		if(d > d_best + this->children[i]->halfwidth)
			continue; /* child out of range */

		/* since this child isn't out of range, should
		 * check it against the best so far, since we
		 * might get lucky and find an even better element */
		ret = this->children[i]->nearest_neighbor(best_so_far, p);
		if(ret)
			return -5;
	}
	
	/* searched all nodes under this one.  The best so far is
	 * what it is, so yield it as the final result at this
	 * level */
	return 0;
}

int quadnode_t::nodes_in_range(const Vector2d& p, double r,
				vector<quaddata_t*>& neighs) const
{
	size_t i;
	int ret;
	double d;

	/* check if leaf */
	if(this->isleaf())
	{
		/* check if non-empty */
		if(!(this->isempty()))
		{
			/* check edge case of negative range */
			if(r < 0)
				neighs.push_back(this->data);

			/* check distance of data from p */
			d = (p - this->data->average).squaredNorm();
			if(d < r*r)
				neighs.push_back(this->data);
		}
		return 0;
	}

	/* recurse over intersecting children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
	{
		/* only search non-null children */
		if(this->children[i] == NULL)
			continue;

		/* check if child intersects with circle around p */
		d = (p - this->children[i]->center).lpNorm<Infinity>();
		if(r >= 0 && d > r + this->children[i]->halfwidth)
			continue; /* child out of range */
	
		/* recurse to this child */
		ret = this->children[i]->nodes_in_range(p, r, neighs);
		if(ret)
			return -1;
	}
	
	/* success */
	return 0;
}
	
void quadnode_t::raytrace(vector<quaddata_t*>& xings,
				const linesegment_2d_t& line) const
{
	size_t i;

	/* first, check if this ray even intersects this node */
	if(!(this->intersects(line)))
		return; /* we're done here */

	/* check if this node had any data to add */
	if(this->data != NULL)
		xings.push_back(this->data);

	/* recurse for children */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->raytrace(xings, line);
}

void quadnode_t::print(ostream& os) const
{
	size_t i;

	/* check if we're at a leaf */
	if(this->isleaf())
	{
		/* print data if it exists */
		if(this->data != NULL)
			this->data->print(os);

		/* we're done here */
		return;
	}

	/* recurse */
	for(i = 0; i < CHILDREN_PER_QUADNODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->print(os);
}
