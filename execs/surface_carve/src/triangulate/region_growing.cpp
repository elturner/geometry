#include "region_growing.h"
#include <list>
#include <set>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include "../structs/normal.h"
#include "../structs/triangulation.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

/*********** REGION GROWING FUNCTIONS ************/

void region_grow_all(vector<planar_region_t>& rl, triangulation_t& tri)
{
	vector<triangle_t*>::iterator it;
	set<triangle_t*>::iterator it2;
	int i, j;
	double x, y, z;

	/* clear list */
	rl.clear();

	/* set all triangles to have negative region */
	for(it = tri.triangles.begin(); it != tri.triangles.end(); it++)
		(*it)->region_id = -1;

	/* now iterate over triangles, and grow planar regions
	 * out of areas that are currently unclaimed */
	for(it = tri.triangles.begin(); it != tri.triangles.end(); it++)
	{
		/* check if current triangle unclaimed */
		if((*it)->region_id >= 0)
			continue;

		/* get next planar region */
		i = rl.size();
		rl.resize(i+1);
		rl[i].grow_from_seed(*it);

		/* mark each triangle in r as claimed */
		for(it2 = rl[i].tris.begin(); it2 != rl[i].tris.end(); 
								it2++)
		{
			/* use triangle index field to indicate region */
			(*it2)->region_id = i;

			/* update the region neighbor count for this
			 * triangle. */
			(*it2)->region_neigh_count = 0;
			for(j = 0; j < NUM_VERTS_PER_TRI; j++)
			{
				/* if the j'th neighbor is also
				 * part of the i'th region, increment
				 * the neighbor count */
				if(rl[i].tris.find((*it2)->t[j]) 
						!= rl[i].tris.end())
					(*it2)->region_neigh_count++;
			}
		}
		
		/* compute mean point of region */
		
		x = y = z = 0;
		for(it2 = rl[i].tris.begin(); it2 != rl[i].tris.end(); 
								it2++)
			for(j = 0; j < NUM_VERTS_PER_TRI; j++)
			{
				if((*it2)->v[j] == NULL)
				{
					PRINT_ERROR("OH NOES!");
					x = x;
					y = y;
				}

				x += (*it2)->v[j]->x;
				y += (*it2)->v[j]->y;
				z += (*it2)->v[j]->z;
			}
		x /= (NUM_VERTS_PER_TRI * rl[i].tris.size());
		y /= (NUM_VERTS_PER_TRI * rl[i].tris.size());
		z /= (NUM_VERTS_PER_TRI * rl[i].tris.size());
		rl[i].avg_pos.x = x;
		rl[i].avg_pos.y = y;
		rl[i].avg_pos.z = z;
	}
}

void region_grow_coalesce_small(vector<planar_region_t>& rl)
{
	vector<planar_region_t>::iterator rit;
	set<triangle_t*>::iterator tit;
	triangle_t* t;
	int neigh_id, i, ni, my_region;
	bool coalesce_region;

	/* iterate over the list of regions */
	for(rit = rl.begin(); rit != rl.end(); rit++)
	{
		/* check the size of the current region */
		if((*rit).tris.size() >= MIN_NUM_TRIS_PER_REGION)
			continue;

		/* This regions is small, so determine if all of
		 * its triangles border the same, larger region */
		neigh_id = -1;
		coalesce_region = true;
		for(tit = (*rit).tris.begin(); tit != (*rit).tris.end();
								tit++)
		{
			/* get region of current triangle */
			my_region = (*tit)->region_id;

			/* iterate over the neighbors of this triangle */
			for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			{
				/* check the region id of the neighbor */
				ni = (*tit)->t[i]->region_id;

				/* we only care if ni is a 
				 * different region */
				if(ni == my_region)
					continue;

				/* check if we've seen at least another
				 * neighboring region */
				if(neigh_id == -1)
					neigh_id = ni;
				else if(neigh_id != ni)
				{
					/* the current region has at least
					 * two neighbors, so we cannot
					 * coalesce it */
					coalesce_region = false;
					break;
				}
			}

			/* check if we should stop */
			if(!coalesce_region)
				break;
		}

		/* check if we should coalesce this region into
		 * the region defined by neigh_id */
		if(!coalesce_region || neigh_id < 0)
			continue;
	
		/* update each triangle in this region */
		for(tit = (*rit).tris.begin(); tit != (*rit).tris.end();
								tit++)
		{
			/* the normal vector of the region should not
			 * be changed, since these triangles are considered
			 * outliers. */
			(*tit)->region_id = neigh_id;
			rl[neigh_id].tris.insert(*tit);

			/* since we know this triangle is in the strict
			 * interior of this region, we can set its
			 * region neighbor count to full */
			(*tit)->region_neigh_count = NUM_EDGES_PER_TRI;

			/* next, we must iterate over this triangle's
			 * neighbors, to update their region counts */
			for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			{
				t = (*tit)->t[i];

				/* check if we need to update the
				 * region neighbor count */
				if(t->region_neigh_count 
						< NUM_EDGES_PER_TRI 
						&& t->region_id == neigh_id)
					t->region_neigh_count++;
			}
		}

		/* clear this region */
		(*rit).tris.clear();
	}
}

