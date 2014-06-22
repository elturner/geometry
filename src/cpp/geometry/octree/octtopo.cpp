#include "octtopo.h"
#include "octree.h"
#include "octnode.h"
#include <vector>

/**
 * @file octtopo.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The octtopo_t class is used for computing octree topology
 *
 * @section DESCRIPTION
 *
 * This file contains the octtopo_t class, which is used to provide
 * additional representations of octree and octnode topology.  Its
 * main purpose is to allow for relative neighbor linkages between
 * adjacent nodes.
 *
 * While this class is not part of the octree structure directly, it
 * can be provided with an octree to be initialized, and used to augment
 * an existing tree structure information.
 */

using namespace std;
using namespace octtopo;

/* octtopo_t function implementations */

void octtopo_t::init(const octree_t& tree)
{
	// TODO
}

// TODO

