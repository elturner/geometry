#include "union_find.h"
#include <cstddef>
#include <vector>
#include <map>

/**
 * @file union_find.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Classes to perform the union-find algorithm
 *
 * @section DESCRIPTION
 *
 * This file contains the union_find_t class, which is used to 
 * perform the union-find algorithm.  This algorithm is used to 
 * identify the set of connected components in a graph.
 *
 * This library is used by specifying the number of nodes in a graph,
 * and the edge connections between each node.
 */

using namespace std;

/*-------------------------------------*/
/* union-find function implementations */
/*-------------------------------------*/

void union_find_t::init(size_t N)
{
	size_t i;

	/* clear any existing data */
	this->forest.resize(N);
	for(i = 0; i < N; i++)
		forest[i] = i; /* make everything a root */
}
		
int union_find_t::add_edge(size_t a, size_t b)
{
	size_t ra, rb, N;

	/* verify input represent valid nodes */
	N = this->forest.size();
	if(a >= N || b >= N)
		return -1; /* error, invalid indices */

	/* get the root indices of the given nodes */
	ra = this->get_root(a);
	rb = this->get_root(b);

	/* to connect these nodes, set the roots to be
	 * the same */
	this->forest[rb] = ra;

	/* success */
	return 0;
}
		
void union_find_t::get_unions(vector<vector<size_t> >& unions)
{
	map<size_t, size_t> roots2unions;
	size_t i, n, r;

	/* count the number of roots in the tree */
	n = this->forest.size();
	for(i = 0; i < n; i++)
	{
		/* check if i'th node is a root */
		r = this->get_root(i);
		if(r != i)
			continue;

		/* add root to map */
		roots2unions[r] = roots2unions.size();
	}

	/* prepare output structure */
	unions.clear();
	unions.resize(roots2unions.size());
	for(i = 0; i < n; i++)
	{
		/* add the i'th node to the appropriate union */
		unions[roots2unions[this->get_root(i)]].push_back(i);
	}
}

/*------------------*/
/* helper functions */
/*------------------*/

int union_find_t::get_root(size_t i)
{
	size_t r, p;

	/* get parent of i */
	p = forest[i];

	/* check if i is a root */
	if(p == i)
		return i;

	/* i is not a root, which means we
	 * need to recurse until we find its
	 * root. */
	r = this->get_root(i);

	/* we can simplify the forest to improve
	 * performance on the next call */
	forest[i] = r;
	return r;
}
