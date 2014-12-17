#include "cell_graph.h"
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include <vector>
#include <ostream>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include "quadtree.h"
#include "../structs/point.h"
#include "../structs/normal.h"
#include "../rooms/tri_rep.h"
#include "../util/error_codes.h"
#include "../util/constants.h"

using namespace std;

/****** CELL GRAPH FUNCTIONS *******/

cell_graph_t::cell_graph_t()
{
	this->num_rooms = 0;
}

cell_graph_t::~cell_graph_t() {}
	
void cell_graph_t::remove(cell_t* c)
{
	set<cell_t*>::iterator eit;

	/* check arguments */
	if(c == NULL)
		return;

	/* remove edge refs */
	for(eit = c->edges.begin(); eit != c->edges.end(); eit++)
	{
		/* get edge vertex */
		if((*eit) == NULL)
			continue;

		/* remove edge from other side */
		(*eit)->edges.erase(c);

	}

	/* remove from map */
	this->V.erase(*c);
}

void cell_graph_t::remove_outliers()
{
	set<cell_t>::iterator it;
	set<cell_t*> to_remove;
	set<cell_t*>::iterator oit;
	cell_t* c;

	/* iterate over cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		c = (cell_t*) (&(*it));

		/* check status */
		if(c->is_outlier())
			to_remove.insert(c);
	}

	/* remove outlier cells */
	for(oit = to_remove.begin(); oit != to_remove.end(); oit++)
		this->remove(*oit);
}
	
void cell_graph_t::reset_heights()
{
	set<cell_t>::iterator it;
	cell_t* c;

	/* iterate over cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		c = (cell_t*) (&(*it));
		c->min_z = -ASSUMED_WALL_HEIGHT/2;
		c->max_z = ASSUMED_WALL_HEIGHT/2;
	}
}
	
void cell_graph_t::flatten_room_heights()
{
	vector<vector<double> > room_min_z_vals;
	vector<vector<double> > room_max_z_vals;
	vector<double> room_median_min_z;
	vector<double> room_median_max_z;
	set<cell_t>::iterator it;
	set<int>::iterator rit;
	cell_t* c;
	int i;

	/* initialize height lists for rooms */
	room_min_z_vals.resize(this->num_rooms);
	room_max_z_vals.resize(this->num_rooms);
	room_median_min_z.resize(this->num_rooms);
	room_median_max_z.resize(this->num_rooms);

	/* iterate over cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* iterate over the rooms that contain this cell */
		for(rit = (*it).room_ids.begin(); 
				rit != (*it).room_ids.end(); rit++)
		{
			/* add this cell's height ranges to this room */
			room_min_z_vals[*rit].push_back((*it).min_z);
			room_max_z_vals[*rit].push_back((*it).max_z);
		}
	}

	/* get the median range for each room */
	for(i = 0; i < this->num_rooms; i++)
	{
		if(!room_min_z_vals[i].empty())
		{
			sort(room_min_z_vals[i].begin(), 
					room_min_z_vals[i].end());
			room_median_min_z[i] = room_min_z_vals[i][
					room_min_z_vals[i].size()/2];
		}
		if(!room_max_z_vals[i].empty())
		{
			sort(room_max_z_vals[i].begin(), 
					room_max_z_vals[i].end());
			room_median_max_z[i] = room_max_z_vals[i][
					room_max_z_vals[i].size()/2];
		}
	}

	/* reset these heights for each cell */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* ignore if this cell belongs to no rooms */
		if((*it).room_ids.empty())
			continue;

		/* get variables */
		c = (cell_t*) (&(*it));
		rit = (*it).room_ids.begin();

		/* change cell's height range */
		c->min_z = room_median_min_z[*rit];
		c->max_z = room_median_max_z[*rit];
		
		/* if there are multiple rooms, then take
		 * the intersection of the ranges */
		rit++;
		for( ; rit != (*it).room_ids.end(); rit++)
		{
			if(c->min_z < room_median_min_z[*rit])
				c->min_z = room_median_min_z[*rit];
			if(c->max_z > room_median_max_z[*rit])
				c->max_z = room_median_max_z[*rit];
		}
	}
}
	
/*** PROCESSING ***/