void region_grow_coalesce(vector<planar_region_t>& rl, 
					unsigned int min_reg_size)
{
	set<triangle_t*>::iterator it;
	queue<int> small_reg_inds;
	unsigned int i, e, ri, n;
	int j, m;

	/* find all the regions that are too small */
	n = rl.size();
	for(i = 0; i < n; i++)
		if(rl[i].tris.size() < min_reg_size)
			small_reg_inds.push(i);

	/* check for the edge case where every region is too small */
	if(small_reg_inds.size() >= n)
		return; /* stop now, can't be done */

	/* keep check regions as long as there exists one that 
	 * is too small */
	while(!small_reg_inds.empty())
	{
		/* get index of current region */
		i = small_reg_inds.front();
		small_reg_inds.pop();

		/* find the neighboring region with the largest
		 * number of triangles */
		j = m = -1;
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
		{
			/* check if this is a boundary triangle */
			if((*it)->region_neigh_count >= NUM_EDGES_PER_TRI)
				continue; /* not a boundary tri */

			/* check neighbors for size */
			for(e = 0; e < NUM_EDGES_PER_TRI; e++)
			{
				/* ignore edges that are internal
				 * to the region */
				ri = (*it)->t[e]->region_id;
				if(ri == i)
					continue;

				/* get count of this neighbor region */
				if((int) (rl[ri].tris.size()) > m)
				{
					j = ri;
					m = rl[ri].tris.size();
				}
			}
		}

		/* coalesce with largest neighbor */
		if(j < 0 || ((int) i) == j || m <= 0)
			continue;

		/* iterate over the triangles in this region, updating
		 * the region neighbor counts */
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
			for(e = 0; e < NUM_EDGES_PER_TRI; e++)
				if((*it)->t[e]->region_id == j)
				{	
					/* update this tri's neigh count */
					(*it)->region_neigh_count++;

					/* update neigh count of 
					 * neighboring triangles, */
					(*it)->t[e]->region_neigh_count++;
				}

		/* iterate over triangles again */
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
		{
			/* update region id */
			(*it)->region_id = j;
		}

		/* add region i triangles to the list in region j */
		rl[j].tris.insert(rl[i].tris.begin(), rl[i].tris.end());

		/* clear this region */
		rl[i].tris.clear();
	}
}

