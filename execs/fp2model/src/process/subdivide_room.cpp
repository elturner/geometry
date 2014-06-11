#include "subdivide_room.h"
#include <geometry/poly_intersect/poly2d.h>
#include <mesh/floorplan/floorplan.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include <set>

/**
 * @file subdivide_room.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Creates functions used to subdivide floorplan rooms
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to subdivide the geometry of
 * the rooms of floorplans.  These functions are necessary for exporting
 * to file formats that have a limit on the complexity of surfaces,
 * such as EnergyPlus IDF format.
 */

using namespace std;
using namespace fp;

/*---------------- function implementations ---------------*/

void bisect_room(fp::room_t& a, fp::room_t& b, const fp::room_t& r,
                 const fp::floorplan_t& f)
{
	int ai, bi;

	/* get seed triangles that are as far away from each other
	 * as possible */
	get_seeds(ai, bi, r, f);

	/* use these seeds to partition the set of triangles indicated
	 * in r into two subsets */
	partition_tri_sets(a, b, r, ai, bi, f);
}


/*-------------- helper functions ------------------*/

void get_seeds(int& ai, int& bi, const fp::room_t& r,
               const fp::floorplan_t& f)
{
	set<int>::const_iterator it, jt;
	double d, d_best, ix, iy, jx, jy, dx, dy;
	int i_best, j_best;

	/* iterate through all pairs of triangles in this room */
	d_best = -1;
	i_best = j_best = 0;
	for(it = r.tris.begin(); it != r.tris.end(); it++)
	{
		/* get center of triangle */
		poly2d::triangle_circumcenter(
			f.verts[f.tris[*it].verts[0]].x,
			f.verts[f.tris[*it].verts[0]].y,
			f.verts[f.tris[*it].verts[1]].x,
			f.verts[f.tris[*it].verts[1]].y,
			f.verts[f.tris[*it].verts[2]].x,
			f.verts[f.tris[*it].verts[2]].y,
			ix, iy);

		/* iterate over remaining triangles */
		for(jt = it; jt != r.tris.end(); jt++)
		{
			/* get center of triangles */
			poly2d::triangle_circumcenter(
				f.verts[f.tris[*jt].verts[0]].x,
				f.verts[f.tris[*jt].verts[0]].y,
				f.verts[f.tris[*jt].verts[1]].x,
				f.verts[f.tris[*jt].verts[1]].y,
				f.verts[f.tris[*jt].verts[2]].x,
				f.verts[f.tris[*jt].verts[2]].y,
				jx, jy);

			/* check distance of triangles */
			dx = (ix-jx);
			dy = (iy-jy);
			d = dx*dx + dy*dy;

			/* compare to best found so far */
			if(d > d_best)
			{
				/* store these results as best so far */
				i_best = *it;
				j_best = *jt;
				d_best = d;
			}
		}
	}

	/* return the best pair found, which is the pair of triangles
	 * that are farthest apart */
	ai = i_best;
	bi = j_best;
}

void partition_tri_sets(fp::room_t& a, fp::room_t& b, const fp::room_t& r,
                        int ai, int bi, const fp::floorplan_t& f)
{
	priority_queue<pair<double, int> > a_queue, b_queue;
	pair<double, int> p;
	edge_t e;
	double a_area, b_area;
	unsigned int i;
	bool add_to_a;
	int t, neigh;

	/* delete any info that's currently stored in subrooms */
	a.tris.clear();
	a.ind   = r.ind;
	a.min_z = r.min_z;
	a.max_z = r.max_z;
	
	b.tris.clear();
	b.ind   = r.ind;
	b.min_z = r.min_z;
	b.max_z = r.max_z;

	/* the queues store the pairs of <edge length, triangle index>,
	 * so the first triangles popped out of the queue are the ones that
	 * share the greatest edge with the pre-partitioned triangles */

	/* initialze the two subsets with the seed values */
	a_queue.push(pair<double, int>(0, ai));
	a_area = 0;
	
	b_queue.push(pair<double, int>(0, bi));
	b_area = 0;

	/* iterate over remaining triangles in the room, adding them
	 * to one partition or the other */
	while(!a_queue.empty() || !b_queue.empty())
	{
		/* determine which subpartition to add to */
		if(b_queue.empty())
			add_to_a = true;
		else if(a_queue.empty())
			add_to_a = false;
		else 
			add_to_a = (a_area < b_area);

		/* get next triangle for this subset */
		if(add_to_a)
		{
			/* increase the 'a' subset by adding another
			 * triangle to it */
			p = a_queue.top();
			a_queue.pop();
		}
		else
		{
			/* increase the 'b' subset by adding another
			 * triangle to it */
			p = b_queue.top();
			b_queue.pop();
		}

		/* check if this is actually a valid triangle to add */
		t = p.second;
		if(!r.tris.count(t) || a.tris.count(t) || b.tris.count(t))
			continue; /* not a valid triangle for this case */

		/* add triangle to the appropriate subset */
		if(add_to_a)
		{
			/* add triangle to subroom */
			a.tris.insert(t);
			a_area += f.compute_triangle_area(t);
		}
		else
		{
			/* add triangle to subroom */
			b.tris.insert(t);
			b_area += f.compute_triangle_area(t);
		}

		/* check neighbors for further expansion */
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		{
			/* add neighbors into queue */
			neigh = f.tris[t].neighs[i]; 
			e = f.tris[t].get_edge(i); 
			p = pair<double, int>(f.compute_edge_length(e),
						neigh);
		
			/* insert into appropriate queue */
			if(add_to_a)
				a_queue.push(p);
			else
				b_queue.push(p);
		}
	}
}