int cell_graph_t::populate(quadtree_t& tree)
{
	vector<quaddata_t*> dats;
	vector<quaddata_t*>::iterator it;
	set<cell_t>::iterator cit;
	point_t p;
	int ret;

	/* clear any existing data */
	V.clear();

	/* get the points from the tree */
	ret = tree.neighbors_in_range(p, -1, dats);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* create a cell for each point */
	for(it = dats.begin(); it != dats.end(); it++)
		this->V.insert(cell_t(*it));

	/* check for invalid heights */
	for(cit = this->V.begin(); cit != this->V.end(); cit++)
	{
		/* check heights validity */
		if(cit->min_z >= cit->max_z)
		{
			/* reset all heights */
			this->reset_heights();
			break;
		}
	}

	/* success */
	return 0;
}
	
int cell_graph_t::simplify_straights(tri_rep_t& trirep, bool simpdoor)
{
	queue<cell_t*> to_check;
	set<cell_t>::iterator it;
	cell_t* c, *c1, *c2;
	set<int> verts_removed;
	set<int>::iterator sit;
	normal_t n1, n2;
	int ret;

	/* first, we want to check all cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
		to_check.push((cell_t*) (&(*it)));

	/* keep straightening as long as we change the graph */
	while(!(to_check.empty()))
	{
		/* get current cell */
		c = to_check.front();
		to_check.pop();

		/* check if this cell represents a 'straight',
		 * that is, it has exactly two edge neighbors */
		if(!(c->is_ordinary()))
			continue;

		/* check that this cell isn't the boundary between
		 * multiple rooms.  If so, we don't want to reduce its
		 * detail. */
		if(c->is_room_boundary())
			continue;

		/* get neighbors */
		c1 = (cell_t*) (*(c->edges.begin()));
		c2 = (cell_t*) (*(c->edges.rbegin()));

		/* verify that neighbors are also not boundary cells */
		if(!simpdoor && (c1->is_room_boundary() 
					|| c2->is_room_boundary()))
			continue;

		/* get the edge neighbors' direction from current node */
		n1.dir(c->pos, c1->pos);
		n2.dir(c->pos, c2->pos);

		/* determine if these direction are opposing */
		if(n1.dot(n2) >= -PARALLEL_THRESHOLD)
			continue;
		
		/* these directions are opposing, so the
		 * current cell is unnecessary for the geometry
		 * of the graph. */
		
		/* attempt to simplify the trirep */
		ret = trirep.collapse_edge(c1->vertex_index, 
						c->vertex_index);
		if(ret)
		{
			/* check if actual error occurred */
			if(ret < 0)
			{
				/* something bad happened when 
				 * attempting to simplify the 
				 * triangulation. */
				return PROPEGATE_ERROR(-1, ret);
			}

			/* try a more aggressive triangulation
			 * restructuring */
			verts_removed.clear();
			ret = trirep.remove_boundary_vertex(
					c->vertex_index, verts_removed);
			if(ret)
			{
				/* topology prevented edge collapse, so
				 * move on to next attempt. */
				continue;
			}
		}

		/* simplify the graph */
		c->replace_with_clique();
		to_check.push(c1);
		to_check.push(c2);
	}

	/* success */
	return 0;
}
	