void region_grow_snap_verts(vector<planar_region_t>& rl, 
						triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	set<int>::iterator sit;
	set<int> vrs;
	vertex_t* v;
	normal_t n, ni, nj, nk;
	int num_regs, i, j;
	double d, hi, hj, ci, cj;

	/* iterate over each vertex in the triangulation */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* get the vertex in question */
		v = (*vit).second;

		/* iterate over all triangles that are incident to this
		 * vertex, determining which regions each triangle belongs
		 * to */
		vrs.clear();
		for(tit = v->mytris.begin(); tit != v->mytris.end(); tit++)
			vrs.insert((*tit)->region_id);

		/* check what subspace this vertex is restricted to */
		num_regs = vrs.size();
		if(num_regs <= 0)
		{
			PRINT_ERROR("[region_grow_snap_verts]\tfound "
					"isolated vertex, ignoring...");
		}
		else if(num_regs == 1)
		{
			/* project this vertex onto the plane of
			 * this region */
			i = *(vrs.begin());

			/* get displacement of vertex from plane */
			n.x = v->x - rl[i].avg_pos.x;
			n.y = v->y - rl[i].avg_pos.y;
			n.z = v->z - rl[i].avg_pos.z;

			/* only interested in 
			 * orthogonal component */
			d = NORMAL_DOT(n, rl[i].avg_norm);
			n.x = rl[i].avg_norm.x * d;
			n.y = rl[i].avg_norm.y * d;
			n.z = rl[i].avg_norm.z * d;

			/* subtract orthogonal component
			 * from point location */
			v->x -= n.x;
			v->y -= n.y;
			v->z -= n.z;
		}
		else if(num_regs >= 2)
		{
			/* project this vertex onto the line defined
			 * by the intersection of these two regions */
			sit = vrs.begin();
			i = *sit;
			sit++;
			j = *sit;

			/* Note: the following math was taken from
			 * en.wikipedia.org/wiki/Plane_(geometry)#Line_of_
			 * 		intersection_between_two_planes
			 */

			/* get normals of these planes */
			ni.x = rl[i].avg_norm.x;
			ni.y = rl[i].avg_norm.y;
			ni.z = rl[i].avg_norm.z;
			nj.x = rl[j].avg_norm.x;
			nj.y = rl[j].avg_norm.y;
			nj.z = rl[j].avg_norm.z;

			/* get cross product of normals */
			nk.x =  ni.y * nj.z - ni.z * nj.y;
			nk.y = -ni.x * nj.z + ni.z * nj.x;
			nk.z =  ni.x * nj.y - ni.y * nj.x;
			d = NORMAL_MAGNITUDE(nk);
			if(d <= 0)
			{
				/* parallel planes, can't do much */
				continue;
			}
			nk.x /= d;
			nk.y /= d;
			nk.z /= d;

			/* get heights of each plane */
			hi = NORMAL_DOT(rl[i].avg_pos, ni);
			hj = NORMAL_DOT(rl[j].avg_pos, nj);

			/* get angle between planes with dot product */
			d = NORMAL_DOT(ni, nj);

			/* find c1 and c2 */
			ci = (hi - hj*d) / (1 - d*d);
			cj = (hj - hi*d) / (1 - d*d);

			/* find displacement of v */
			n.x = v->x - ci*ni.x - cj*nj.x;
			n.y = v->y - ci*ni.y - cj*nj.y;
			n.z = v->z - ci*ni.z - cj*nj.z;

			/* keep only component parallel to nk */
			d = NORMAL_DOT(nk, n);
			v->x = nk.x*d + ci*ni.x + cj*nj.x;
			v->y = nk.y*d + ci*ni.y + cj*nj.y;
			v->z = nk.z*d + ci*ni.z + cj*nj.z;
		}
	}
}

