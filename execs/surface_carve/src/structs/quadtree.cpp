#include "quadtree.h"
#include <math.h>
#include <stdlib.h>
#include "../util/parameters.h"
#include "../util/error_codes.h"

/**** QUADNODE FUNCTIONS ****/

quadnode_t::quadnode_t()
{
	int i;

	/* use default values */
	this->x = 0;
	this->y = 0;
	this->s = 1;
	this->f = false;

	/* set to be a leaf */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

quadnode_t::quadnode_t(double xx, double yy, int ss)
{
	int i;

	/* copy parameters */
	this->x = xx;
	this->y = yy;
	this->s = ss;
	this->f = false;

	/* set to be a leaf */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

quadnode_t::~quadnode_t()
{
	int i;

	/* free children */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		if(this->children[i])
		{
			delete (this->children[i]);
			this->children[i] = NULL;
		}
}

bool quadnode_t::is_leaf()
{
	int i;

	/* check edge case of null this */
	if(!this)
		return true;

	/* if any child present, not a leaf */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		if(this->children[i])
			return false;

	/* all children are null */
	return true;
}
	
bool quadnode_t::at_max_depth()
{
	return (this->s <= 1);
}

void quadnode_t::subdivide()
{
	int i, ns;
	double nsdt;

	/* first, verify this is a leaf */
	if(!(this->is_leaf()))
		return;

	/* verify we have room to subdivide */
	if(this->at_max_depth())
		return;
	ns = (this->s) / 2;
	nsdt = ns * 0.5;

	/* create a new child for each children pointer */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
	{
		/* initialize new node */
		this->children[i] = new quadnode_t(
			quadtree_child_arrangement[i][0]*nsdt + this->x,
			quadtree_child_arrangement[i][1]*nsdt + this->y,
			ns);
		this->children[i]->f = this->f;
	}
}

bool quadnode_t::is_inside(double xx, double yy)
{
	double dx, dy;
	double sot;

	/* find displacement to node */
	dx = xx - this->x;
	dy = yy - this->y;
	sot = ((double) this->s) / 2;

	/* check if inside box */
	return (-sot <= dx && dx < sot && -sot <= dy && dy < sot);
}

void quadnode_t::set_leaf_value(double xx, double yy, bool ff)
{
	int i;

	/* first, check if point in this node */
	if(!this->is_inside(xx, yy))
		return;

	/* next, check if we reached end of traversal */
	if(this->at_max_depth())
	{
		this->f = ff;
		return;
	}

	/* If got here, we need to keep traversing.  Subdivide this
	 * node and try this point on all children */
	this->subdivide();
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		this->children[i]->set_leaf_value(xx, yy, ff);
}

void quadnode_t::simplify()
{
	int i;
	bool will_simplify;

	/* first, check if leaf. If so, no simplification can be done. */
	if(this->is_leaf())
		return;

	/* simplify children */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
		this->children[i]->simplify();

	/* if all children are leaves with the same value, you
	 * can delete them, setting the current node to be a leaf
	 * with that same value. */
	will_simplify = true;
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE && will_simplify; i++)
	{
		/* check if current child fits characteristics
		 * -- Will only simplify true nodes */
		if(!(this->children[i]->is_leaf())
				|| !(this->children[i]->f))
		{
			will_simplify = false;
			break;
		}
	}

	/* check if we can simplify */
	if(!will_simplify)
		return;

	/* delete all children */
	for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
	{
		delete (this->children[i]);
		this->children[i] = NULL;
	}

	/* set value */
	this->f = true;
}

void quadnode_t::get_neighbors_under(vector<quadnode_t*>& neighs,
							quadnode_t* q)
{
	int i;

	/* first check edge case */
	if(this == q)
		return;

	/* check that the bounds of q touch the bounds of this node */
	if(q->x < this->x)
	{
		/* check if this node's RHS touching */
		if(q->x + q->s*0.5 + this->s*0.5 < this->x)
			return;
	}
	else
	{
		/* check if this node's LHS touching */
		if(this->x + this->s*0.5 + q->s*0.5 < q->x)
			return;
	}

	/* check bounds vertically */
	if(q->y < this->y)
	{
		/* check if this node's top side touching */
		if(q->y + q->s*0.5 + this->s*0.5 < this->y)
			return;
	}
	else
	{
		/* check if bottom side touching */
		if(this->y + this->s*0.5 + q->s*0.5 < q->y)
			return;
	}

	/* at this point we know q touches this node.  If q
	 * is a leaf, then just add it to our neighbors */
	if(q->is_leaf())
		neighs.push_back(q);
	else
	{
		/* test the children of q */
		for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
			this->get_neighbors_under(neighs, q->children[i]);
	}
}
	
int quadnode_t::edge_in_common(int& x1, int& y1, int& x2, int& y2,
							quadnode_t* q)
{
	int m_x_min, m_x_max, m_y_min, m_y_max;
	int q_x_min, q_x_max, q_y_min, q_y_max;

	/* get corners of both nodes */
	m_x_min = (int) round( this->x - this->s*0.5 );
	m_y_min = (int) round( this->y - this->s*0.5 );
	m_x_max = (int) round( this->x + this->s*0.5 );
	m_y_max = (int) round( this->y + this->s*0.5 );

	q_x_min = (int) round( q->x - q->s*0.5 );
	q_y_min = (int) round( q->y - q->s*0.5 );
	q_x_max = (int) round( q->x + q->s*0.5 );
	q_y_max = (int) round( q->y + q->s*0.5 );

	/* check the four possible sides the neighbor q could be on */
	if(m_x_max == q_x_min)
	{
		/* vertical edge going up */
		x1 = x2 = m_x_max;
		y1 = (m_y_min > q_y_min) ? m_y_min : q_y_min;
		y2 = (m_y_max < q_y_max) ? m_y_max : q_y_max;
	}
	else if(m_x_min == q_x_max)
	{
		/* vertical edge going down */
		x1 = x2 = m_x_min;
		y1 = (m_y_max < q_y_max) ? m_y_max : q_y_max;
		y2 = (m_y_min > q_y_min) ? m_y_min : q_y_min;
	}
	else if(m_y_max == q_y_min)
	{
		/* horizontal edge, going left */
		y1 = y2 = m_y_max;
		x1 = (m_x_max < q_x_max) ? m_x_max : q_x_max;
		x2 = (m_x_min > q_x_min) ? m_x_min : q_x_min;
	}
	else if(m_y_min == q_y_max)
	{
		/* horizontal edge, going right */
		y1 = y2 = m_y_min;
		x1 = (m_x_min > q_x_min) ? m_x_min : q_x_min;
		x2 = (m_x_max < q_x_max) ? m_x_max : q_x_max;
	}
	else
	{
		/* error, q not a neighbor */
		return -1;
	}

	/* success */
	return 0;
}

void quadnode_t::triangulate(vector<quadtri_t>& tris, quadnode_t* root)
{
	vector<quadnode_t*> neighs;
	bool min_feature;
	int ret, i, n, x1, x2, y1, y2, x_mid, y_mid;

	/* If this node is not a leaf, then just
	 * recursively triangulate its children */
	if(!(this->is_leaf()))
	{
		/* call on children */
		for(i = 0; i < QUADTREE_CHILDREN_PER_NODE; i++)
			this->children[i]->triangulate(tris, root);
		return;
	}

	/* we know this is a leaf, but check if it's a leaf we
	 * care about. */
	if(!(this->f))
		return;

	/* Here, we know this node is a leaf, so we need to
	 * generate triangles based on its neighbors. */

	/* first, find those neighbors */
	this->get_neighbors_under(neighs, root);

	/* there are two ways to triangulate a node.  If the
	 * node is smaller than all of its neighbors, a simple
	 * two triangles will do */
	min_feature = true;
	n = neighs.size();
	for(i = 0; i < n; i++)
		if(neighs[i]->s < this->s)
		{
			/* found a neighboring node smaller than this
			 * one, which means we cannot use a simple
			 * triangulation */
			min_feature = false;
			break;
		}

	/* check which type of triangulation we can perform on this node */
	if(min_feature)
	{
		/* find positions of corners */
		x1 = (int) round( this->x - this->s*0.5 );
		y1 = (int) round( this->y - this->s*0.5 );
		x2 = (int) round( this->x + this->s*0.5 );
		y2 = (int) round( this->y + this->s*0.5 );

		/* add triangles with vertices at the corners 
		 * of this node */
		tris.push_back(quadtri_t());
		tris.back().v[0].set(x2, y2);
		tris.back().v[1].set(x1, y2);
		tris.back().v[2].set(x1, y1);
		tris.push_back(quadtri_t());
		tris.back().v[0].set(x2, y2);
		tris.back().v[1].set(x1, y1);
		tris.back().v[2].set(x2, y1);
	}
	else
	{
		/* a more complex triangulation is required.
		 * Fortunately, the node is a square, so placing
		 * a vertex at the center ensures that a triangulation
		 * can be formed simply.  Note that this case only happens
		 * at node sizes larger than the smallest, so the center
		 * point of a node can also be expressed with integer
		 * coordinates. */
		x_mid = (int) ( this->x );
		y_mid = (int) ( this->y );

		/* Create a triangle connecting the above point to
		 * the shared edges of all neighbors */
		for(i = 0; i < n; i++)
		{
			ret = this->edge_in_common(x1, y1, x2, y2, 
							neighs[i]); 
			if(ret)
			{
				/* error occurred, i'th node is not
				 * a neighbor */
				PRINT_ERROR("ERROR IN TRIANGULATING "
							"QUADNODE");
				continue;
			}
		
			/* create the triangle with these vertices */
			tris.push_back(quadtri_t());
			tris.back().v[0].set(x_mid, y_mid);
			tris.back().v[1].set(x1, y1);
			tris.back().v[2].set(x2, y2);
		}
	}
}

/**** QUADTREE FUNCTIONS ****/

quadtree_t::quadtree_t()
{
	this->root.s = 1;
	this->triangles.clear();
}

quadtree_t::quadtree_t(int s)
{
	this->root.s = s;
	this->triangles.clear();
}

quadtree_t::~quadtree_t()
{
	/* Note:  root node will clear itself */
}

void quadtree_t::fill_point(double x, double y, bool f)
{
	this->root.set_leaf_value(x, y, f);
}

void quadtree_t::triangulate()
{
	/* clear any existing triangles */
	this->triangles.clear();

	/* verify that the tree is simplified */
	this->root.simplify();

	/* triangulate the root */
	this->root.triangulate(this->triangles, &(this->root));
}