int cell_graph_t::simplify(tri_rep_t& trirep, double threshold,
						bool simpdoor)
{
	set<cell_t*> removed_cells;
	priority_queue<edge_error_t> pq;
	set<cell_t>::iterator it;
	set<cell_t*>::iterator eit;
	cell_t* a, *b;
	set<int> verts_removed;
	edge_error_t e;
	bool skip_this_edge;
	int i, ret;

	/* check arguments */
	if(threshold < 0)
		return 0; /* no simplification to be done */

	/* iterate through cells, initializing error matrices */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		a = (cell_t*) (&(*it));
		a->init_err_mat();
	}

	/* create a priority queue of all possible edges to simplify */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		a = (cell_t*) (&(*it));

		/* iterate over edges */
		for(eit = a->edges.begin(); eit != a->edges.end(); eit++)
		{
			b = (*eit);

			/* only add edge in one direction */
			if(a > b)
				continue;
			
			/* add edge to queue */
			pq.push(edge_error_t(a, b));
		}
	}

	/* simplify edges */
	while(!(pq.empty()))
	{
		/* get the next edge to simplify */
		e = pq.top();
		pq.pop();
		a = e.a;
		b = e.b;

		/* check if error beyond threshold */
		if(e.err > threshold)
			break;

		/* check if any of the cells of this edge have already
		 * been removed */
		if(removed_cells.count(a) || removed_cells.count(b))
			continue;

		/* check if b is shared by multiple rooms, in which
		 * case, we don't want to remove this edge. */
		if(b->is_room_boundary() && !simpdoor)
			continue;

		/* also check any neighbors of b.  If b is adjacent to
		 * a room boundary, then we don't want to remove it. */
		skip_this_edge = false;
		for(eit = b->edges.begin(); eit != b->edges.end(); eit++)
			if((*eit)->is_room_boundary())
			{
				skip_this_edge = true;
				break;
			}
		if(skip_this_edge && !simpdoor)
			continue;
	
		/* attempt to simplify edge within the triangulation */
		ret = trirep.collapse_edge(a->vertex_index,
						b->vertex_index);
		if(ret)
		{
			/* check if actual error occurred */
			if(ret < 0)
			{
				/* something bad happened when 
				 * attempting to simplify the 
				 * triangulation. */
				return PROPEGATE_ERROR(-1, ret);
			}

			/* try a more aggressive triangulation
			 * restructuring */
			verts_removed.clear();
			ret = trirep.remove_boundary_vertex(
					b->vertex_index, verts_removed);
			if(ret)
			{
				/* topology prevented edge collapse, so
				 * move on to next attempt. */
				continue;
			}

		}

		/* simplify this edge */
		a->transfer_all_edges(b);
		for(i = 0; i < ERROR_MATRIX_SIZE; i++)
			a->err_mat[i] = e.err_mat_sum[i];

		/* the cell b no longer connects to edges */
		removed_cells.insert(b);
	
		/* all of a's new edges should be inserted */
		for(eit = a->edges.begin(); eit != a->edges.end(); eit++)
		{
			b = *eit;

			/* add edge to queue */
			pq.push(edge_error_t(a, b));
		}
	}

	/* success */
	return 0;
}
	
int cell_graph_t::remove_sharps(tri_rep_t& trirep, double threshold)
{
	map<cell_t*, double> to_remove;
	map<cell_t*, double>::iterator mit;
	set<cell_t>::iterator it;
	set<int> verts_removed;
	cell_t* a, *b, *c;
	normal_t ab, ac;
	double angle;
	int ret;

	/* check arguments */
	if(threshold < 0)
		return 0; /* do nothing */

	/* iterate over the cells of this graph, checking for sharps */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* get current cell */
		a = (cell_t*) (&(*it));

		/* check number of neighbors */
		if(!(a->is_ordinary()))
			continue; /* only simplify standard walls */

		/* get neighboring cells */
		b = *(a->edges.begin());
		c = *(a->edges.rbegin());

		/* get the angle bac */
		ab.dir(a->pos, b->pos);
		ac.dir(a->pos, c->pos);
		angle = fabs(ab.angle(ac));

		/* compare to threshold */
		if(threshold > angle)
		{
			/* this is a very sharp angle, so
			 * we want to remove it.  BUT, if we're
			 * already about to remove one of the neighbors,
			 * we don't want to remove both.*/
			if(to_remove.count(b))
			{
				/* check if current sharp is worse
				 * to remove */
				if(angle > to_remove[b])
					continue;

				/* if got here, then we want to remove
				 * a instead of b */
				to_remove.erase(b);
			}
			if(to_remove.count(c))
			{
				/* check if current worse than c */
				if(angle > to_remove[c])
					continue;

				/* if got here, then we want to remove
				 * a instead of c */
				to_remove.erase(c);
			}

			/* if got here, then we should remove a */
			to_remove.insert(pair<cell_t*, double>(a, angle));
		}
	}

	/* remove all the stored elements */
	for(mit = to_remove.begin(); mit != to_remove.end(); mit++)
	{
		/* remove the vertices */
		verts_removed.clear();
		ret = trirep.remove_boundary_vertex(
				mit->first->vertex_index, verts_removed);
		if(ret)
		{
			/* topology prevented edge collapse, so
			 * move on to next attempt. */
			continue;
		}

		/* simplify the graph */
		mit->first->replace_with_clique();
	}
	
	/* success */
	return 0;
}

/* get_root:
 *
 * 	Helper function for union-find.  Gets
 * 	the root of i, and simplies forest.
 */
