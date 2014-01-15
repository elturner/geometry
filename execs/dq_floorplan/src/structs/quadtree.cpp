#include "quadtree.h"
#include <vector>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "point.h"
#include "normal.h"
#include "parameters.h"

using namespace std;

/***** QUADTREE FUNCTIONS ****/

quadtree_t::quadtree_t()
{
	/* make the simplest tree */
	this->root = NULL;
	this->max_depth = -1;
}

/* pass the max depth via grid res */
quadtree_t::quadtree_t(double r)
{
	point_t c;

	/* make simplest tree, so max depth is zero */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
}

quadtree_t::~quadtree_t()
{
	this->clear();
}
	
void quadtree_t::set_resolution(double r)
{
	point_t c;

	/* clear the tree */
	if((this->root) != NULL)
		delete (this->root);

	/* remake tree */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
}
	
double quadtree_t::get_resolution()
{
	return (2.0 * this->root->halfwidth) / (1 << this->max_depth);
}
	
void quadtree_t::clear()
{
	if((this->root) != NULL)
	{
		delete (this->root);
		this->root = NULL;
	}
	this->max_depth = -1;
}
	
void quadtree_t::clone_from(const quadtree_t& other)
{
	/* destroy all data in this tree */
	this->clear();

	/* copy tree information */
	this->max_depth = other.max_depth;

	/* recursive clone nodes */
	this->root = other.root->clone();
}

quaddata_t* quadtree_t::insert(point_t& p)
{
	int c;
	quadnode_t* nn;
	quaddata_t* ret;
	point_t np;

	/* verify root isn't null */
	if(!(this->root))
	{
		cerr << "[insert]\tError: root is null" << endl;
		return NULL;
	}

	/* check edge case of the tree being empty */
	if(this->root->isempty() && this->max_depth == 0)
	{
		/* reset root.center to be this point, and since
		 * the root is a leaf, add this point to its
		 * data structure */
		this->root->center = p;

		/* add p to data of root */
		ret = this->root->insert(p, 0);
		if(ret == NULL)
		{
			cerr << "[insert]\tError inserting point"
				" into empty tree.  Uh-oh.  p = ";
			p.print(cerr);
			cerr << endl;
			return ret;
		}
	}
		
	/* first, check if this point goes out of bounds of
	 * this tree. */
	while(!(this->root->contains(p)))
	{
		/* expand the tree in the direction of p
		 * by creating a wrapper node that will become
		 * the new root.  The first task is to determine
		 * which child of this wrapper the current root will
		 * be */
		if(this->root->center.get(0) < p.get(0))
		{
			if(this->root->center.get(1) < p.get(1))
			{
				/* child #2, lower left */
				c = 2;
				np.set(0, this->root->center.get(0)
					+ this->root->halfwidth);
				np.set(1, this->root->center.get(1)
					+ this->root->halfwidth);
			}
			else
			{
				/* child #1, upper left */
				c = 1;
				np.set(0, this->root->center.get(0)
					+ this->root->halfwidth);
				np.set(1, this->root->center.get(1)
					- this->root->halfwidth);
			}
		}
		else
		{
			if(this->root->center.get(1) < p.get(1))
			{
				/* child #3, lower right */
				c = 3;
				np.set(0, this->root->center.get(0)
					- this->root->halfwidth);
				np.set(1, this->root->center.get(1)
					+ this->root->halfwidth);
			}
			else
			{
				/* child #0, upper right */
				c = 0; 
				np.set(0, this->root->center.get(0)
					- this->root->halfwidth);
				np.set(1, this->root->center.get(1)
					- this->root->halfwidth);
			}
		}

		/* next, create the wrapper node */
		nn = new quadnode_t(np, this->root->halfwidth * 2);

		/* populate the children of this wrapper */
		nn->children[c] = this->root;
		
		/* set nn to be new root */
		this->root = nn;
		this->max_depth++; /* adding another layer, but keeping
					resolution the same */
	}

	/* insert point in tree */
	ret = this->root->insert(p, this->max_depth);
	if(ret == NULL)
	{
		cerr << "[insert]\tError inserting point into tree" << endl;
		cerr << "\t\tp = ";
		p.print(cerr);
		cerr << endl;
	}
	return ret;
}
	
