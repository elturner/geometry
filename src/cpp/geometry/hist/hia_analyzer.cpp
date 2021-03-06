#include "hia_analyzer.h"
#include <io/hia/hia_io.h>
#include <geometry/shapes/bounding_box.h>
#include <geometry/hist/hia_cell_info.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <float.h>

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
	this->rooms.clear();
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
			/* overlapping cells should not happen 
			 *
			 * But if they do, then just take one of
			 * them and ignore the other. */
		}
	}

	/* clean up */
	infile.close();
	toc(clk, "Reading hia file");
	return 0;
}
		
double hia_analyzer_t::get_open_height_at(const Eigen::Vector2d& p) const
{
	hia_cell_index_t ind;
	cellmap_t::const_iterator it;

	/* get the index of this posiiton */
	ind = this->get_index_of(p);

	/* get the info at this index */
	it = this->get_info_for(ind);
	if(it == this->end())
		return -1; /* invalid */

	/* return the open height amount */
	return it->second.open_height;
}
		
int hia_analyzer_t::populate_neighborhood_sums(double dist)
{
	cellmap_t::iterator it, neigh_it;
	set<hia_cell_index_t> neighs;
	set<hia_cell_index_t>::iterator nit;
	progress_bar_t progbar;
	size_t num_so_far;
	tictoc_t clk;
	int ret;

	/* begin processing */
	tic(clk);
	progbar.set_name("Neighbor sums");
	num_so_far = 0;

	/* iterate over the cells */
	for(it = this->cells.begin(); it != this->cells.end(); it++)
	{
		/* update progress bar */
		progbar.update(num_so_far++, this->cells.size());

		/* reset the sum for this cell */
		it->second.neighborhood_sum = 0;

		/* find the neighbors of this cell */
		neighs.clear();
		ret = this->neighbors_within(it->first, dist, neighs);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	
		/* compute the sum for this cell */
		for(nit = neighs.begin(); nit != neighs.end(); nit++)
		{
			/* get neigh info */
			neigh_it = this->cells.find(*nit);
			if(neigh_it == this->cells.end())
				return PROPEGATE_ERROR(-2, ret);

			/* add to sum */
			it->second.neighborhood_sum 
				+= neigh_it->second.open_height;
		}
	}

	/* clean up */
	progbar.clear();
	toc(clk, "Computing neighbor sums");
	return 0;
}
		
int hia_analyzer_t::label_local_maxima(double dist)
{
	pair<roommap_t::iterator, bool> ins;
	cellmap_t::iterator it, neigh_it;
	set<hia_cell_index_t> neighs;
	set<hia_cell_index_t>::iterator nit;
	progress_bar_t progbar;
	size_t num_so_far, num_localmaxes;
	bool islocalmax;
	tictoc_t clk;
	int ret;

	/* begin processing */
	tic(clk);
	progbar.set_name("Labeling local max");
	num_so_far = 0;
	num_localmaxes = 0;

	/* iterate over the cells */
	for(it = this->cells.begin(); it != this->cells.end(); it++)
	{
		/* update progress bar */
		progbar.update(num_so_far++, this->cells.size());

		/* find the neighbors of this cell */
		neighs.clear();
		ret = this->neighbors_within(it->first, dist, neighs);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	
		/* iterate over the neighbors, checking if the current
		 * cell has the largest sum */
		islocalmax = true;
		for(nit = neighs.begin(); nit != neighs.end(); nit++)
		{
			/* get neigh info */
			neigh_it = this->cells.find(*nit);
			if(neigh_it == this->cells.end())
				return PROPEGATE_ERROR(-2, ret);
			
			/* compare the sums */
			if(it->second.neighborhood_sum 
					< neigh_it->second.neighborhood_sum)
			{
				/* a neighbor is larger, this cell is NOT
				 * a local max */
				islocalmax = false;
				it->second.room_index = -1;
				break;
			}
			else if(it->second.neighborhood_sum 
				== neigh_it->second.neighborhood_sum)
			{
				/* the neighbor has the same sum.  we should
				 * only count one of them as a local max,
				 * so choose the one that is smaller. */
				if(neigh_it->first < it->first)
				{
					/* the other cell is smaller, it
					 * should be the local max, not
					 * this one */
					islocalmax = false;
					it->second.room_index = -1;
					break;
				}
			}
			else
			{ /* this cell is larger, keep searching */ }
		}

		/* finished with the loop, check if we are local max */
		if(islocalmax)
		{
			/* make a unique index for this localmax */
			it->second.room_index = num_localmaxes;
			num_localmaxes++;

			/* add as a new room */
			ins = this->rooms.insert(pair<hia_cell_index_t,
				hia_room_info_t>(it->first,
					hia_room_info_t(it->first)));
			if(!(ins.second))
				return -3;
		}
	}

	/* clean up */
	progbar.clear();
	toc(clk, "Computing local maxima");
	return 0;
}

/* the following defines the class used in the priority queue 
 * in the function propegate_room_labels */
class pq_dist_val_t
{
	/* parameters */
	public:

		/* the distance of current cell from seed point */
		double dist;

		/* the current cell */
		hia_cell_index_t curr;

		/* the seed cell */
		hia_cell_index_t seed;

	/* functions */
	public:

		/** 
		 * make a new dist value from given params
		 *
		 * @param s      The original seed
		 * @param prev_dist_to_seed   The previous distance value
		 * @param next_oh   The next open_height value
		 * @param c      The new cell to add to chain
		 */
		pq_dist_val_t(const hia_cell_index_t& s,
				double prev_dist_to_seed,
				double next_oh,
				const hia_cell_index_t& c)
		{
			/* store indices */
			this->seed = s;
			this->curr = c;

			/* the distance between seeds is the sum of
			 * the inverses of the open_height values of
			 * each cell in the chain.
			 *
			 * If a cell's open_height is 0, then the
			 * distance is infinite.
			 */
			if(next_oh <= 0)
				this->dist = DBL_MAX;
			else
				this->dist = prev_dist_to_seed + 1/next_oh;
		};

		/* operator indicates how to sort 
		 *
		 * We want the smallest values to come up
		 * in the queue first, so sort backwards. */
		inline bool operator < (const pq_dist_val_t& other) const
		{ return this->dist > other.dist; };

		/**
		 * Copy operator
		 */
		inline pq_dist_val_t& operator= (const pq_dist_val_t& other)
		{
			this->seed = other.seed;
			this->curr = other.curr;
			this->dist = other.dist;

			return *this;
		};
};
		
int hia_analyzer_t::propegate_room_labels()
{
	roommap_t::iterator rit;
	cellmap_t::iterator cit, sit;
	priority_queue<pq_dist_val_t> pq;
	set<hia_cell_index_t> local;
	set<hia_cell_index_t>::iterator lit;
	progress_bar_t progbar;
	tictoc_t clk;
	size_t num_labeled;

	/* begin processing */
	tic(clk);
	progbar.set_name("Labeling rooms");
	num_labeled = 0;

	/* iterate through all room seeds, adding their neighbors to
	 * the priority queue */
	for(rit = this->rooms.begin(); rit != this->rooms.end(); rit++)
	{
		/* get the neighbors for the seed cell of this room */
		local.clear();
		rit->first.get_neighs(local);

		/* get info for this seed */
		cit = this->cells.find(rit->first);
		if(cit == this->cells.end())
			return -1; /* can't find seed info */

		/* this seed is already in its room */
		progbar.update(num_labeled++, this->cells.size());

		/* add all these neighbors to the priority queue */
		for(lit = local.begin(); lit != local.end(); lit++)
		{
			/* add this cell as a potential next-step 
			 *
			 * Note that these neighbor cells may not
			 * actually exist, so the next part of this
			 * function needs to appropriately filter
			 * those out. */
			pq.push(pq_dist_val_t(cit->first, 0,
					cit->second.open_height, *lit));
		}
	}

	/* now that we've populated the priority queue, keep running
	 * until we run out of cells */
	while(!(pq.empty()))
	{
		/* get the next value from the queue */
		pq_dist_val_t curr = pq.top();
		pq.pop();

		/* get the info for this cell */
		cit = this->cells.find(curr.curr);
		if(cit == this->cells.end())
			continue; /* can't find info, not valid cell */

		/* check if this value has already been taken */
		if(cit->second.room_index >= 0)
			continue; /* already in a room */

		/* add this cell to the room it's closest to,
		 * which would be its seed room */
		rit = this->rooms.find(curr.seed);
		if(rit == this->rooms.end())
			return -3; /* can't find room */

		/* also get info for the room seed itself */
		sit = this->cells.find(curr.seed);
		if(sit == this->cells.end())
			return -4; /* can't find info for room seed */

		/* add cell to room */
		rit->second.insert(cit->first);
		
		/* set the room index of this cell so that we know
		 * it has been assigned */
		cit->second.room_index = sit->second.room_index;
		
		/* we've now labeled another cell */
		progbar.update(num_labeled++, this->cells.size());

		/* now that the current cell has been added to a room,
		 * check all of its neighbors to see if they should
		 * also be added to this room. */
		local.clear();
		cit->first.get_neighs(local);
		for(lit = local.begin(); lit != local.end(); lit++)
		{
			/* insert this neighbor into the queue to
			 * check.  Its distance should be increased
			 * from the current cell based on the open_height
			 * value of the current cell. */
			pq.push(pq_dist_val_t(
				sit->first, /* the room seed */
				curr.dist, /* distance of current cell */
				cit->second.open_height, /* delta dist */
				*lit)); /* the next cell to check */
		}
	}

	/* clean up */
	progbar.clear();
	toc(clk, "Labeling rooms");
	return 0;
}
		
void hia_analyzer_t::write_neighborhood_sums(std::ostream& os) const
{
	cellmap_t::const_iterator it;

	/* iterate over the cells */
	for(it = this->cells.begin(); it != this->cells.end(); it++)
	{
		/* write info */
		os << it->first.x_ind << " " << it->first.y_ind << " "
			<< it->second.neighborhood_sum << endl;
	}
}
		
void hia_analyzer_t::write_localmax(std::ostream& os) const
{
	cellmap_t::const_iterator it;

	/* iterate over the cells */
	for(it = this->cells.begin(); it != this->cells.end(); it++)
	{
		/* check if room index is valid */
		if(it->second.room_index < 0)
			continue; /* skip it */

		/* write info */
		os << it->second.center.transpose() << " "
			<< it->second.room_index << endl;
	}
}
		
int hia_analyzer_t::neighbors_within(const hia_cell_index_t& seed,
					double dist, 
			std::set<hia_cell_index_t>& neighs) const
{
	pair<set<hia_cell_index_t>::iterator, bool> ins;
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
		ins = neighs.insert(curr);
		if(!(ins.second))
		{
			/* Insert failed because cell already counted
			 * as a neighbor.  Since we've already seen it
			 * don't bother proceeding. */
			continue;
		}

		/* now search the neighbors of the current cell */
		local.clear();
		curr.get_neighs(local);
		for(lit = local.begin(); lit != local.end(); lit++)
			qu.push(*lit);
	}
	
	/* success */
	return 0;
}