void region_grow_snap(vector<planar_region_t>& rl)
{
	vector<planar_region_t>::iterator rit;
	set<triangle_t*>::iterator tit;
	double x, y, z, d;
	unsigned int m;
	bool skip_tri;
	normal_t n;
	int i;

	/* iterate over regions */
	for(rit = rl.begin(); rit != rl.end(); rit++)
	{
		/* only bother snapping regions that have
		 * sufficient cardinality */
		m = (*rit).tris.size();
		if(m < MIN_SNAP_REGION_SIZE)
			continue;

		/* compute mean point of region */
		x = (*rit).avg_pos.x;
		y = (*rit).avg_pos.y;
		z = (*rit).avg_pos.z;

		/* iterate over all vertices of region (again),
		 * and snap them to the defined plane */
		for(tit = (*rit).tris.begin(); tit != (*rit).tris.end(); 
								tit++)
		{
			/* check that this triangle is interior to
			 * the current region.  If not, then skip it */
			if((*tit)->region_neigh_count < NUM_EDGES_PER_TRI)
			{
				/* If the regions this triangle borders
				 * are no larger than this one, then
				 * snap.  We want points to snap to
				 * large regions. */
				skip_tri = false;
				for(i = 0; i < NUM_EDGES_PER_TRI; i++)
					if(rl[(*tit)->t[i]->region_id
							].tris.size() > m)
					{
						skip_tri = true;
						break;
					}
				if(skip_tri)
					continue;
			}

			/* snap each vertex of this triangle */
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			{
				/* get displacement of vertex from plane */
				n.x = (*tit)->v[i]->x - x;
				n.y = (*tit)->v[i]->y - y;
				n.z = (*tit)->v[i]->z - z;

				/* only interested in 
				 * orthogonal component */
				d = NORMAL_DOT(n, (*rit).avg_norm);
				n.x = (*rit).avg_norm.x * d;
				n.y = (*rit).avg_norm.y * d;
				n.z = (*rit).avg_norm.z * d;

				/* subtract orthogonal component
				 * from point location */
				(*tit)->v[i]->x -= n.x;
				(*tit)->v[i]->y -= n.y;
				(*tit)->v[i]->z -= n.z;
			}
		}
	}
}

void color_by_region(vector<planar_region_t>& rl)
{
	set<edge_t>::iterator eit;
	set<triangle_t*>::iterator it;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	int i, k, n;

	/* iterate over regions */
	n = rl.size();
	for(i = 0; i < n; i++)
	{
		/* create a random color for each union */
		red   = (unsigned char) ((rand() % 156) + 100);
		green = (unsigned char) ((rand() % 156) + 100);
		blue  = (unsigned char) ((rand() % 156) + 100);

		/* iterate over all triangles in i'th union */
		for(it = rl[i].tris.begin(); it != rl[i].tris.end(); it++)
		{
			/* iterate over the vertices of this triangle */
			for(k = 0; k < NUM_VERTS_PER_TRI; k++)
			{
				/* check for bad triangles */
				if((*it)->v[k] == NULL)
					continue;

				/* color vertex based on region */
				(*it)->v[k]->red = red;
				(*it)->v[k]->green = green;
				(*it)->v[k]->blue = blue;
			}
		}

		/* iterate over the boundary edges, if they exist */
		for(eit = rl[i].boundary.begin(); 
					eit != rl[i].boundary.end(); eit++)
		{
			/* color edge vertices black */
			(*eit).start->red = (*eit).end->red = 0;
			(*eit).start->green = (*eit).end->green = 0;
			(*eit).start->blue = (*eit).end->blue = 0;
		}
	}
}

/********* EDGE FUNCTIONS *************/

edge_t::edge_t()
{
	/* set default values for this edge */
	this->start = NULL;
	this->end = NULL;
}

edge_t::edge_t(vertex_t* s, vertex_t* e)
{
	/* set parameters to given values */
	this->start = s;
	this->end = e;
}

void edge_t::init(vertex_t* s, vertex_t* e)
{	
	/* set parameters to given values */
	this->start = s;
	this->end = e;
}

void edge_t::get_reverse(edge_t& e) const
{
	e.start = this->end;
	e.end = this->start;
}

/********* PLANAR REGION FUNCTIONS ************/

void planar_region_t::add_boundary_edges(vertex_t** vs, int n)
{
	int i, j;
	edge_t e, rev;
	set<edge_t>::iterator eit;

	for(i = 0; i < n; i++)
	{
		j = (i+1) % n;
		if(vs[i]->boundary && vs[j]->boundary)
		{
			/* set a new edge */
			e.init(vs[i], vs[j]);

			/* check if the reverse of this edge
			 * is already in this region. */
			e.get_reverse(rev);
			eit = this->boundary.find(rev);
			if(eit != this->boundary.end())
			{
				/* the edge and its reverse
				 * cancel each other out.  They're
				 * no longer boundaries */
				this->boundary.erase(eit);
				continue;
			}

			/* add edge to region */
			this->boundary.insert(e);
		}
	}
}

