#include "octree_padder.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>

/**
 * @file   octree_padder.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will insert additional exterior leaf nodes into an octree
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to modify the structure of an
 * octree in order to make further processing easier.
 *
 * The approach of 'padding' is to find any areas in the octree
 * where an interior node is adjacent to null space.  In this situation,
 * a dummy exterior node with no observations is inserted in the null
 * space.  This allows for processing later in the pipeline to comfortably
 * assume that any interior node is fully surrounded by non-null area,
 * which is useful for boundary processing.
 */

using namespace octree_padder;
using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void octree_padder::pad(octree_t& tree)
{
	/* just call the recursive function, given the root of the tree */
	pad_recur(tree.get_root());
}
	
void octree_padder::pad_recur(octnode_t* node)
{
	size_t i;

	/* stop if we reach a null node */
	if(node == NULL)
		return;

	/* also stop if we reach a leaf node */
	if(node->isleaf())
	{
		/* since this node is a leaf, it should
		 * have some data */
		if(node->data == NULL)
			node->data = new octdata_t();
		
		/* since it's a leaf, we don't need to proceed */
		return;
	}

	/* now that we've confirmed the node is not a leaf, if
	 * any of its children are null, then we know some of the
	 * other children cannot be null.  Thus, we should pad any
	 * null child found. */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
	{
		/* check if child exists */
		if(node->children[i] == NULL)
			node->init_child(i); /* pad this child */

		/* child isn't null, so we
		 * need to recurse to explore the full
		 * tree */
		octree_padder::pad_recur(node->children[i]);
	}
}