quaddata_t* quadtree_t::insert(point_t& p, normal_t& n)
{
	quaddata_t* dat;

	/* insert this point */
	dat = this->insert(p);
	if(dat == NULL)
		return NULL;

	/* incorporate the given normal into existing normal */
	dat->norm.weighted_sum(dat->num_points, n, 1);
	if(dat->norm.iszero())
	{
		/* do not allow zero normals */
		dat->norm = n;
	}

	/* success */
	return dat;
}

quaddata_t* quadtree_t::insert(point_t& p, normal_t& n,
					unsigned int pose_ind)
{
	quaddata_t* dat;

	/* insert the point and normal */
	dat = this->insert(p, n);
	if(dat == NULL)
		return NULL;

	/* add pose index to data */
	dat->pose_inds.insert(pose_ind);
	return dat;
}

quaddata_t* quadtree_t::retrieve(point_t& p)
{
	/* just call root's retrieve function */
	quadnode_t* node = this->root->retrieve(p);
	if(node == NULL)
		return NULL;
	return node->data;
}
	
quaddata_t* quadtree_t::nearest_neighbor(point_t& p)
{
	quaddata_t* best;
	int ret;

	/* search the root */
	best = NULL;
	ret = this->root->nearest_neighbor(&best, p);
	if(ret)
	{
		cerr << "[nearest_neighbor]\t"
			"Call failed with error #" << ret << " at"
			" position: ";
		p.print(cerr);
		cerr << endl;
		return NULL;
	}

	/* return the best node (will be null if tree empty) */
	return best;
}

int quadtree_t::neighbors_in_range(point_t& p, double r,
			vector<quaddata_t*>& neighs)
{
	/* call root's function */
	return this->root->nodes_in_range(p, r, neighs);
}

void quadtree_t::raytrace(vector<quaddata_t*>& xings,
					point_t& a, point_t& b)
{
	/* call root's function */
	this->root->raytrace(xings, a, b);
}

void quadtree_t::print(ostream& os)
{
	/* set up stream */
	os << setprecision(9) << fixed;

	/* print tree geometry to stream */
	os << this->max_depth << endl
	   << this->root->halfwidth << endl
	   << this->root->center.get(0) << " "
	   << this->root->center.get(1) << endl;

	/* print data of tree */
	this->root->print(os);
}
	
int quadtree_t::parse(istream& is)
{
	int i, num_points, num_poses;
	unsigned int pose_ind;
	double hw, x, y, nx, ny;
	point_t center, p;
	normal_t n;
	quaddata_t* dat;

	/* clear the current tree */
	this->clear();

	/* parse tree structure from file */
	is >> this->max_depth;
	is >> hw;
	is >> x >> y;

	/* check validity */
	if(this->max_depth < 0)
		return -1;
	if(hw <= 0)
		return -2;

	/* initialize root */
	center.set(0,x);
	center.set(1,y);
	this->root = new quadnode_t(center, hw);

	/* insert data */
	while(!(is.eof()))
	{
		/* read initial part of data */
		is >> x >> y >> nx >> ny >> num_points >> num_poses;
	
		/* verify */
		if(num_points <= 0)
			return -3;
		if(num_poses < 0)
			return -4;

		/* insert point */
		p.set(0,x);
		p.set(1,y);
		n.set(0,nx);
		n.set(1,ny);
		dat = this->insert(p, n);
		if(dat == NULL)
			return -5;

		/* update data parameters */
		dat->average = p;
		dat->num_points = num_points;
		dat->norm = n;
		for(i = 0; i < NUM_DIMS; i++)
			dat->sum_pos.set(i, p.get(i) * num_points);

		/* read pose indices */
		for(i = 0; i < num_poses; i++)
		{
			is >> pose_ind;
			dat->pose_inds.insert(pose_ind);
		}
	}

	/* success */
	return 0;
}

