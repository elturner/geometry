#include "imagemap.h"
#include <vector>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <queue>
#include <stack>
#include "Point2D.h"

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
	Point2D c;
	res = r;

	/* make simplest tree, so max depth is zero */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
}

quadtree_t::~quadtree_t()
{
	this->clear();
}

quadtree_t::quadtree_t(const quadtree_t& other)
{
	/* mark the tree as newly created */
	this->root = NULL;
	this->max_depth = -1;

	/* copy the other tree */
	this->clone_from(other);
}
	
void quadtree_t::set_resolution(double r)
{
	Point2D c;
	res = r;

	/* clear the tree */
	if((this->root) != NULL)
		delete (this->root);

	/* remake tree */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
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

void quadtree_t::clear()
{
	if((this->root) != NULL)
	{
		delete (this->root);
		this->root = NULL;
	}
	this->max_depth = -1;
}

quaddata_t* quadtree_t::insert(Point2D& p)
{
	int c;
	quadnode_t* nn;
	quaddata_t* ret;
	Point2D np;

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
			cerr << p << endl;
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
		if(this->root->center.x() < p.x())
		{
			if(this->root->center.y() < p.y())
			{
				/* child #2, lower left */
				c = 2;
				np.x() = this->root->center.x()
					+ this->root->halfwidth;
				np.y() = this->root->center.y()
					+ this->root->halfwidth;
			}
			else
			{
				/* child #1, upper left */
				c = 1;
				np.x() = this->root->center.x()
					+ this->root->halfwidth;
				np.y() = this->root->center.y()
					- this->root->halfwidth;
			}
		}
		else
		{
			if(this->root->center.y() < p.y())
			{
				/* child #3, lower right */
				c = 3;
				np.x() = this->root->center.x()
					- this->root->halfwidth;
				np.y() = this->root->center.y()
					+ this->root->halfwidth;
			}
			else
			{
				/* child #0, upper right */
				c = 0; 
				np.x() = this->root->center.x()
					- this->root->halfwidth;
				np.y() = this->root->center.y()
					- this->root->halfwidth;
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
		cerr << p << endl;
	}
	return ret;
}
	
quaddata_t* quadtree_t::insert(Point2D& p, 
	size_t imgid, 
	double score)
{
	quaddata_t* dat;

	/* insert the point  */
	dat = this->insert(p);
	if(dat == NULL)
		return NULL;

	/* add pose index to data */
	std::map<size_t,double>::iterator it
		= dat->data.find(imgid);
	if(it == dat->data.end())
	{
		dat->data[imgid] = score;
	}
	else
	{
		dat->data[imgid] = std::max(it->second,score);
	}

	return dat;
}

quaddata_t* quadtree_t::retrieve(Point2D& p) const
{
	/* just call root's retrieve function */
	return this->root->retrieve(p);
}
	
/* ray_trace function */
quadnode_t* quadtree_t::ray_trace(Point2D& p1, Point2D& p2) {

	/* Push the root node of the tree onto the searchStack */
	if(this->root == NULL || this->max_depth < 0) {
		return NULL;
	}

	/* This will be where the return value is stored */
	quadnode_t* iNode = NULL;

	/* Create a stack with which we will store the elements to check */
	priority_queue< pair<double, quadnode_t*> > searchQueue;
	
	/* Check to see if this intersects the root node */
	pair<double, quadnode_t*> pairToPush( 0, this->root );
	if( this->root->intersects_line_segment(p1,p2) ) {
		searchQueue.push(pairToPush);
	}

	/* While the queue is non-empty, push the children of the thing on it */
	while( !searchQueue.empty() ) {

		const quadnode_t * currentNode = searchQueue.top().second; 
		searchQueue.pop();

		/* Check to see if we are done */
		for(size_t i = 0; i < CHILDREN_PER_NODE; i++) {

			/* Check if the child has any points in it at all */
			if( currentNode->children[i] == NULL )
				continue;

			/* Check to see if the child intersects the line */
			if( !currentNode->children[i]->intersects_line_segment(p1,p2) ) {
				continue;
			}

			/* If it does then we check if it is a leaf node.  If it is we
			   return that as the answer, if not we push it into the queue */
			if( currentNode->children[i]->isleaf() ) {
				iNode = currentNode->children[i];
				return iNode;
			}
			else {
				pairToPush.first = p1.sq_dist_to(currentNode->children[i]->center);
				pairToPush.second = currentNode->children[i];
				searchQueue.push(pairToPush);
			}
		}	
	}

	return iNode;
}

void quadnode_t::raytrace(vector<quaddata_t*>& xings,
     Point2D& a, Point2D& b)
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

/* generate the boxes that should exist between the two points */
void quadtree_t::trace_and_insert(vector<quaddata_t*>& xings,
     Point2D& a, Point2D& b)
{
	// Clear and check for null root 
	xings.clear();
	if(root == NULL)
		return;

	this->insert(a);
	this->insert(b);

	// Call the trace on the root 
	root->trace_and_insert(xings, a, b, 0, max_depth);	
}

/* generate the boxes that should exist between the two points */
void quadnode_t::trace_and_insert(vector<quaddata_t*>& xings,
     Point2D& a, Point2D& b, int depth, int maxdepth)
{
	/* check if this is a root node */
	if(depth == maxdepth)
	{
		xings.push_back(this->insert(this->center, 0));
		return;
	}

	/* check if the current node intersects the child nodes */
	for(size_t i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* make sure the child exists */
		init_child(i);

		/* check if it intersects the child */
		if(children[i]->intersects_line_segment(a,b))
			children[i]->trace_and_insert(xings,a,b,depth+1,maxdepth);
	}

	/* when done return */
	return;
}

void quadtree_t::raytrace(vector<quaddata_t*>& xings,
 	Point2D& a, Point2D& b)
{
	if(root == NULL)
	{
		xings.clear();
		return;
	}
	root->raytrace(xings, a, b);
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

quadnode_t::quadnode_t(Point2D& c, double hw)
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
	
bool quadnode_t::isleaf() const
{
	int i;

	/* check if any children aren't null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			return false;

	/* all children are null, must be leaf */
	return true;
}
	
inline bool quadnode_t::isempty() const
{
	int i;

	/* a node is empty if all of its children are
	 * empty and it has no data */

	/* check for data */
	if(this->data != NULL)
		return false;

	/* check children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL )
			return false; /* child not empty */

	/* all children are empty and no data at this level */
	return true;
}

void quadnode_t::init_child(int i)
{
	double chw;
	Point2D cc;

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
			cc.x() = this->center.x() + chw;
			cc.y() = this->center.y() + chw;
			break;

		case 1: /* upper left */
			cc.x() = this->center.x() - chw;
			cc.y() = this->center.y() + chw;
			break;

		case 2: /* lower left */
			cc.x() = this->center.x() - chw;
			cc.y() = this->center.y() - chw;
			break;

		case 3: /* lower right */
			cc.x() = this->center.x() + chw;
			cc.y() = this->center.y() - chw;
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

inline bool quadnode_t::contains(Point2D& p)
{
	int i;
	double pi, ci, h;
	
	/* check p with respect to the bounds of this node
	 * in each dimension */
	h = this->halfwidth;
	for(i = 0; i < NUM_DIMS; i++)
	{
		pi = p[i];
		ci = this->center[i];
		
		/* containment is inclusive on left, exclusive on right */
		if(pi < ci - h || pi >= ci + h)
			return false; /* out of bounds */
	}

	/* all dimensions check out */
	return true;
}

inline int quadnode_t::child_contains(Point2D& p) const
{
	double dx, dy;
	
	/* get displacement of p with respect to center */
	dx = p.x() - this->center.x();
	dy = p.y() - this->center.y();

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

bool quadnode_t::intersects_line_segment(Point2D& a, Point2D& b)
{
	double t, x, y;
	int i;

	/* check if contains end-points */
	if(this->contains(a) || this->contains(b))
		return true;

	/* first, check the bounding box for this segment */
	for(i = 0; i < NUM_DIMS; i++)
		if(a[i] < b[i])
		{
			if(b[i] < this->center[i] 
					- this->halfwidth
					|| a[i] > this->center[i] 
					+ this->halfwidth)
				return false;
		}
		else
		{
			if(a[i] < this->center[i] 
					- this->halfwidth
					|| b[i] > this->center[i] 
					+ this->halfwidth)
				return false;
		}

	/* check edge case */
	if(a[0] == b[0] || a[1] == b[1])
	{
		/* line is vertical or horizontal, so from bounding box 
		 * check, it cannot intersect this axis-aligned square */
		return false;
	}

	/* check intersections with faces */
	
	/* east */
	x = this->center.x() + this->halfwidth;
	t = (x - b.x()) / (a.x() - b.x());
	y = fabs(b.y() + t*(a.y() - b.y()) - this->center.y());
	if(t >= 0 && t <= 1 && y <= this->halfwidth)
		return true;
	
	/* west */
	x = this->center.x() - this->halfwidth;
	t = (x - b.x()) / (a.x() - b.x());
	y = fabs(b.y() + t*(a.y() - b.y()) - this->center.y());
	if(t >= 0 && t <= 1 && y <= this->halfwidth)
		return true;

	/* north */
	y = this->center.y() + this->halfwidth;
	t = (y - b.y()) / (a.y() - b.y());
	x = fabs(b.x() + t*(a.x() - b.x()) - this->center.x());
	if(t >= 0 && t <= 1 && x <= this->halfwidth)
		return true;

	/* south */
	y = this->center.y() - this->halfwidth;
	t = (y - b.y()) / (a.y() - b.y());
	x = fabs(b.x() + t*(a.x() - b.x()) - this->center.x());
	if(t >= 0 && t <= 1 && x <= this->halfwidth)
		return true;

	/* no intersections found */
	return false;
}

quaddata_t* quadnode_t::insert(Point2D& p, int d)
{
	int i;

	/* verify that node contains p */
	if(!(this->contains(p)))
	{
		cerr << "[insert]\tGot to node that doesn't contain"
			" the point! d = " << d << endl;
		cerr << "\tnode center: ";
		cerr << this->center << endl;
		cerr << "\tnode hw: " << this->halfwidth << endl;
		cerr << "\tp: ";
		cerr << p << endl << endl;
		return NULL;
	}

	/* check if base case reached */
	if(d <= 0)
	{
		/* incorporate point to this node's data */
		if(this->data == NULL)
		{
			this->data = new quaddata_t();
			this->data->pos = this->center;
		}

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
	
quaddata_t* quadnode_t::retrieve(Point2D& p) const
{
	int i;

	/* check if current node is leaf */
	if(this->isleaf())
		return this->data;

	/* get appropriate child */
	i = this->child_contains(p);

	/* check if child exists */
	if(this->children[i] == NULL)
		return this->data; /* no child, return current node */
	return this->children[i]->retrieve(p); /* recurse through child */
}

/********** QUADDATA FUNCTIONS **************/

quaddata_t::quaddata_t()
{
}

quaddata_t::~quaddata_t() { /* no work necessary */ }



quaddata_t* quaddata_t::clone()
{
	quaddata_t* c;

	/* allocate new memory */
	c = new quaddata_t();

	c->data = data;
	c->pos = pos;

	/* return clone */
	return c;
}

void quadtree_t::print(std::ostream& os)
{
	if(root == NULL)
		return;
	os << res << '\n';
	root->print(os);
}

void quadnode_t::print(std::ostream& os)
{
	/* if this is a leaf print */
	if(isleaf())
	{
		if(data != NULL)
		{
			os << center.x() << " " << center.y() << " " << data->data.size() << " "; 
			
			/* copy out all the image names and sort them */
			priority_queue<pair<double,size_t> > scoredImages;
			for(map<size_t,double>::iterator it = data->data.begin();
				it != data->data.end();
				it++)
				scoredImages.push(make_pair(it->second,it->first));
			
			/* Then add the images in the order we get them back */
			while(!scoredImages.empty())
			{
				os << scoredImages.top().second << " ";
				scoredImages.pop();
			}
			os << '\n';
		}
		return;
	}

	/* else recurse on children */
	for(size_t i = 0; i < CHILDREN_PER_NODE; i++)
	{
		if(children[i] != NULL)
			children[i]->print(os);
	}
}

