#include "quadtree.h"
#include <geometry/quadtree/quadnode.h>
#include <geometry/quadtree/quaddata.h>
#include <geometry/shapes/linesegment_2d.h>
#include <Eigen/Dense>
#include <vector>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**
 * @file   quadtree.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains classes used to define a quadtree
 *
 * @section DESCRIPTION
 *
 * This file implements a quadtree structure.
 * The quadtree represents all of 2D space, and
 * the bounding box grows as more elements are added.
 */

using namespace std;
using namespace Eigen;

/* this macro is used to find the relative depth of two sized nodes */
#define GET_RELATIVE_DEPTH(rootsize, leafsize) \
            ( (int) round( log((rootsize) / (leafsize)) / log(2.0) ) )

/*--------------------*/
/* QUADTREE FUNCTIONS */
/*--------------------*/

quadtree_t::quadtree_t()
{
	/* make the simplest tree */
	this->root = NULL;
	this->max_depth = -1;
}

/* pass the max depth via grid res */
quadtree_t::quadtree_t(double r)
{
	Eigen::Vector2d c(0,0);

	/* make simplest tree, so max depth is zero */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
}

quadtree_t::~quadtree_t()
{
	this->clear();
}

void quadtree_t::set(double r, const Eigen::Vector2d& c, double hw)
{
	/* clear this */
	this->clear();

	/* set the values */
	this->root = new quadnode_t(c, hw);
	this->max_depth = max(GET_RELATIVE_DEPTH(2.0*hw, r), 0);
}

void quadtree_t::set_resolution(double r)
{
	Eigen::Vector2d c(0,0);

	/* clear the tree */
	if((this->root) != NULL)
		delete (this->root);

	/* remake tree */
	this->root = new quadnode_t(c, r/2);
	this->max_depth = 0;
}
	
double quadtree_t::get_resolution() const
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

