#ifndef H_BINARYSEARCH_H
#define H_BINARYSEARCH_H

/**
 * @file binary_search.h
 * @author Nick Corso <ncorso@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file provides binary search functions on sorted vectors
 */

/* includes */
#include <vector>
#include <iostream>

/* the binary_search namespace */
namespace binary_search 
{

	/**
	 * Performs binary search on sorted vector of comparible elements
	 *
	 * Given a sorted vector of comparible elements, will search for
	 * the closest index to the query element that is less than
	 * the query element.
	 *
	 * @param inVect  The sorted vector to search
	 * @param query   The query element to compare
	 *
	 * @return  Returns the closest index in inVect to query that whose
	 *          value is less than query
	 */
	template <typename T> unsigned int binary_search(
	                                       const std::vector<T>& inVect,
	                                       T query)
	{
		unsigned int low, high, mid, last;

		/* Check for the empty case */
		if(inVect.size() == 0)
			return 0;
	
		/* Check edge cases */
		if(query < inVect[0]) 
			return 0;
		last = inVect.size()-1;
		if(query > inVect[last]) 
			return last;

		/* Perform Binary Search */
		low = 0;
		high = inVect.size();
		while(low <= high)
		{
			/* Find the middle of the range */
			mid = (low + high)/2;
	
			/* Subdivide */
			if(inVect[mid] < query)
				low = mid+1;
			else if(inVect[mid] > query)
			{
				high = mid - 1;
	
				/* check if high is now less than t */
				if((mid != 0) && inVect[high] < query)
				{
					low = high;
					break;
				}
			}
			else
				return mid;
		}
	
		/* We know query falls between indicies low and low+1 */
		return low;
	}

	/**
	 * Performs binary search on sorted vector of comparible elements
	 *
	 * Given a sorted vector of doubles it will find the closest element
	 *
	 * @param inVect  The sorted vector to search
	 * @param query   The query element to compare
	 *
	 * @return  Returns the closest index in inVect to query 
	 */
	inline unsigned int get_closest_index(
			const std::vector<double>& inVect, 
			double query)
	{

		unsigned int idx = binary_search<double>(inVect, query);
		
		// Decide between idx and idx+1
		if(idx == inVect.size()-1)
			return idx;
		
		if( ( inVect[idx+1] - query ) > (query - inVect[idx]) ) {
			return idx;
		}
		else {
			return idx+1;
		}
	}

}

#endif



