#ifndef SET_OPS_H
#define SET_OPS_H

/**
 * @file    set_ops.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Wrapper around set operations for STL sets
 *
 * @section DESCRIPTION
 *
 * This file defines functions used to modify sets
 */

#include <algorithm>
#include <iterator>
#include <set>


/**
 * This namespace contains wrappers around common set operations
 */
namespace set_ops
{
	/**
	 * Performs set intersection
	 *
	 * Will store the intersection of two sets in the given output
	 *
	 * @param AIB      The output of intersecting A and B
	 * @param A        The first set
	 * @param B        The second set
	 */
	template<typename T> inline void intersect(std::set<T>& AIB,
				const std::set<T>& A, const std::set<T>& B)
	{
		/* clear outgoing set */
		AIB.clear();

		/* perform intersection */
		std::set_intersection( 
			A.begin(), A.end(), B.begin(), B.end(),
			std::insert_iterator< std::set<T> >(AIB, 
						AIB.begin()));
	};
}

#endif