quaddata_t* quadtree_t::insert(const Eigen::Vector2d& p,
			const Eigen::Vector2d& n, double w)
{
	int c;
	quadnode_t* nn;
	quaddata_t* ret;
	Vector2d np;

	/* verify root isn't null */
	if(!(this->root))
	{
		cerr << "[quadtree::insert]\tError: root is null" << endl;
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
		ret = this->root->insert(p, n, w, 0);
		if(ret == NULL)
		{
			/* error occurred, report to user */
			cerr << "[quadtree::insert]\tError inserting point"
			     << " into empty tree.  Uh-oh.  p = "
			     << p.transpose() << endl;
			return ret;
		}
	}
		
	/* first, check if this point goes out of bounds of
	 * this tree. */
	while(this->root->contains(p) < 0)
	{
		/* expand the tree in the direction of p
		 * by creating a wrapper node that will become
		 * the new root.  The first task is to determine
		 * which child of this wrapper the current root will
		 * be */
		if(this->root->center(0) < p(0))
		{
			if(this->root->center(1) < p(1))
			{
				/* child #2, lower left */
				c = 2;
				np(0) = this->root->center(0)
					+ this->root->halfwidth;
				np(1) = this->root->center(1)
					+ this->root->halfwidth;
			}
			else
			{
				/* child #1, upper left */
				c = 1;
				np(0) = this->root->center(0)
					+ this->root->halfwidth;
				np(1) = this->root->center(1)
					- this->root->halfwidth;
			}
		}
		else
		{
			if(this->root->center(1) < p(1))
			{
				/* child #3, lower right */
				c = 3;
				np(0) = this->root->center(0)
					- this->root->halfwidth;
				np(1) = this->root->center(1)
					+ this->root->halfwidth;
			}
			else
			{
				/* child #0, upper right */
				c = 0; 
				np(0) = this->root->center(0)
					- this->root->halfwidth;
				np(1) = this->root->center(1)
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
	ret = this->root->insert(p, n, w, this->max_depth);
	if(ret == NULL)
	{
		cerr << "[insert]\tError inserting point into tree" << endl;
		cerr << "\t\tp = " << p.transpose() << endl;
	}
	return ret;
}
	
quaddata_t* quadtree_t::insert(const Eigen::Vector2d& p,
				const Eigen::Vector2d& n,
				double z_min, double z_max, double w)
{
	quaddata_t* dat;

	/* insert this point */
	dat = this->insert(p, n, w);
	if(dat == NULL)
		return NULL;

	/* also add the height ranges */
	dat->add(z_min, z_max);

	/* success */
	return dat;
}

quaddata_t* quadtree_t::insert(const Eigen::Vector2d& p, size_t pose_ind)
{
	quaddata_t* dat;

	/* get the existing data for this point */
	dat = this->retrieve(p);
	if(dat == NULL)
		return NULL;

	/* add pose index to data */
	dat->pose_inds.insert(pose_ind);
	return dat;
}

quaddata_t* quadtree_t::retrieve(const Eigen::Vector2d& p) const
{
	/* just call root's retrieve function */
	const quadnode_t* node = this->root->retrieve(p);
	if(node == NULL)
		return NULL;
	return node->data;
}
		
void quadtree_t::subdivide(const Eigen::Vector2d& c, double hw)
{
	double xs[2];
	double ys[2];

	/* check if tree exists */
	if(this->root == NULL)
	{
		/* initialize the tree based on this geometry */
		this->set(2*hw, c, hw);
		return;
	}

	/* generate the bounds for the input square */
	xs[0] = c(0) - hw; /* min-x */
	xs[1] = c(0) + hw; /* max-x */
	ys[0] = c(1) - hw; /* min-y */
	ys[1] = c(1) + hw; /* max-y */

	/* subdivide the tree */
	this->root->subdivide(xs, ys, hw);
}
		
void quadtree_t::simplify()
{
	if(this->root == NULL)
		return; /* do nothing */

	/* simplify the tree structure recursively */
	this->root->simplify();
}
	
quaddata_t* quadtree_t::nearest_neighbor(const Eigen::Vector2d& p) const
{
	quaddata_t* best;
	int ret;

	/* search the root */
	best = NULL;
	ret = this->root->nearest_neighbor(&best, p);
	if(ret)
	{
		cerr << "[quadtree_t::nearest_neighbor]\t"
			"Call failed with error #" << ret << " at"
			" position: " << p.transpose() << endl;
		return NULL;
	}

	/* return the best node (will be null if tree empty) */
	return best;
}

int quadtree_t::neighbors_in_range(const Eigen::Vector2d& p, double r,
			vector<quaddata_t*>& neighs) const
{
	/* call root's function */
	return this->root->nodes_in_range(p, r, neighs);
}

void quadtree_t::raytrace(vector<quaddata_t*>& xings,
				const linesegment_2d_t& line) const
{
	/* call root's function */
	this->root->raytrace(xings, line);
}

void quadtree_t::print(ostream& os) const
{
	/* set up stream */
	os << setprecision(9) << fixed;

	/* print tree geometry to stream */
	os << this->max_depth << endl
	   << this->root->halfwidth << endl
	   << this->root->center(0) << " "
	   << this->root->center(1) << endl;

	/* print data of tree */
	this->root->print(os);
}
	
int quadtree_t::parse(istream& is)
{
	size_t i, num_points, num_poses, pose_ind;
	double hw, x, y, min_z, max_z;
	Vector2d center, p, n;
	quaddata_t* dat;

	/* clear the current tree */
	this->clear();

	/* parse tree structure from file */
	is >> this->max_depth;
	is >> hw;
	is >> x >> y;

	/* check validity */
	if(this->max_depth < 0)
	{
		cerr << "[quadtree_t::parse]\tInvalid Max Depth: " 
		     << this->max_depth << endl;
		return -1; /* invalid tree depth */
	}
	if(hw <= 0)
	{
		cerr << "[quadtree_t::parse]\tInvalid Root Halfwidth: " 
		     << hw << endl;
		return -2; /* invalid tree domain */
	}

	/* initialize root */
	center(0) = x;
	center(1) = y;
	this->root = new quadnode_t(center, hw);

	/* insert data */
	while(!(is.eof()))
	{
		/* read initial part of data */
		is >> x >> y >> min_z >> max_z >> num_points >> num_poses;
	
		/* verify */
		if(num_points == 0)
		{
			cerr << "[quadtree_t::parse]\tInvalid cell weight: "
			     << num_points << endl;
			return -3; /* invalid point */
		}

		/* insert point */
		p << x,y;
		dat = this->insert(p, n, min_z, max_z, num_points);
		if(dat == NULL)
		{
			cerr << "[quadtree_t::parse]\tUnable to insert "
			     << "new data." << endl;
			return -4;
		}

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
