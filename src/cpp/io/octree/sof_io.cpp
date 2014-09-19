#include "sof_io.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <util/error_codes.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

/**
 * @file    sof_io.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Functionality to export octrees to SOF files
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export octree information
 * to a SOF (Signed Octree Format) file.  This file is used for Tao
 * Ju's Dual Contour and Poly Mender code.
 *
 * @section FORMAT
 *
 * The following is his documentation on this format:
 *
 * IV SOF format
 *
 * SOF (Signed Octree Format) records an octree grid with signes attached
 * to the 8 corners of each leaf node. All leaf nodes appear at the same
 * depth that is specified by the <octree_depth> argument to the program.
 * The tree is recorded in SOF file using pre-order traversal. Here is 
 * the structure of a SOF file (binary):
 *
 * <header>
 *
 * <node>
 *
 * <header> is a 4-bytes integer that equals 2 ^ octree_depth. The first
 * byte of a <node> is either 0 (denoting an intermediate node) or 1 
 * (denoting an empty node) or 2 (denoting a leaf node). After the first 
 * byte, an intermediate node <node> contains eight <node> structures for 
 * its eight children; an empty node <node> contains one byte of value 0 
 * or 1 denoting if it is inside or outside; and a leaf node contains one
 * byte whose eight bits correspond to the signs at its eight corners (0
 * for inside and 1 for outside). The order of enumeration of the eight 
 * children nodes in an intermediate nodeis the following (expressed in 
 * coordinates <x,y,z> ): <0,0,0>,<0,0,1>,<0,1,0>,<0,1,1>,<1,0,0>,<1,0,1>,
 * <1,1,0>,<1,1,1>. The enumeration of the eight corners in a leaf node 
 * follows the same order (e.g., the lowest bit records the sign at 
 * <0,0,0>).
 *
 */

using namespace std;

/* The following constants are used in sof files */
static const unsigned char INTERMEDIATE_NODE = 0;
static const unsigned char EMPTY_NODE        = 1;
static const unsigned char LEAF_NODE         = 2;
static const unsigned char INSIDE            = 0;
static const unsigned char OUTSIDE           = 1;

/* this array converts from the child ordering in sof files
 * to the child ordering in the octree_t class */
static const size_t SOF_TO_OCTREE_ORDER[CHILDREN_PER_NODE]
				= {6, 2, 5, 1, 7, 3, 4, 0};

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int sof_io::writesof(const octree_t& tree, const string& filename)
{
	ofstream outfile;
	int ret;

	/* open binary file for writing */
	outfile.open(filename.c_str(), ios_base::out | ios_base::binary);
	if(!(outfile.is_open()))
	{
		cerr << "[sof_io::writesof]\tUnable to write to file: "
		     << filename << endl;
		return -1;
	}

	/* write header information */
	ret = sof_io::writesof_header(tree, outfile);
	if(ret)
	{
		cerr << "[sof_io::writesof]\tCan't write header info to: "
		     << filename << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* recursively write nodes to tree */
	ret = sof_io::writesof_node(tree.get_root(), outfile);
	if(ret)
	{
		cerr << "[sof_io::writesof]\tCan't write node information "
		     << "to: " << filename << endl;
		return PROPEGATE_ERROR(-3, ret);
	}

	/* clean up */
	outfile.close();
	return 0;
}
		
int sof_io::writesog(const octree_t& tree, const string& filename)
{
	return -1; /* TODO implement me */
}

int sof_io::writesof_header(const octree_t& tree, ostream& os)
{
	int d;

	/* get the depth of this octree */
	d = tree.get_max_depth();
	if(d < 0)
		/* tree not initialized; it has invalid depth */
		return -1;

	/* the sof file expects the value 2^depth */
	d = (1 << d);
	os.write((char*) &d, sizeof(d));

	/* success */
	return 0;
}

int sof_io::writesof_node(const octnode_t* node, ostream& os)
{
	unsigned char leafval, v;
	size_t i;
	int ret;

	/* check arguments */
	if(node == NULL)
	{
		/* we've reached null space, which is denoted
		 * as an "empty" node. */
		os.write((char*) &EMPTY_NODE, sizeof(EMPTY_NODE));
		os.write((char*) &OUTSIDE, sizeof(OUTSIDE));
	}
	else if(node->isleaf() || node->data != NULL)
	{
		/* this is a leaf node */
		leafval = 0;
		v = (node->data->is_interior() ? INSIDE : OUTSIDE);
		for(i = 0; i < CHILDREN_PER_NODE; i++)
			leafval |= (v << SOF_TO_OCTREE_ORDER[i]);

		/* write the value */
		os.write((char*) &LEAF_NODE, sizeof(LEAF_NODE));
		os.write((char*) &leafval, sizeof(leafval));	
	}
	else
	{
		/* this is an intermediate node */
		os.write((char*) &INTERMEDIATE_NODE, 
				sizeof(INTERMEDIATE_NODE));
		
		/* write children */
		for(i = 0; i < CHILDREN_PER_NODE; i++)
		{
			/* recurse to child, in order defined by
			 * .sof formatting */
			ret = sof_io::writesof_node(
					node->children[
					SOF_TO_OCTREE_ORDER[i]], os);
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
		}
	}

	/* success */
	return 0;
}

int sog_io::writesog_header(const octree_t& tree, ostream& os)
{
	return -1; // TODO implement me
}
		
int sog_io::writesog_node(const octnode_t* node, ostream& os)
{
	return -1; // TODO implement me
}
