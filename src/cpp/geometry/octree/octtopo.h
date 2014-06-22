#ifndef OCTTOPO_H
#define OCTTOPO_H

/**
 * @file octtopo.h
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

#include "octree.h"
#include "octnode.h"
#include <vector>

/* the following classes are defined in this file */
class octtopo_t;
class octneighbors_t;

/* Each octnode can have neighbors on all six faces.  The
 * relative positions of the nodes are defined in the octnode_t
 * class.
 *
 * Cube description:
 *         7 ________ 6           _____6__             ________
 *         /|       /|         7/|       /|          /|       /|
 *       /  |     /  |        /  |     /5 |        /  6     /  |
 *   4 /_______ /    |      /__4____ /    10     /_______3/    |
 *    |     |  |5    |     |    11  |     |     |     |  |   2 |
 *    |    3|__|_____|2    |     |__|__2__|     | 4   |__|_____|
 *    |    /   |    /      8   3/   9    /      |    /   |    /
 *    |  /     |  /        |  /     |  /1       |  /     5  /
 *    |/_______|/          |/___0___|/          |/_1_____|/
 *   0          1        0          1
 *
 */
// TODO

/*-----------------------------------------------------*/
/*-------------- class declarations -------------------*/
/*-----------------------------------------------------*/

/**
 * The octtopo_t class represents the octnodes' neighbor topology
 */
class octtopo_t
{
	/* parameters */
	private:

		// TODO

	/* functions */
	public:

		// TODO
};

/**
 * The octneighbors_t class represents the neighbors of a signle octnode
 */
class octneighbors_t
{
	/* security */
	friend class octtopo_t;

	/* parameters */
	private:

		// TODO
	
	/* functions */
	public:

		// TODO
};

#endif