int get_root(vector<int>& forest, int i)
{
	int r, p;

	/* error check */
	if(i < 0)
		return i;

	/* get parent of i */
	p = forest[i];

	/* check if i is root */
	if(p == i)
		return i;

	/* recurse */
	r = get_root(forest, p);

	/* simplify and return */
	forest[i] = r;
	return r;
}
	
void cell_graph_t::union_find(vector<set<cell_t*> >& unions)
{
	set<cell_t>::iterator it;
	set<cell_t*>::iterator eit;
	vector<cell_t*> index_map;
	vector<int> forest;
	vector<int> roots;
	vector<int>::iterator iit;
	int i, n, r, ri;

	/* index the cells */
	this->index_cells();

	/* store the cells in a reference vector */
	n = this->V.size();
	index_map.resize(n);
	for(it = this->V.begin(); it != this->V.end(); it++)
		index_map[(*it).index] = (cell_t*) (&(*it));

	/* initialize the forest so every cell is a root */
	forest.resize(n);
	for(i = 0; i < n; i++)
		forest[i] = i;

	/* iterate over cells, connect unions via edges */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* Check all other cells that *it is connected to.
		 * The cell with the smallest index is the root. */ 
		r = get_root(forest, (*it).index);
		for(eit = (*it).edges.begin(); eit != (*it).edges.end();
								eit++)
		{
			/* get index of neighboring cell, check its
			 * root, and possibly update r so it is the
			 * smallest of all roots */
			i = (*eit)->index;
			ri = get_root(forest, i);
			if(r < 0 || (ri >= 0 && ri < r))
				r = ri;
		}

		/* verify r is non-negative */
		if(r < 0)
			continue;
	
		/* set the root of all of these cells to be r */
		i = (*it).index;
		ri = get_root(forest, i);
		if(ri >= 0 && r >= 0)
			forest[ri] = r;
	
		for(eit = (*it).edges.begin(); eit != (*it).edges.end();
								eit++)
		{
			i = (*eit)->index;
			ri = get_root(forest, i);
			if(ri >= 0 && r >= 0)
				forest[ri] = r;
		}
	}

	/* fully simplify tree, and count tree */
	roots.clear();
	for(i = 0; i < n; i++)
	{
		ri = get_root(forest, i);
		if(ri == i)
			roots.push_back(i);
	}

	/* initialize union list */
	unions.clear();
	unions.resize(roots.size());

	/* fill union list */
	for(i = 0; i < n; i++)
	{
		ri = get_root(forest, i);
		iit = lower_bound(roots.begin(), roots.end(), ri);
		unions[(int) (iit - roots.begin())].insert(index_map[i]);
	}
}

void cell_graph_t::remove_unions_below(double len)
{
	vector<set<cell_t*> > unions;
	set<cell_t*>::iterator it, eit;
	int i, n;
	double il;

	/* generate unions for this graph */
	this->union_find(unions);
	n = unions.size();

	/* iterate over unions */
	for(i = 0; i < n; i++)
	{
		/* find the sum of the lengths of the edges within
		 * this union */
		il = 0;
		for(it = unions[i].begin(); it != unions[i].end(); it++)
		{
			/* Add up the lengths of the edges of this cell.
			 * All edges are internal to this union, since
			 * the edges are used as connectivity to define
			 * the union. */
			for(eit = (*it)->edges.begin();
					eit != (*it)->edges.end(); eit++)
				il += sqrt((*it)->dist_sq(*eit));
		}

		/* since edges are bidirectional, each edge was counted
		 * twice, so we need to correct for that */
		il /= 2;

		/* compare this length with our threshold */
		if(il < len)
		{
			/* remove all the cells within this union */
			for(it = unions[i].begin(); 
					it != unions[i].end(); it++)
				this->remove(*it);
		}
	}
}

