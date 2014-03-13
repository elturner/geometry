#include "octnode.h"
#include "shape.h"
#include "octdata.h"
#include <Eigen/Dense>
#include <iostream>

/**
 * @file octnode.cpp
 * @auther Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the octnode_t class, which is used as the
 * primary tree element for the octree_t class.
 */

using namespace std;
using namespace Eigen;

/***** OCTNODE FUNCTIONS ****/

octnode_t::octnode_t()
{
	int i;

	/* set default values */
	this->center << 0,0,0;
	this->halfwidth = -1;
	this->data = NULL;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

octnode_t::octnode_t(const Vector3d& c, double hw)
{
	int i;

	/* default data value */
	this->data = NULL;
	
	/* set default geometry values */
	this->center = c;
	this->halfwidth = hw;

	/* set children to null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		this->children[i] = NULL;
}

octnode_t::~octnode_t()
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
	
bool octnode_t::isleaf() const
{
	int i;

	/* check if any children aren't null */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			return false;

	/* all children are null, must be leaf */
	return true;
}
	
void octnode_t::init_child(int i)
{
	double chw;
	Vector3d cc;

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
	cc = relative_child_pos(i)*chw + this->center; 
	
	/* create new node */
	this->children[i] = new octnode_t(cc, chw);
}
	
octnode_t* octnode_t::clone() const
{
	octnode_t* c;
	int i;

	/* allocate a new node */
	c = new octnode_t(this->center, this->halfwidth);

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

int octnode_t::contains(const Vector3d& p) const
{
	Vector3d diff;
	double d;

	/* check that p is within cube around center of this node */
	diff = (p - this->center);
	d = diff.cwiseAbs().maxCoeff();
	if(d > this->halfwidth)
		return -1; /* point is not in this node */

	/* check which side of xy plane the child is on */
	if(diff(2) >= 0)
	{
		/* z is positive,
		 * check which quadrant this is in */
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
		
	/* z is negative,
	 * check which quadrant this is in */
	if(diff(0) >= 0)
	{
		/* x is positive */
		if(diff(1) >= 0)
			return 4; /* upper right */
		return 7; /* lower right */
	}
		
	/* x is negative */
	if(diff(1) >= 0)
		return 5; /* upper left */
	return 6; /* lower left */
}
	
bool octnode_t::simplify()
{
	unsigned int i;
	bool thresh_met, t;

	/* check if every child exists and has non-null data */
	thresh_met = true; /* arbitrary initial value */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* check if child exists */
		if(this->children[i] == NULL)
			return false; /* cannot be simplified */
		
		/* check if child's data exist */
		if(this->children[i]->data == NULL)
			return false; /* cannot be simplified */

		/* check if child's data meets threshold requirements */
		if(this->children[i]->data->get_count() == 0)
			return false; /* not enough data */
	
		/* get label for child */
		t = this->children[i]->data->is_interior();
		if(i == 0)
			thresh_met = t; /* no other children compared yet */
		else if(t != thresh_met)
			return false; /* children disagree */
	}

	/* all checks have been passed, we can simplify this node */
	if(this->data == NULL)
		this->data = new octdata_t();
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* merge child data with this data, remove child */
		this->data->merge(this->children[i]->data);
		delete (this->children[i]);
		this->children[i] = NULL;
	}

	/* success */
	return true;
}
		
bool octnode_t::subdivide()
{
	return false; // TODO
}

octnode_t* octnode_t::retrieve(const Vector3d& p) const
{
	int i;

	/* get appropriate child */
	i = this->contains(p);
	if(i < 0)
		return NULL; /* point not even in this node, error */

	/* check if child exists */
	if(this->children[i] == NULL)
		return ((octnode_t*) this); /* no child, return current
		                             * node as deepest */
	return this->children[i]->retrieve(p); /* recurse through child */
}
		
void octnode_t::find(const shape_t& s)
{
	unsigned int i;

	/* check if we have reached a node with data */
	if(this->data != NULL)
		this->data = s.apply_to_leaf(this->center,
		                             this->halfwidth,
		                             this->data);
	
	/* recurse on any existent children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			this->children[i]->find(s);
}

void octnode_t::insert(const shape_t& s, int d)
{
	Vector3d child_center;
	unsigned int i;
	double chw;

	/* check if we reached final depth, or if this node
	 * has data already (in which case we want to stop carving
	 * deeper. */
	if(d <= 0 || this->data != NULL)
	{
		// TODO perform a 'should subdivide' check here

		/* reached final depth, apply to this node and finish */
		this->data = s.apply_to_leaf(this->center,
				this->halfwidth, this->data);
		return;
	}

	/* recurse over children */
	chw = this->halfwidth / 2; /* children are half the width */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* check if child exists */
		if(this->children[i] != NULL)
		{
			/* check for intersection */
			if(!s.intersects(this->children[i]->center,
				         this->children[i]->halfwidth))
				continue;
		}
		else
		{
			/* child does not exist. check
			 * if shape would intersect it. */
			child_center = relative_child_pos(i)*chw
					+ this->center; 
			if(!s.intersects(child_center, chw))
				continue;

			/* shape intersects it, so make the subnode */
			this->children[i] = new octnode_t(child_center,chw);
		}

		/* recurse */
		this->children[i]->insert(s, d-1);
	}

	/* attempt to simplify this node */
	this->simplify();
}	
		
