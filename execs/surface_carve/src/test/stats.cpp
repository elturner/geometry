#include "stats.h"
#include <vector>
#include <map>
#include <iostream>
#include "../structs/pose.h"
#include "../structs/point.h"
#include "../util/error_codes.h"

using namespace std;

void print_planar_region_stats(vector<planar_region_t>& regions)
{
	vector<planar_region_t>::iterator it;
	unsigned int num_tris, num_singles, num_zeros;

	/* initialize */
	num_tris = 0;
	num_singles = 0;
	num_zeros = 0;

	/* iterate over the regions */
	for(it = regions.begin(); it != regions.end(); it++)
	{
		num_tris += (*it).tris.size();
		if((*it).tris.size() == 1)
			num_singles++;
		if((*it).tris.size() == 0)
			num_zeros++;
	}

	/* finalize values */
	if(regions.empty() || regions.size() == num_zeros)
		num_tris = 0;
	else
		num_tris /= (regions.size() - num_zeros);

	/* print */
	cout << endl
	     << "---------------------------------------------" << endl 
	     << " Planar Region Statistics:" << endl
	     << endl
	     << "   # of regions:                     "
	     					<< regions.size() << endl
	     << "   # of \"large\" regions:             "
	     << (regions.size() - num_singles - num_zeros) << endl
	     << "   # of regions with 1 tri:          "
	     					<< num_singles << endl
	     << "   # of regions with 0 tris:         " 
	     					<< num_zeros << endl
	     << "   Average size of non-empty region: "
	     					<< num_tris << endl
	     << "---------------------------------------------" 
	     					<< endl << endl;
}