int cell_graph_t::partition_regions(vector<vector<cell_t*> >& regions, 
						tri_rep_t& orienter)
{
	set<cell_t>::iterator vit;
	set<cell_t*> corners;
	set<cell_t*> to_remove;
	set<cell_t*>::iterator cit;
	cell_t* c, *c_start, *c_next;
	normal_t edge_dir;
	double d, as, an;

	/* first, check that all cells are ordinary. */
	for(vit = this->V.begin(); vit != this->V.end(); vit++)
	{
		/* get current cell */
		c = (cell_t*) (&(*vit));

		/* check if corner */
		if(c->is_corner())
			corners.insert(c);
	}

	/* we want to selectively remove corners if one of the regions they
	 * are adjacent to is too small, and their angle is obtuse. */
	to_remove.clear();
	for(cit = corners.begin(); cit != corners.end(); cit++)
	{
		/* get starting corner */
		c_start = (cell_t*) (*cit);

		/* add initial connection to traverse */
		edge_dir.set(0, 1);
		edge_dir.set(1, 0);
	
		/* traversing the graph forward until another corner
		 * is found. This must happen eventually */
		c = c_start;
		d = 0;
		do
		{
			/* searching for the edge that is oriented
			 * correct and is the sharpest turn */
			c_next = c->traverse(edge_dir, orienter);
			if(c_next == NULL)
				break;

			/* keep track of distance */
			d += sqrt(c->dist_sq(c_next));
			c = c_next;
		}
		while(!corners.count(c) || to_remove.count(c));
		if(c_next == NULL)
			continue;

		/* check if this corner should be coalesced */
		if(d < REGION_COALESCE_MIN_WALL_LENGTH)
		{
			/* either the start or the end of this
			 * region should be removed from the 
			 * corners list (which will coalesce it
			 * with the previous or next region, 
			 * respectively.  Choose the one with the
			 * better angle. */
			as = c_start->corner_angle();
			an = c_next->corner_angle();
			if(as > an)
				to_remove.insert(c_start);
			else
				to_remove.insert(c_next);
		}
	}

	/* remove the specified corners from our list */
	for(cit = to_remove.begin(); cit != to_remove.end(); cit++)
		corners.erase(*cit);

	/* initialize list of regions */
	regions.clear();

	/* iterate over finalized corners, forming regions */
	for(cit = corners.begin(); cit != corners.end(); cit++)
	{
		/* create new region */
		regions.push_back(vector<cell_t*>());

		/* get starting corner */
		c = (cell_t*) (*cit);

		/* add initial connection to region */
		regions.back().push_back(c);
		edge_dir.set(0, 1);
		edge_dir.set(1, 0);
		
		/* keep traversing the graph until another corner
		 * is found. This must happen eventually */
		do
		{
			/* searching for the edge that is oriented
			 * correct and is the sharpest turn */
			c = c->traverse(edge_dir, orienter);
			if(c == NULL)
				break;

			/* add newly traversed edge to region */
			regions.back().push_back(c);
		}
		while(!corners.count(c));	
	}
	
	/* every element should have been seen, given that we
	 * verified that the graph contains only ordinary cells. */
	return 0;
}

void cell_graph_t::compute_height_bounds(double& min_z, double& max_z)
{
	set<cell_t>::iterator it;

	/* initialize bounds */
	min_z = 1;
	max_z = -1;

	/* iterate through cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		if(min_z > max_z)
		{
			/* intialize range */
			min_z = it->min_z;
			max_z = it->max_z;
		}
		else
		{
			/* compare to current cell */
			if(min_z > it->min_z)
				min_z = it->min_z;
			if(max_z < it->max_z)
				max_z = it->max_z;
		}
	}
}

void cell_graph_t::find_clique(cell_t* c, set<cell_t*>& clique)
{
	set<cell_t*>::iterator it;
	vector<cell_t*> intersect;
	vector<cell_t*>::iterator vit;
	cell_t* e;
	int conn;

	/* clear input set */
	clique.clear();

	/* check arguments */
	if(c == NULL)
		return;

	/* prepare temp space */
	intersect.resize(c->edges.size());

	/* iterate over edges */
	for(it = c->edges.begin(); it != c->edges.end(); it++) 
	{
		/* get the cell at the other end of this edge */
		e = (*it);
		if(e == NULL)
		{
			PRINT_ERROR("[find_clique]\tFOUND NULL EDGE"); 
			continue;
		}

		/* get intersection between connectivities of these two
		 * cells */
		vit = set_intersection(c->edges.begin(),
					c->edges.end(),
					e->edges.begin(),
					e->edges.end(),
					intersect.begin());
		
		/* get cardinality of intersection */
		conn = vit - intersect.begin();

		/* check if cardinality is sufficient */
		if(conn <= 0)
			continue;

		/* fill 'clique' with this newly found clique */
		clique.insert(c);
		clique.insert(e);
		clique.insert(*(intersect.begin()));
	
		/* verify that there are three unique elements */
		if(clique.size() != 3)
		{
			PRINT_ERROR("[find_clique]\t"
				"simple edges probably afoot");
			clique.clear();
			continue;
		}

		return;
	}
}

