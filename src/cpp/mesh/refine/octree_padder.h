#ifndef OCTREE_PADDER_H
#define OCTREE_PADDER_H

/**
 * @file   octree_padder.h
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

#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>

/**
 * The octree_padder_t namespace is used to modify an octree structure
 *
 * Used to modify the structure of an octree in order to make further 
 * processing easier.
 *
 * The approach of 'padding' is to find any areas in the octree
 * where an interior node is adjacent to null space.  In this situation,
 * a dummy exterior node with no observations is inserted in the null
 * space.  This allows for processing later in the pipeline to comfortably
 * assume that any interior node is fully surrounded by non-null area,
 * which is useful for boundary processing.
 */
namespace octree_padder
{
	/*-------------------*/
	/* primary functions */
	/*-------------------*/

	/**
	 * Will pad all null space in an octree with dummy leafs
	 *
	 * This function will recursively search an octree.  If a
	 * node is found that is not a leaf, but has null children,
	 * these null slots will be filled with dummy leaf nodes
	 * that represent exterior nodes with no observations.
	 *
	 * The purpose behind this function is to allow for later
	 * processing to easily assume that any interior node is
	 * not adjacent to any null areas.  This property is important
	 * for finding and reconstructing the boundary of the environment
	 * in an efficient manner.
	 *
	 * @param tree   The octree to recursively modify
	 */
	void pad(octree_t& tree);

	/*------------------*/
	/* helper functions */
	/*------------------*/

	/**
	 * The helper recursive function for pad().
	 *
	 * This will recursively check octnodes, in order to
	 * process the entirety of a tree that is passed to pad(tree).
	 *
	 * @param node  The node to recursively check.
	 */
	void pad_recur(octnode_t* node);
}

#endif