void planar_region_t::add_boundary_edge(const edge_t& e)
{
	edge_t rev;
	set<edge_t>::iterator eit;

	/* check if the reverse of this edge
	 * is already in this region. */
	e.get_reverse(rev);
	eit = this->boundary.find(rev);
	if(eit != this->boundary.end())
	{
		/* the edge and its reverse
		 * cancel each other out.  They're
		 * no longer boundaries */
		this->boundary.erase(eit);
	}
	else
	{
		/* add edge to region */
		this->boundary.insert(e);
	}
}

void planar_region_t::grow_from_seed(triangle_t* seed)
{
	set<triangle_t*>::iterator it;
	queue<triangle_t*> qu;
	triangle_t* t;
	normal_t n;
	double d;
	int i;

	/* initialize planar region */
	normal_of_tri(this->avg_norm, seed);
	this->tris.clear();

	/* add seed to queue */
	qu.push(seed);

	/* grow as long as we can */
	while(!qu.empty())
	{
		/* get next element */
		t = qu.front();
		qu.pop();

		/* check if we've seen it before */
		if(t == NULL || this->tris.find(t) != this->tris.end()
						|| t->region_id >= 0)
			continue;

		/* check if triangle fits well with region */
		normal_of_tri(n, t);
		d = NORMAL_DOT(n, this->avg_norm);
		if(PARALLEL_THRESHOLD > d || !isfinite(d))
			continue;
	
		/* update region to include this triangle's geometry */
		normal_average(this->avg_norm, 
			this->avg_norm, this->tris.size(), n, 1);
		
		/* add this triangle to region */
		this->tris.insert(t);

		/* add this triangle's neighbors to queue */
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			qu.push(t->t[i]);
	}
}
	
planar_region_t& planar_region_t::operator=(const planar_region_t& prt)
{
	/* blindly copy each field over */
	this->avg_norm.x = prt.avg_norm.x;
	this->avg_norm.y = prt.avg_norm.y;
	this->avg_norm.z = prt.avg_norm.z;
	this->tris = prt.tris;
	this->my_area = prt.my_area;
	return (planar_region_t&) (prt);
}

double planar_region_t::area()
{
	set<triangle_t*>::iterator it;
	double a;

	/* check if area already computed */
	if(this->my_area >= 0)
		return this->my_area;

	/* initialize sum */
	a = 0;

	/* iterate over triangles */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
		a += (*it)->area();

	/* return final result */
	this->my_area = a;
	return a;
}

/******************** ADDITIONAL FUNCTIONS *********************/

void prune_invalid_triangles_from_regions(vector<planar_region_t>& pl,
					triangulation_t& tri)
{
	set<triangle_t*>::iterator it, it2;
	int i, n, m;

	/* first, mark all triangle indices in this triangulation
	 * as -1 */
	n = tri.triangles.size();
	for(i = 0; i < n; i++)
		tri.triangles[i]->index = -1;

	/* next, iterate through regions, and remove any triangle
	 * whose index is not -1 */
	m = pl.size();
	for(i = 0; i < m; i++)
	{
		/* iterate through triangles */
		for(it = pl[i].tris.begin(); it != pl[i].tris.end(); )
		{
			/* check index */
			if((*it)->index != -1)
			{
				/* delete bad triangle */
				it2 = it;
				it++;
				pl[i].tris.erase(it2);
			}
			else
				it++;
		}
	}

	/* next, index the triangles in tri */
	tri.index_triangles();

	/* once again check each region, this time making sure the
	 * triangle indices are non-negative */
	for(i = 0; i < m; i++)
	{
		/* iterate through triangles */
		for(it = pl[i].tris.begin(); it != pl[i].tris.end(); )
		{
			/* check index */
			if((*it)->index < 0)
			{
				/* delete bad triangle */
				it2 = it;
				it++;
				pl[i].tris.erase(it2);
			}
			else
				it++;
		}
	}
}