void cell_graph_t::index_cells()
{
	set<cell_t>::iterator it;
	int i;

	/* assign all the current cells a unique index */
	i = 0;
	for(it = this->V.begin(); it != this->V.end(); it++)
		((cell_t*) (&(*it)))->index = i++;
}

void cell_graph_t::print_cells(ostream& os)
{
	set<cell_t>::iterator it;

	/* iterate through cells */
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		os << (*it).pos.get(0) << " "
		   << (*it).pos.get(1) << endl;
	}	
}

void cell_graph_t::print_edges(ostream& os)
{
	set<cell_t>::iterator it;
	set<cell_t*>::iterator eit;

	/* iterate through cells */
	this->index_cells();
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* iterate through edges */
		for(eit = (*it).edges.begin(); eit != (*it).edges.end();
								eit++)
		{
			/* print out edge pair (x1 y1 x2 y2) */
			os << (*it).pos.get(0) << " "
			   << (*it).pos.get(1) << " "
			   << (*it).index << " "
			   << (*eit)->pos.get(0) << " "
			   << (*eit)->pos.get(1) << " "
			   << (*eit)->index << endl;
		}
	}
}
	
void cell_graph_t::print_edges_3D(ostream& os)
{	
	set<cell_t>::iterator it;
	set<cell_t*>::iterator eit;
	int num_verts_written;
	int r, g, b, ro, go, bo;

	/* iterate through cells */
	num_verts_written = 0;
	for(it = this->V.begin(); it != this->V.end(); it++)
	{
		/* get colors for this cell */
		(*it).color_by_room(r, g, b);

		/* iterate through edges */
		for(eit = (*it).edges.begin(); eit != (*it).edges.end();
								eit++)
		{
			/* only print edge in one direction */
			if(&(*it) < (*eit))
				continue;

			/* get color for other cell */
			(*eit)->color_by_room(ro, go, bo);

			/* print out edge as two triangles,
			 * currently unoriented. */
			os << "v "
			   << (*it).pos.get(0) << " "
			   << (*it).pos.get(1) << " "
			   << (*it).min_z << " "
			   << r << " " << g << " " << b << endl
			   << "v "
			   << (*eit)->pos.get(0) << " "
			   << (*eit)->pos.get(1) << " "
			   << (*eit)->min_z << " " 
			   << ro << " " << go << " " << bo << endl
			   << "v "
			   << (*eit)->pos.get(0) << " "
			   << (*eit)->pos.get(1) << " "
			   << (*eit)->max_z << " " 
			   << ro << " " << go << " " << bo
			   << endl
			   << "v "
			   << (*it).pos.get(0) << " "
			   << (*it).pos.get(1) << " "
			   << (*it).max_z << " " 
			   << r << " " << g << " " << b << endl
			   << "f "
			   << (num_verts_written+1) << " "
			   << (num_verts_written+2) << " "
			   << (num_verts_written+3) << endl
			   << "f "
			   << (num_verts_written+1) << " "
			   << (num_verts_written+3) << " "
			   << (num_verts_written+4) << endl;
			num_verts_written += 4; 
		}
	}
}
	
/****** CELL FUNCTIONS ********/

cell_t::cell_t()
{
	this->init(NULL);
}

cell_t::cell_t(quaddata_t* dat)
{
	this->init(dat);
}

cell_t::~cell_t()
{
	this->data = NULL;
	this->union_id = -1;
	this->edges.clear();
}
	