void quadtree_t::print_points(ostream& os)
{
	this->root->print_points(os);
}

void quadtree_t::print_nodes(ostream& os)
{
	this->root->print_nodes(os);
}

/***** QUADNODE FUNCTIONS ****/

quadnode_t::quadnode_t()
{
	int i;

	/* set default values */
	this->halfwidth = -1;
	this->data = NULL;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

quadnode_t::quadnode_t(point_t& c, double hw)
{
	int i;

	/* set default values */
	this->center = c;
	this->halfwidth = hw;
	this->data = NULL;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

quadnode_t::~quadnode_t()
{
	int i;

	/* free children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
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
	
bool quadnode_t::isleaf()
{
	int i;

	/* check if any children aren't null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			return false;

	/* all children are null, must be leaf */
	return true;
}
	
inline bool quadnode_t::isempty()
{
	int i;

	/* a node is empty if all of its children are
	 * empty and it has no data */

	/* check for data */
	if(this->data != NULL)
		return false;

	/* check children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			return false; /* child not empty */

	/* all children are empty and no data at this level */
	return true;
}

void quadnode_t::init_child(int i)
{
	double chw;
	point_t cc;

	/* check if bad argument */
	if(i < 0 || i >= CHILDREN_PER_NODE)
	{
		cerr << "[init_child]\tGiven invalid"
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
	
	/* set position based on child number */
	switch(i)
	{
		case 0: /* upper right */
			cc.set(0, this->center.get(0) + chw);
			cc.set(1, this->center.get(1) + chw);
			break;

		case 1: /* upper left */
			cc.set(0, this->center.get(0) - chw);
			cc.set(1, this->center.get(1) + chw);
			break;

		case 2: /* lower left */
			cc.set(0, this->center.get(0) - chw);
			cc.set(1, this->center.get(1) - chw);
			break;

		case 3: /* lower right */
			cc.set(0, this->center.get(0) + chw);
			cc.set(1, this->center.get(1) - chw);
			break;

		default:
			/* case can never happen */
			cerr << "[init_child]\tERROR: something"
				" has gone terribly wrong..." << endl;
			return;
	}

	/* create new node */
	this->children[i] = new quadnode_t(cc, chw);
}
	
quadnode_t* quadnode_t::clone()
{
	quadnode_t* c;
	int i;

	/* allocate a new node */
	c = new quadnode_t(this->center, this->halfwidth);

	/* make a copy of the data */
	if(this->data != NULL)
		c->data = this->data->clone();

	/* copy children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			c->children[i] = this->children[i]->clone();

	/* return result */
	return c;
}

inline bool quadnode_t::contains(point_t& p)
{
	int i;
	double pi, ci, h;
	
	/* check p with respect to the bounds of this node
	 * in each dimension */
	h = this->halfwidth;
	for(i = 0; i < NUM_DIMS; i++)
	{
		pi = p.get(i);
		ci = this->center.get(i);
		
		/* containment is inclusive on left, exclusive on right */
		if(pi < ci - h || pi >= ci + h)
			return false; /* out of bounds */
	}

	/* all dimensions check out */
	return true;
}

inline int quadnode_t::child_contains(point_t& p)
{
	double dx, dy;
	
	/* get displacement of p with respect to center */
	dx = p.get(0) - this->center.get(0);
	dy = p.get(1) - this->center.get(1);

	/* check which quadrant this is in */
	if(dx >= 0)
	{
		if(dy >= 0)
			return 0; /* upper right */
		else
			return 3; /* lower right */
	}
	else
	{
		if(dy >= 0)
			return 1; /* upper left */
		else
			return 2; /* lower left */
	}
}
	
bool quadnode_t::intersects_line_segment(point_t& a, point_t& b)
{
	double t, x, y;
	int i;

	/* check if contains end-points */
	if(this->contains(a) || this->contains(b))
		return true;

	/* first, check the bounding box for this segment */
	for(i = 0; i < NUM_DIMS; i++)
		if(a.get(i) < b.get(i))
		{
			if(b.get(i) < this->center.get(i) 
					- this->halfwidth
					|| a.get(i) > this->center.get(i) 
					+ this->halfwidth)
				return false;
		}
		else
		{
			if(a.get(i) < this->center.get(i) 
					- this->halfwidth
					|| b.get(i) > this->center.get(i) 
					+ this->halfwidth)
				return false;
		}

	/* check edge case */
	if(a.get(0) == b.get(0) || a.get(1) == b.get(1))
	{
		/* line is vertical or horizontal, so from bounding box 
		 * check, it cannot intersect this axis-aligned square */
		return false;
	}

	/* check intersections with faces */
	
	/* east */
	x = this->center.get(0) + this->halfwidth;
	t = (x - b.get(0)) / (a.get(0) - b.get(0));
	y = fabs(b.get(1) + t*(a.get(1) - b.get(1)) - this->center.get(1));
	if(t >= 0 && t <= 1 && y <= this->halfwidth)
		return true;
	
	/* west */
	x = this->center.get(0) - this->halfwidth;
	t = (x - b.get(0)) / (a.get(0) - b.get(0));
	y = fabs(b.get(1) + t*(a.get(1) - b.get(1)) - this->center.get(1));
	if(t >= 0 && t <= 1 && y <= this->halfwidth)
		return true;

	/* north */
	y = this->center.get(1) + this->halfwidth;
	t = (y - b.get(1)) / (a.get(1) - b.get(1));
	x = fabs(b.get(0) + t*(a.get(0) - b.get(0)) - this->center.get(0));
	if(t >= 0 && t <= 1 && x <= this->halfwidth)
		return true;

	/* south */
	y = this->center.get(1) - this->halfwidth;
	t = (y - b.get(1)) / (a.get(1) - b.get(1));
	x = fabs(b.get(0) + t*(a.get(0) - b.get(0)) - this->center.get(0));
	if(t >= 0 && t <= 1 && x <= this->halfwidth)
		return true;

	/* no intersections found */
	return false;
}

quaddata_t* quadnode_t::insert(point_t& p, int d)
{
	int i;

	/* verify that node contains p */
	if(!(this->contains(p)))
	{
		cerr << "[insert]\tGot to node that doesn't contain"
			" the point! d = " << d << endl;
		cerr << "\tnode center: ";
		this->center.print(cerr);
		cerr << endl;
		cerr << "\tnode hw: " << this->halfwidth << endl;
		cerr << "\tp: ";
		p.print(cerr);
		cerr << endl << endl;
		return NULL;
	}

	/* check if base case reached */
	if(d <= 0)
	{
		/* incorporate point to this node's data */
		if(this->data == NULL)
			this->data = new quaddata_t();
		this->data->add(p);
		return this->data;
	}

	/* get the child that contains p */
	i = this->child_contains(p);

	/* make sure child exists */
	if(this->children[i] == NULL)
		this->init_child(i);

	/* continue insertion */
	return this->children[i]->insert(p, d-1);
}
	
quadnode_t* quadnode_t::retrieve(point_t& p)
{
	int i;

	/* check if current node is leaf */
	if(this->isleaf())
		return this;

	/* get appropriate child */
	i = this->child_contains(p);

	/* check if child exists */
	if(this->children[i] == NULL)
		return this; /* no child, return current node */
	return this->children[i]->retrieve(p); /* recurse through child */
}

int quadnode_t::nearest_neighbor(quaddata_t** best_so_far, point_t& p)
{
	int i, ret;
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
		for(i = 0; i < CHILDREN_PER_NODE; i++)
			if(this->children[i] != NULL
				&& !(this->children[i]->isempty()))
			{
				/* get distance of child to p */
				d = p.dist_sq(
					this->children[i]->center);
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
	d_best = p.dist_sq((*best_so_far)->average);

	/* if this node is non-empty leaf, check against data */
	if(this->isleaf())
	{
		/* check if this node could compete with best
		 * so far */
		if(!(this->isempty()))
		{
			/* compare distances */
			d = p.dist_sq(this->data->average);
			if(d < d_best)
			{
				/* our best guess is now this node */
				*best_so_far = this->data;
			}
		}
		
		/* since this is a leaf, can't do anything more */
		return 0;
	}
	d_best = sqrt(d_best); /* easier to take distance rather
					than squared-distance at
					this place in the code */

	/* now that we have an approximate solution, check children
	 * for better possibilites.  Of course, no need to recurse
	 * into the child that contains best_so_far */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* only search non-null children */
		if(this->children[i] == NULL)
			continue;

		/* check if this child contains current best */
		if(this->children[i]->contains((*best_so_far)->average))
			continue; /* don't bother searching */

		/* check if this child intersects the circle
		 * of distance d_best from p.
		 * If not, then there's no way that it
		 * or its descendants will change the estimate. */
		d = p.dist_L_inf(this->children[i]->center);
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

int quadnode_t::nodes_in_range(point_t& p, double r,
				vector<quaddata_t*>& neighs)
{
	int i, ret;
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
			d = p.dist_sq(this->data->average);
			if(d < r*r)
				neighs.push_back(this->data);
		}
		return 0;
	}

	/* recurse over intersecting children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* only search non-null children */
		if(this->children[i] == NULL)
			continue;

		/* check if child intersects with circle around p */
		d = p.dist_L_inf(this->children[i]->center);
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
					point_t& a, point_t& b)
{
	int i;

	/* first, check if this ray even intersects this node */
	if(!(this->intersects_line_segment(a, b)))
		return; /* we're done here */

	/* check if this node had any data to add */
	if(this->data != NULL)
		xings.push_back(this->data);

	/* recurse for children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->raytrace(xings, a, b);
}

void quadnode_t::print(ostream& os)
{
	int i;

	/* check if we're at a leaf */
	if(this->isleaf())
	{
		/* print data if it exists */
		if(this->data != NULL)
		{
			this->data->print(os);
			os << endl;
		}

		/* we're done here */
		return;
	}

	/* recurse */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->print(os);
}

void quadnode_t::print_points(ostream& os)
{
	int i;

	/* check if data exists for this node */
	if(this->data != NULL)
	{
		/* print data */
		os << this->data->average.get(0) << " " 
		   << this->data->average.get(1) << endl;
	}
	
	/* recurse over children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->print_points(os);
}

void quadnode_t::print_nodes(ostream& os)
{
	int i;

	/* print this node */
	os << this->center.get(0) << " "
	   << this->center.get(1) << " "
	   << this->halfwidth << endl;

	/* recurse over children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->print_nodes(os);
}

/********** QUADDATA FUNCTIONS **************/

quaddata_t::quaddata_t()
{
	/* set all counters to zero */
	this->num_points = 0;
}

quaddata_t::~quaddata_t() { /* no work necessary */ }

void quaddata_t::add(point_t& p)
{
	int i;

	/* add to sum */
	this->sum_pos += p;
	this->num_points++;

	/* update other values */
	for(i = 0; i < NUM_DIMS; i++)
	{
		this->average.set(i,
			this->sum_pos.get(i) / this->num_points);
	}
}
	
quaddata_t* quaddata_t::clone()
{
	quaddata_t* c;

	/* allocate new memory */
	c = new quaddata_t();
	c->average = this->average;
	c->num_points = this->num_points;
	c->pose_inds.insert(this->pose_inds.begin(), this->pose_inds.end());
	c->norm = this->norm;
	c->sum_pos = this->sum_pos;

	/* return clone */
	return c;
}
	
void quaddata_t::print(ostream& os)
{
	set<unsigned int>::iterator it;

	/* print static info */
	os << this->average.get(0) << " "
	   << this->average.get(1) << " "
	   << this->norm.get(0) << " "
	   << this->norm.get(1) << " "
	   << this->num_points << " "
	   << this->pose_inds.size();

	/* write poses */
	for(it = this->pose_inds.begin(); it != this->pose_inds.end(); it++)
		os << " " << (*it);
}
