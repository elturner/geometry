#include "hia_analyzer.h"
#include <io/hia/hia_io.h>
#include <geometry/shapes/bounding_box.h>
#include <geometry/hist/hia_cell_info.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <set>

/**
 * @file   hia_analyzer.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports data from hia file, performs room analysis
 *
 * @section DESCRIPTION
 *
 * The hia_analyzer_t class will perform geometric analysis on the
 * contents of a hia file in order to facilitate floorplan generation.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int hia_analyzer_t::readhia(const std::string& filename)
{
	pair<cellmap_t::iterator, bool> ins;
	hia::reader_t infile;
	hia::cell_t c;
	hia_cell_index_t index;
	hia_cell_info_t info;
	unsigned int i;
	tictoc_t clk;
	int ret;

	/* open the file */
	tic(clk);
	ret = infile.open(filename);
	if(ret)
	{
		cerr << "[hia_analyzer_t::readhia]\tUnable to open hia "
		     << "file: " << filename << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* store header data */
	this->level = infile.level_index();
	this->bounds.init(infile.x_min(), infile.y_min(), infile.z_min(),
			infile.x_max(), infile.y_max(), infile.z_max());
	if(!(this->bounds.is_valid()))
	{
		cerr << "[hia_analyzer_t::readhia]\tHia file has invalid "
		     << "bounds: " << filename << endl;
		return -2;
	}
	this->resolution = infile.resolution();
	if(this->resolution <= 0)
	{
		cerr << "[hia_analyzer_t::readhia]\tHia file has invalid "
		     << "resolution: " << filename << endl;
		return -3;
	}

	/* import cell data from file */
	this->cells.clear();
	for(i = 0; i < infile.num_cells(); i++)
	{
		/* attempt to get next cell */
		ret = infile.next(c);
		if(ret)
		{
			cerr << "[hia_analyzer_t::readhia]\tUnable to "
			     << "import cell #" << i << "/" 
			     << infile.num_cells() << " of file: "
			     << filename << endl;
			return PROPEGATE_ERROR(-4, ret);
		}

		/* prepare index of the next cell */
		info.init(c);
		index = this->get_index_of(info.center);

		/* store this cell in our analyzer structure */
		ins = this->cells.insert(
				pair<hia_cell_index_t, 
				hia_cell_info_t>(index,info));
		if(!(ins.second))
		{
			/* overlapping cells should not happen */
			cerr << "[hia_analyzer_t::readhia]\tEncountered "
			     << "overlapping cells at index ("
			     << index.x_ind << ", " << index.y_ind << ") "
			     << "for file: " << filename << endl;
			return -5;
		}
	}

	/* clean up */
	infile.close();
	toc(clk, "Reading hia file");
	return 0;
}
		
int hia_analyzer_t::neighbors_within(const hia_cell_index_t& seed,
					double dist, 
				std::set<hia_cell_index_t>& neighs) const
{
	cellmap_t::const_iterator seed_it, curr_it;
	set<hia_cell_index_t> local;
	set<hia_cell_index_t>::iterator lit;
	queue<hia_cell_index_t> qu;
	hia_cell_index_t curr;

	/* get information about this seed */
	seed_it = this->cells.find(seed);
	if(seed_it == this->cells.end())
	{
		cerr << "[hia_analyzer_t::neighbors_within]\tSeed is not "
		     << "in map!" << endl;
		return -1;
	}

	/* insert the seed into the queue */
	qu.push(seed);

	/* keep processing as long as stuff is in the queue */
	while(!(qu.empty()))
	{
		/* get the next index from the queue */
		curr = qu.front();
		qu.pop();

		/* get the info for this element */
		curr_it = this->cells.find(curr);
		if(curr_it == this->cells.end())
		{
			/* the current cell is not in the map,
			 * so stop searching */
			continue;
		}

		/* check if the current cell is within the search
		 * distance of the seed */
		if( (seed_it->second.center 
				- curr_it->second.center).norm() > dist)
			continue; /* too far, ignore it */

		/* the current cell is valid and within the search
		 * distance, so add it to our list of neighbors */
		neighs.insert(curr);

		/* now search the neighbors of the current cell */
		local.clear();
		curr.get_neighs(local);
		for(lit = local.begin(); lit != local.end(); lit++)
			qu.push(*lit);
	}
	
	/* success */
	return 0;
}