void cell_t::init_err_mat()
{
	set<cell_t*>::iterator it;
	double a, b, c;
	int i;
	normal_t n;

	/* clear error matrix */
	for(i = 0; i < ERROR_MATRIX_SIZE; i++)
		this->err_mat[i] = 0;

	/* iterate over the edges of this cell */
	for(it = this->edges.begin(); it != this->edges.end(); it++)
	{
		/* get error vector for this edge */
		
		/* compute tangent of edge */
		n.dir(this->pos, (*it)->pos);

		/* get a,b,c values of error vector, where
		 *
		 * 	[x y 1] * err_vect = distance from line of edge
		 */
		a = n.get(1);
		b = -n.get(0);
		c = -a*this->pos.get(0) - b*this->pos.get(1);

		/* populate matrix with these values */
		this->err_mat[0] += a*a;
		this->err_mat[1] += a*b;
		this->err_mat[2] += a*c;
		this->err_mat[3] += b*b;
		this->err_mat[4] += b*c;
		this->err_mat[5] += c*c;
	}
}

double cell_t::get_simplification_error(double* mat)
{
	double A, B, C, D, E, F, x, y;

	/* error = Ax^2 + 2Bxy + 2Cx + Dy^2 + 2Ey + F 
	 * 
	 * for:
	 *
	 * 	err_mat =
	 * 			A	B	C
	 * 			B	D	E
	 * 			C	E	F
	 */
	A = mat[0];
	B = mat[1];
	C = mat[2];
	D = mat[3];
	E = mat[4];
	F = mat[5];
	x = this->pos.get(0);
	y = this->pos.get(1);

	/* compute error */
	return fabs(A*x*x + 2*B*x*y + 2*C*x + D*y*y + 2*E*y + F);
}

bool cell_t::is_corner()
{
	/* basic check */
	if(!(this->is_ordinary()))
		return false;
	
	/* return the cosine of the corner angle */
	return this->corner_angle();
}

bool cell_t::corner_angle()
{
	set<cell_t*>::iterator ait, bit;
	normal_t an, bn;

	/* get the neighbors of this cell */
	ait = this->edges.begin();
	bit = ait; bit++;

	/* ait and bit are the neighbors of this
	 * cell, so we should test the angle that
	 * is generated by ait to this to bit 
	 *
	 *	a  <---  this  <---  b
	 */
	an.dir(this->pos, (*ait)->pos);
	bn.dir((*bit)->pos, this->pos);
	
	/* make comparison */
	return (an.dot(bn) < PARALLEL_THRESHOLD);
}
	
cell_t* cell_t::traverse(normal_t& edge_dir, tri_rep_t& orienter)
{
	double ang, ang_best;
	normal_t next_edge_dir, next_edge_dir_best;
	set<cell_t*>::iterator eita, eitb;

	/* init */
	ang_best = -DBL_MAX;
	next_edge_dir_best.set(0,0);
	next_edge_dir_best.set(1,0);
	eitb = this->edges.end();
			
	/* get the correct edge on which to traverse */
	for(eita = this->edges.begin(); 
			eita != this->edges.end(); eita++)
	{
		/* check if this potential edge is
		 * oriented correctly */
		if(!orienter.orient_edge(this->vertex_index,
				(*eita)->vertex_index))
			continue;
		next_edge_dir.dir(this->pos, (*eita)->pos);

		/* compare to any other correctly-
		 * oriented edges we may have found */
		ang = edge_dir.angle(next_edge_dir);
		if(ang > ang_best)
		{
			/* eita is better, since it's
			 * a sharper turn, which means
			 * we are less likely to miss
			 * any edges */
			ang_best = ang;
			eitb = eita;
			next_edge_dir_best = next_edge_dir;
		}
	}

	/* check if we found an edge */
	if(eitb == this->edges.end())
	{
		/* no valid edge found */
		return NULL;
	}

	/* return the new-found edge to traverse */
	edge_dir = next_edge_dir_best;
	return (*eitb);
}
	
cell_t* cell_t::traverse_back(normal_t& edge_dir, tri_rep_t& orienter)
{
	double ang, ang_best;
	normal_t next_edge_dir, next_edge_dir_best;
	set<cell_t*>::iterator eita, eitb;

	/* init */
	ang_best = -DBL_MAX;
	next_edge_dir_best.set(0,0);
	next_edge_dir_best.set(1,0);
	eitb = this->edges.end();
			
	/* get the correct edge on which to traverse */
	for(eita = this->edges.begin(); 
			eita != this->edges.end(); eita++)
	{
		/* check if this potential edge is
		 * oriented correctly (in reverse) */
		if(!orienter.orient_edge((*eita)->vertex_index,
					this->vertex_index))
			continue;

		/* get the direction of this potential path
		 * (in reverse) */
		next_edge_dir.dir((*eita)->pos, this->pos);

		/* compare to any other correctly-
		 * oriented edges we may have found */
		ang = next_edge_dir.angle(edge_dir);
		if(ang > ang_best)
		{
			/* eita is better, since it's
			 * a sharper turn, which means
			 * we are less likely to miss
			 * any edges */
			ang_best = ang;
			eitb = eita;
			next_edge_dir_best = next_edge_dir;
		}
	}

	/* check if we found an edge */
	if(eitb == this->edges.end())
	{
		/* no valid edge found */
		return NULL;
	}

	/* return the new-found edge to traverse */
	edge_dir = next_edge_dir_best;
	return (*eitb);
}

