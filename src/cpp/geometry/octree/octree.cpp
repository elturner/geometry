#include "octree.h"
#include "octnode.h"
#include "octdata.h"
#include "shape.h"
#include <util/error_codes.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <float.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace std;
using namespace Eigen;

/***** OCTTREE FUNCTIONS ****/

octree_t::octree_t()
{
	/* make the simplest tree */
	this->root = NULL;
	this->max_depth = -1;
}

/* pass the max depth via grid res */
octree_t::octree_t(double r)
{
	/* make simplest tree, so max depth is zero */
	this->root = NULL;
	this->set_resolution(r);
}

octree_t::~octree_t()
{
	this->clear();
}
	
void octree_t::set_resolution(double r)
{
	Vector3d c;

	/* clear the tree */
	if((this->root) != NULL)
		delete (this->root);

	/* remake tree */
	c << 0,0,0;
	this->root = new octnode_t(c, r/2);
	this->max_depth = 0;
}
	
double octree_t::get_resolution() const
{
	return (2.0 * this->root->halfwidth) / (1 << this->max_depth);
}
	
void octree_t::clear()
{
	if((this->root) != NULL)
	{
		delete (this->root);
		this->root = NULL;
	}
	this->max_depth = -1;
}
	
void octree_t::clone_from(const octree_t& other)
{
	/* destroy all data in this tree */
	this->clear();

	/* copy tree information */
	this->max_depth = other.max_depth;

	/* recursive clone nodes */
	this->root = other.root->clone();
}
		
int octree_t::include_in_domain(const Eigen::Vector3d& p)
{
	octnode_t* wrapper;
	Vector3d wc;
	int c;

	/* verify the root isn't null, which would imply that
	 * a resolution has not yet been set */
	if(this->root == NULL || this->max_depth < 0)
		return -1;

	/* check edge case of an empty tree.  In this case,
	 * it is easier to simply move the tree's origin to
	 * be centered at this point */
	if(this->root->data == NULL && this->max_depth == 0)
	{
		this->root->center = p;
		this->max_depth++;
		return 0;
	}

	/* check if the point is out-of-bounds of the current
	 * tree.  Only process if not. */
	while(this->root->contains(p) < 0)
	{
		/* expand the tree in the direction of p by creating
		 * a wrapper node that will become the new root.  The
		 * current root will become a child of this new wrapper
		 * node. The first task is to determine which child of
		 * this wrapper the current root will be. */
		if(this->root->center(2) < p(2))
		{
			/* original root is -z of this point, so
			 * it will be new child #4, 5, 6, or 7 */
			if(this->root->center(0) < p(0))
			{
				/* center is to -x of point, so child
				 * is either #5 or 6 */
				if(this->root->center(1) < p(1))
					c = 6;
				else
					c = 5;
			}
			else
			{
				/* center is to +x of point, so child
				 * is either #4 or 7 */
				if(this->root->center(1) < p(1))
					c = 7;
				else
					c = 4;
			}
		}
		else
		{
			/* original root is +z of point, so new child
			 * will be #0, 1, 2, or 3 */
			if(this->root->center(0) < p(0))
			{
				/* center is to -x of point, so child
				 * is either #1 or 2 */
				if(this->root->center(1) < p(1))
					c = 2;
				else
					c = 1;
			}
			else
			{
				/* center is to +x of point, so child
				 * is either #0 or 3 */
				if(this->root->center(1) < p(1))
					c = 3;
				else
					c = 0;
			}
		}

		/* get the center position of the new wrapper node, by
		 * negating the relative child position of the current
		 * root from it */
		wc = this->root->center 
			+ (this->root->halfwidth 
				* (-1 * relative_child_pos(c)));
		
		/* create new wrapper node */
		wrapper = new octnode_t(wc, 2 * this->root->halfwidth);
		wrapper->children[c] = this->root;
		
		/* expand size of tree */
		this->root = wrapper;
		this->max_depth++;
	}

	/* success */
	return 0;
}
		
void octree_t::find(const shape_t& s)
{
	/* find all leafs that intersect this shape */
	this->root->find(s);
}

int octree_t::insert(const shape_t& s)
{
	Vector3d p;
	unsigned int i, n;
	int ret;

	/* make sure all of shape is contained within this tree's domain */
	n = s.num_verts();
	for(i = 0; i < n; i++)
	{
		/* get vertex as a point */
		p = s.get_vertex(i);

		/* extend tree if necessary */
		ret = this->include_in_domain(p);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}

	/* carve the tree */
	this->root->insert(s, this->max_depth-1);

	/* success */
	return 0;
}

/* The following defines are used for reading and writing files */
#define OCTFILE_MAGIC_NUMBER "octfile"
#define OCTFILE_MAGIC_LENGTH 8

int octree_t::serialize(const string& fn) const
{
	ofstream outfile;
	unsigned int c;

	/* open binary file for writing */
	outfile.open(fn.c_str(), ios_base::out | ios_base::binary);
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* get number of nodes in tree */
	c = (this->root == NULL) ? 0 : this->root->get_num_nodes();
	
	/* export header information */
	outfile.write(OCTFILE_MAGIC_NUMBER, OCTFILE_MAGIC_LENGTH);
	outfile.write((char*) &(this->max_depth), sizeof(this->max_depth));
	outfile.write((char*) &c, sizeof(c));

	/* export tree information */
	this->root->serialize(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
	
int octree_t::parse(const string& fn)
{
	char magic[OCTFILE_MAGIC_LENGTH];
	ifstream infile;
	unsigned int c;
	int ret;

	/* open binary file for reading */
	infile.open(fn.c_str(), ios_base::in | ios_base::binary);
	if(!(infile.is_open()))
		return -1; /* could not open file */

	/* check magic number from header */
	infile.read(magic, OCTFILE_MAGIC_LENGTH);
	if(strcmp(magic, OCTFILE_MAGIC_NUMBER))
		return -2; /* not a valid octfile */
	
	/* get remainder of header information */
	infile.read((char*) &(this->max_depth), sizeof(this->max_depth));
	infile.read((char*) &c, sizeof(c)); /* count # of nodes in tree */

	/* destroy any existing information */
	if(this->root != NULL)
		delete (this->root);
	this->root = new octnode_t();

	/* read in tree information */
	ret = this->root->parse(infile);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* clean up */
	infile.close();
	return 0;
}