unsigned int octnode_t::get_num_nodes() const
{
	unsigned int i, c;

	/* count self */
	c = 1;

	/* iterate over children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(this->children[i] != NULL)
			c += this->children[i]->get_num_nodes();

	/* return total */
	return c;
}
		
void octnode_t::serialize(ostream& os) const
{
	double d;
	unsigned int i;

	/* write out node geometry information */
	d = this->center(0); os.write((char*) &d, sizeof(d));
	d = this->center(1); os.write((char*) &d, sizeof(d));
	d = this->center(2); os.write((char*) &d, sizeof(d));
	os.write((char*) &(this->halfwidth), sizeof(this->halfwidth));

	/* write out data information */
	if(this->data != NULL)
	{
		os.put(1); /* data are available for this node */
		this->data->serialize(os);
	}
	else
		os.put(0); /* no data available */

	/* write out children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* write data for this child */
		if(this->children[i] == NULL)
			os.put(0); /* no child here */
		else
		{
			os.put(1); /* child exists */
			this->children[i]->serialize(os);
		}
	}
}
		
int octnode_t::parse(std::istream& is)
{
	double d;
	char c;
	unsigned int i;
	int ret;

	/* read in geometry information */
	is.read((char*) &d, sizeof(d)); this->center(0) = d;
	is.read((char*) &d, sizeof(d)); this->center(1) = d;
	is.read((char*) &d, sizeof(d)); this->center(2) = d;
	is.read((char*) &d, sizeof(d)); this->halfwidth = d;

	/* delete any existing data */
	if(this->data != NULL)
	{
		delete (this->data);
		this->data = NULL;
	}

	/* read in data information */
	is.get(c); /* flag states if node has data */
	if(c != 0)
	{
		/* read in data */
		this->data = new octdata_t();
		ret = this->data->parse(is);
		if(ret)
			return -1; /* could not read data */
	}

	/* read in child information */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* delete any existing child here */
		if(this->children[i] != NULL)
		{
			delete (this->children[i]);
			this->children[i] = NULL;
		}

		/* check if i'th child exists in file */
		is.get(c);
		if(c == 0)
			continue;

		/* read in child recursively in a depth-first manner */
		this->children[i] = new octnode_t();
		ret = this->children[i]->parse(is);
		if(ret)
			return -2; /* could not parse child */

	}

	/* success */
	return 0;
}