void cell_t::add_edge(cell_t* other)
{
	/* check arguments */
	if(this == NULL || other == NULL)
	{
		PRINT_ERROR("ERROR: Tried to add edge between null cells");
		return;
	}

	/* make sure edge isn't simple */
	if(this == other)
		return;

	/* add other to this */
	this->edges.insert(other);

	/* add this to other */
	other->edges.insert(this);
}
	
void cell_t::remove_edge(cell_t* other)
{
	/* check arguments */
	if(this == NULL || other == NULL)
	{
		PRINT_ERROR("ERROR: Tried to remove edge from null cells");
		return;
	}

	/* remove other from this */
	this->edges.erase(other);

	/* remove this from other */
	other->edges.erase(this);
}
	
void cell_t::transfer_all_edges(cell_t* other)
{
	set<cell_t*>::iterator it;

	/* check arguments */
	if(this == NULL || other == NULL)
	{
		PRINT_ERROR("ERROR: tried to transfer edges of null cells");
		return;
	}

	/* iterate over the edges of other */
	for(it = other->edges.begin(); it != other->edges.end(); it++)
	{
		/* add edge to this */
		this->add_edge(*it);

		/* remove reference to other from *it */
		(*it)->edges.erase(other);
	}

	/* clear all of other's edges */
	other->edges.clear();
	
	/* reset union info */
	other->union_id = -1;
}
	
void cell_t::replace_with_clique()
{
	set<cell_t*>::iterator it, it2;
	set<cell_t*> to_remove;

	/* iterate over edges */
	for(it = this->edges.begin(); it != this->edges.end(); )
	{
		/* iterate over remaining edges */
		it2 = it;
		it2++;
		for( ; it2 != this->edges.end(); it2++)
		{
			/* make sure the cells at the end of these
			 * two edges are connected */
			(*it)->add_edge(*it2);
		}

		/* remove this edge */
		it2 = it;
		it++;
		this->remove_edge(*it2);
	}
	
	/* This cell is now isolated in the graph */
	this->union_id = -1;
}
	
void cell_t::color_by_room(int& r, int& g, int& b) const
{
	/* determine color based on number of rooms that
	 * contain this cell */
	switch(this->room_ids.size())
	{
		case 0:
			/* no rooms, color it white */
			r = g = b = 255;
			return;
		case 1:
			/* randomly seed based on room id */
			srand(*(this->room_ids.begin()));
			r = 64 + rand() % 128;
			g = 64 + rand() % 128;
			b = 64 + rand() % 128;
			return;
		default:
			/* multiple rooms, color it black */
			r = g = b = 0;
			return;
	}
}

/***** EDGE ERROR FUNCTIONS ******/

edge_error_t::edge_error_t()
{
	this->a = NULL;
	this->b = NULL;
	this->err = DBL_MAX;
}

edge_error_t::edge_error_t(cell_t* aa, cell_t* bb)
{
	int i;
	double ae, be;

	/* store cells */
	this->a = aa;
	this->b = bb;

	/* combine error matrices */
	for(i = 0; i < ERROR_MATRIX_SIZE; i++)
		this->err_mat_sum[i] = this->a->err_mat[i] 
					+ this->b->err_mat[i];

	/* find position to minimize the simplification error */
	ae = this->a->get_simplification_error(this->err_mat_sum);
	be = this->b->get_simplification_error(this->err_mat_sum);

	/* rearrange so that 'a' has the smaller error */
	if(ae > be)
	{
		this->a = bb;
		this->b = aa;
		this->err = be;
	}
	else
	{
		this->err = ae;
	}
}

edge_error_t::~edge_error_t()
{
	/* no work required */
}
