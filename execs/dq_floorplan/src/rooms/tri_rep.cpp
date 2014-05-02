#include "tri_rep.h"
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "../structs/triple.h"
#include "../structs/cell_graph.h"
#include "../delaunay/triangulation/vertex.h"
#include "../delaunay/triangulation/geometry.h"
#include "../delaunay/triangulation/triangulation.h"
#include "../util/constants.h"
#include "../util/room_parameters.h"
#include "../util/error_codes.h"

using namespace std;

/************* TRI_INFO_T functions ***************/

tri_info_t::tri_info_t()
{
	this->init();
}

tri_info_t::tri_info_t(const triple_t& t, triangulation_t& tri,
					set<triple_t>& interior)
{
	this->init(t, tri, interior);
}

tri_info_t::~tri_info_t()
{}

void tri_info_t::init()
{	
	/* set properties */
	this->rcc = -1;
	vertex_set(&(this->cc), 0, 0);
	this->is_local_max = false;
}

void tri_info_t::init(const triple_t& t, triangulation_t& tri,
					set<triple_t>& interior)
{
	triple_t n;
	vertex_t* pi, *pj, *pk;
	int a;

	/* compute neighbors of this triangle */
	a = tri_get_apex(&tri, t.j, t.i);
	n.init(t.j, t.i, a); /* triangle on other side of edge (i,j) */
	if(interior.count(n))
		this->neighs.insert(n);

	a = tri_get_apex(&tri, t.k, t.j);
	n.init(t.k, t.j, a); /* triangle on other side of edge (j,k) */
	if(interior.count(n))
		this->neighs.insert(n);

	a = tri_get_apex(&tri, t.i, t.k);
	n.init(t.i, t.k, a); /* triangle on other side of edge (k,i) */
	if(interior.count(n))
		this->neighs.insert(n);	

	/* find triangle in triangulation */
	pi = TRI_VERTEX_POS(&(tri), t.i);
	pj = TRI_VERTEX_POS(&(tri), t.j);
	pk = TRI_VERTEX_POS(&(tri), t.k);

	/* compute circumcircle information */
	this->rcc = geom_circumcenter(pi, pj, pk, &(this->cc));

	/* for now, this is not a local max */
	this->is_local_max = false;
	this->root = t;
}
	
void tri_info_t::init(const triple_t& t, tri_rep_t& trirep)
{
	vector<triple_t> ns;
	vector<triple_t>::iterator nit;
	map<int, set<triple_t> >::iterator vit[NUM_VERTS_PER_TRI];
	vertex_t* p[NUM_VERTS_PER_TRI];
	int num_neighs[NUM_VERTS_PER_TRI];
	int i;

	/* get info for each vertex */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
	{
		/* get vertex info */
		vit[i] = trirep.vert_map.find(t.get(i));
		if(vit[i] == trirep.vert_map.end())
		{
			/* invalid vertex */
			PRINT_ERROR("[tri_info_t::init]\tinvalid vertex");
			LOGI("\tvit[i] = %d\n", t.get(i));
			return;
		}

		/* how many triangles are connected to this vertex? */
		num_neighs[i] = vit[i]->second.size();

		/* get vertex position */
		p[i] = TRI_VERTEX_POS(&(trirep.tri), t.get(i));
	}

	/* find neighbors for this new triangle */
	ns.resize(num_neighs[0] + num_neighs[1] + num_neighs[2]);
	nit = ns.begin();
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		nit = set_intersection(vit[i]->second.begin(), 
			vit[i]->second.end(),
			vit[(i+1)%NUM_VERTS_PER_TRI]->second.begin(),
			vit[(i+1)%NUM_VERTS_PER_TRI]->second.end(),
			nit);
	ns.resize(nit - ns.begin());
	
	/* add neighbors */
	this->neighs.clear();
	this->neighs.insert(ns.begin(), ns.end());
	this->neighs.erase(t);

	/* compute geometric intrinsics */
	this->rcc = geom_circumcenter(p[0], p[1], p[2], &(this->cc));

	/* set room values to be default */
	this->is_local_max = false;
	this->root = t;
}

/******************** TRIREP FUNCTIONS **************************/

tri_rep_t::tri_rep_t()
{
	/* zero out the triangulation */
	memset(&(this->tri), 0, sizeof(triangulation_t));
}

tri_rep_t::~tri_rep_t()
{
	this->tris.clear();
	this->vert_map.clear();
	this->room_heights.clear();
	tri_cleanup(&(this->tri));
}

void tri_rep_t::init(set<triple_t>& interior)
{
	set<triple_t>::iterator it;

	/* clear any data from the trirep structure */
	this->tris.clear();
	this->room_heights.clear();

	/* iterate over the interior triangles of this triangulation */
	for(it = interior.begin(); it != interior.end(); it++)
	{
		/* add triangle */
		this->tris.insert(make_pair<triple_t, tri_info_t>(
			*it, tri_info_t(*it, this->tri, interior)));
	
		/* add its vertices */
		this->vert_map[it->i].insert(*it);
		this->vert_map[it->j].insert(*it);
		this->vert_map[it->k].insert(*it);
	}
}
	
bool tri_rep_t::contains(triple_t& t)
{
	return (this->tris.count(t) > 0);
}
	
pair<map<triple_t, tri_info_t>::iterator, bool> 
			tri_rep_t::add(const triple_t& t)
{
	map<triple_t, tri_info_t>::iterator it;
	set<triple_t>::iterator sit;
	tri_info_t info;
	int i;

	/* check if valid triangle */
	if(!t.unique())
	{
		/* not valid triangle, won't add to struct */
		return make_pair<
			map<triple_t, tri_info_t>::iterator, 
			bool>(this->tris.end(), false);
	}

	/* check if this triangle already exists */
	it = this->tris.find(t);
	if(it != this->tris.end())
	{
		/* triangle already exists in map, do nothing */
		return make_pair<
			map<triple_t, tri_info_t>::iterator, 
			bool>(it, false);
	}

	/* initialize info about this triangle */
	info.init(t, *this);

	/* add it to vertex map */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		this->vert_map[t.get(i)].insert(t);

	/* add it to neighboring triangles */
	for(sit = info.neighs.begin(); sit != info.neighs.end(); sit++)
	{
		/* get neighbor info */
		it = this->tris.find(*sit);
		if(it == this->tris.end())
			return make_pair<
				map<triple_t, tri_info_t>::iterator, 
				bool>(this->tris.end(), false);
	
		/* add to neighbor's neighbors */
		it->second.neighs.insert(t);
	}

	/* add triangle to map */
	return this->tris.insert(make_pair<triple_t, tri_info_t>(t, info));
}
	
int tri_rep_t::fill_polygonal_hole(vector<int>& vs, const triple_t& root)
{
	pair<map<triple_t, tri_info_t>::iterator, bool> ret;
	triple_t t;
	double ang, max_ang;
	int n, i, i_max, j, prev_i, next_i, next_j;
	bool has_collision;

	/* iteratively fill hole by ear clipping */
	while(vs.size() >= NUM_VERTS_PER_TRI)
	{
		/* get size */
		n = vs.size();

		/* compute the angles of each vertex in this polygon, 
		 * looking for the biggest */
		i_max = -1;
		max_ang = -DBL_MAX;
		for(i = 0; i < n; i++)
		{
			/* get adjacent vertices */
			prev_i = (i+n-1) % n;
			next_i = (i+1) % n;

			/* get angle of current vertex */
			ang = this->angle(vs[prev_i], vs[i], vs[next_i]);
			if(fabs(ang) == DBL_MAX)
				return -1;
	
			/* compare to other angles */
			if(ang <= max_ang || ang < 0)
				continue;

			/* check that no other vertex collides with
			 * the triangle that would be formed by
			 * clipping this one */
			has_collision = false;
			for(j = 0; j < n; j++)
			{
				/* ignore edge cases */
				if(i == j || j == prev_i || j == next_i)
					continue;

				/* check if j'th vertex collides with
				 * proposed triangle */
				has_collision = this->in_triangle(vs[j],
						vs[prev_i], vs[i], 
						vs[next_i]);

				/* perform a secondary check of any
				 * line intersections with edges from
				 * the j'th vertex */
				if(!has_collision)
				{
					/* get current edge of polygon */
					next_j = (j+1) % n;
					if(next_j == prev_i)
						continue;

					/* check if the j'th vertex is 
					 * on the wrong side of the 
					 * edge pq */
					has_collision = 
						this->line_intersection(
							vs[j], 
							vs[next_j], 
							vs[prev_i], 
							vs[next_i]);
				}
				
				/* check if any sort of collison 
				 * occurred */
				if(has_collision)
				{
					/* collision occurs, which means
					 * we cannot clip this vertex
					 * yet. */
					break;
				}
			}
			if(has_collision)
				continue;

			/* this vertex is now our best to clip */
			i_max = i;
			max_ang = ang;
		}

		/* check i_max */
		if(i_max < 0)
		{
			PRINT_WARNING("[tri_rep_t::fill_polygonal_hole]"
					"\tear-clipping aborted");
			LOGI("\t\tpoly size = %d\n", (int) vs.size());
			return -2;
		}

		/* check that we will be making a valid triangle */
		if(vs[(i_max+n-1)%n] != vs[(i_max+1)%n])
		{
			/* add a triangle at i_max, which indicates the 
			 * sharpest bend in this polygon.  This isn't a 
			 * delaunay triangulation, but it's simple and 
			 * reasonable */
			t.init(vs[(i_max+n-1)%n], vs[i_max], 
					vs[(i_max+1)%n]);
			ret = this->add(t);
			if(!(ret.second))
				return -3;

			/* set the root of this new triangle */
			ret.first->second.root = root;
		}

		/* remove i_max from the list of vertices */
		vs.erase(vs.begin() + i_max);
	}

	/* success */
	return 0;
}
	
double tri_rep_t::dist(int a, int b)
{
	vertex_t* va, *vb;
	double dx, dy;

	/* get vertices */
	va = TRI_VERTEX_POS(&(this->tri), a);
	vb = TRI_VERTEX_POS(&(this->tri), b);

	/* compute element-wise difference */
	dx = va->pos[0] - vb->pos[0];
	dy = va->pos[1] - vb->pos[1];

	/* compute distance */
	return sqrt(dx*dx + dy*dy);
}

void tri_rep_t::remove(const triple_t& t)
{
	map<triple_t, tri_info_t>::iterator it, nit;
	map<int, set<triple_t> >::iterator vit;
	set<triple_t>::iterator sit;

	/* get this t in this object */
	it = this->tris.find(t);
	if(it == this->tris.end())
		return; /* don't need to do anything */

	/* iterate over neighbors, remove references */
	for(sit = it->second.neighs.begin(); 
				sit != it->second.neighs.end(); sit++)
	{
		/* get iterator to this neighbor */
		nit = this->tris.find(*sit);
		if(nit == this->tris.end())
			continue;

		/* remove reference to t from the neighbor */
		nit->second.neighs.erase(t);
	}

	/* iterate over vertices, remove reference to this triangle */
	vit = this->vert_map.find(t.i);
	if(vit != this->vert_map.end())
	{
		/* remove t from the neighbor set of this vertex */
		vit->second.erase(t);
	}
	vit = this->vert_map.find(t.j);
	if(vit != this->vert_map.end())
	{
		/* remove t from the neighbor set of this vertex */
		vit->second.erase(t);
	}
	vit = this->vert_map.find(t.k);
	if(vit != this->vert_map.end())
	{
		/* remove t from the neighbor set of this vertex */
		vit->second.erase(t);
	}

	/* NOTE:  t may be a root triangle.  If so, removing it
	 * at the wrong time may cause issues.  This is not checked
	 * in this function, so be warned! */

	/* remove t from this object */
	this->tris.erase(it);
}
	
void tri_rep_t::remove(int a)
{
	map<int, set<triple_t> >::iterator it;
	set<triple_t> to_remove;
	set<triple_t>::iterator sit;

	/* get the triangles that adjoin this vertex */
	it = this->vert_map.find(a);
	if(it == this->vert_map.end())
		return; /* vertex not present, do nothing */

	/* delete vertex */
	to_remove.insert(it->second.begin(), it->second.end());
	this->vert_map.erase(it);
	
	/* delete all these triangles */
	for(sit = to_remove.begin(); sit != to_remove.end(); sit++)
		this->remove(*sit);
}
	
int tri_rep_t::compute_boundary_edges(vector<vector<int> >& edge_list,
						set<triple_t>& tris)
{
	map<int, set<edge_t> > edge_map;
	set<edge_t> all_edges;
	set<edge_t>::iterator eit;
	vector<edge_t> to_remove;
	vector<edge_t>::iterator vit;
	set<triple_t>::iterator tit;
	edge_t e;
	int last;

	/* grab all the edges of all triangles */
	for(tit = tris.begin(); tit != tris.end(); tit++)
		tit->get_edges(all_edges);

	/* find opposing edges */
	for(eit = all_edges.begin(); eit != all_edges.end(); eit++)
	{
		/* get flip of this edge */
		e = eit->flip();

		/* check if flip is present */
		if(all_edges.count(e))
		{
			to_remove.push_back(*eit);
			to_remove.push_back(e);
		}
	}
	
	/* cancel opposing edges */
	for(vit = to_remove.begin(); vit != to_remove.end(); vit++)
		all_edges.erase(*vit);
	to_remove.clear();

	/* order them by mapping start vertex to full edge */
	for(eit = all_edges.begin(); eit != all_edges.end(); eit++)
		edge_map[eit->i].insert(*eit);

	/* initialize output */
	edge_list.clear();

	/* build boundaries */
	while(!(all_edges.empty()))
	{
		/* start a new boundary */
		edge_list.push_back(vector<int>(0));

		/* get an edge to follow */
		eit = all_edges.begin();
		edge_list.back().push_back(eit->i);
		edge_list.back().push_back(eit->j);
		last = eit->j;
		edge_map[eit->i].erase(*eit);
		all_edges.erase(eit);

		/* follow edge as long as possible */
		while(!(edge_map[last].empty()))
		{
			/* find an edge that starts with the last
			 * element in the current boundary */
			eit = edge_map[last].begin();
			
			/* check if we've made a loop */
			if(eit->j == edge_list.back().front())
			{
				all_edges.erase(*eit);
				edge_map[last].erase(eit);
				break;
			}

			/* follow this edge */
			edge_list.back().push_back(eit->j);

			/* can no longer use this edge */
			all_edges.erase(*eit);
			edge_map[last].erase(eit);
			
			/* continue searching on latest vertex */
			last = eit->j;
		}
	}

	/* success */
	return 0;
}
	
int tri_rep_t::get_walls(vector<edge_t>& walls)
{
	map<triple_t, tri_info_t>::iterator it;
	set<triple_t> keys;
	vector<vector<int> > edge_list;
	int i, j, n, m, ret;

	/* get all triangles */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
		keys.insert(it->first);

	/* get the boundary edges */
	ret = tri_rep_t::compute_boundary_edges(edge_list, keys);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* initialize output */
	walls.clear();

	/* convert to edges */
	n = edge_list.size();
	for(i = 0; i < n; i++)
	{
		/* iterate over this boundary */
		m = edge_list[i].size();
		for(j = 0; j < m; j++)
		{
			/* make an edge */
			walls.push_back(edge_t(edge_list[i][j], 
					edge_list[i][(j+1)%m]));
		}
	}

	/* success */
	return 0;
}
	
void tri_rep_t::get_rooms(vector<set<triple_t> >& rooms)
{
	map<triple_t, tri_info_t>::iterator it;
	map<triple_t, int> root_map;
	map<triple_t, int>::iterator mit;

	/* initialize the output */
	rooms.clear();

	/* iterate over the triangles in this structure */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* get the current triangle's root's number */
		mit = root_map.find(it->second.root);
		if(mit == root_map.end())
		{
			/* this is a new room, so make an entry in
			 * the map for it */
			mit = root_map.insert(make_pair<triple_t, int>(
				it->second.root, rooms.size())).first;
			rooms.push_back(set<triple_t>());
		}

		/* add this element to the appropriate list in the
		 * output container */
		rooms[mit->second].insert(it->first);
	}
}

bool tri_rep_t::orient_edge(int a, int b)
{
	triple_t t;

	/* call full function */
	return this->orient_edge(a, b, t);
}

bool tri_rep_t::orient_edge(int a, int b, triple_t& t)
{
	map<int, set<triple_t> >::iterator ait, bit;
	vector<triple_t> intersection;
	vector<triple_t>::iterator iit;

	/* find these vertices in this structure */
	ait = this->vert_map.find(a);
	bit = this->vert_map.find(b);
	if(ait == this->vert_map.end() || bit == this->vert_map.end())
		return false;

	/* find the triangles they share */
	intersection.resize(ait->second.size());
	iit = set_intersection(ait->second.begin(), ait->second.end(),
				bit->second.begin(), bit->second.end(),
				intersection.begin());
	intersection.resize(iit - intersection.begin());

	/* If there are no triangles in common, then these vertices don't
	 * share an edge.  If there are multiple triangles in common, then
	 * the edge is not a boundary edge */
	if(intersection.size() != 1)
		return false;
	t = *(intersection.begin()); /* this is the triangle to check */

	/* check order */
	return (t.i == a && t.j == b) 
		|| (t.j == a && t.k == b) 
		|| (t.k == a && t.i == b);
}
	
bool tri_rep_t::room_edge(int a, int b, triple_t& t)
{
	map<triple_t, tri_info_t>::iterator ptit, qtit;
	map<int, set<triple_t> >::iterator ait, bit;
	vector<triple_t> intersection;
	vector<triple_t>::iterator iit, pit;
	vector<triple_t>::reverse_iterator qit;

	/* find these vertices in this structure */
	ait = this->vert_map.find(a);
	bit = this->vert_map.find(b);
	if(ait == this->vert_map.end() || bit == this->vert_map.end())
		return false;

	/* find the triangles they share */
	intersection.resize(ait->second.size());
	iit = set_intersection(ait->second.begin(), ait->second.end(),
				bit->second.begin(), bit->second.end(),
				intersection.begin());
	intersection.resize(iit - intersection.begin());

	/* If there are no triangles in common, then these vertices don't
	 * share an edge.  If there are multiple triangles in common, then
	 * the edge is not a boundary edge */
	if(intersection.size() > 2 || intersection.empty())
		return false;
	if(intersection.size() == 1)
	{
		/* edge only has one trinagle, so it must
		 * be a boundary edge */

		/* this is the triangle to check */
		t = *(intersection.begin());

		/* check order */
		return (t.i == a && t.j == b) 
			|| (t.j == a && t.k == b) 
			|| (t.k == a && t.i == b);
	}

	/* There are two triangles that contain this edge.
	 * Check that they have different rooms */
	pit = intersection.begin();
	ptit = this->tris.find(*pit);
	if(ptit == this->tris.end())
		return false;

	qit = intersection.rbegin();
	qtit = this->tris.find(*qit);
	if(qtit == this->tris.end())
		return false;

	/* check roots */
	if(ptit->second.root == qtit->second.root)
		return false;

	/* check the two triangles that contain this edge */
	if( (pit->i == a && pit->j == b) || (pit->j == a && pit->k == b) 
			|| (pit->k == a && pit->i == b) )
	{
		/* pit is the triangle we want! */
		t = *pit;
		return true;
	}

	/* verify that the other triangle contains this edge */
	if( (qit->i == a && qit->j == b) || (qit->j == a && qit->k == b) 
			|| (qit->k == a && qit->i == b) )
	{
		/* qit is the triangle we want! */
		t = *qit;
		return true;
	}

	/* neither triangle contains edge.  Uh oh. */
	return false;
}
	
double tri_rep_t::angle(int a, int b, int c)
{
	vertex_t* av, *bv, *cv;
	point_t p, q;
	normal_t ab, bc;

	/* get position of vertices */
	av = TRI_VERTEX_POS(&(this->tri), a);
	bv = TRI_VERTEX_POS(&(this->tri), b);
	cv = TRI_VERTEX_POS(&(this->tri), c);
	
	/* get edge directions */
	p.set(0, av->pos[0]);
	p.set(1, av->pos[1]);
	q.set(0, bv->pos[0]);
	q.set(1, bv->pos[1]);
	ab.dir(p, q);
	p.set(0, cv->pos[0]);
	p.set(1, cv->pos[1]);
	bc.dir(q, p);

	/* compute the angle */
	return ab.angle(bc);
}
	
bool tri_rep_t::line_intersection(int a1, int a2, int b1, int b2)
{
	vertex_t* a1v, *a2v, *b1v, *b2v;

	/* check degenerate case */
	if(a1 == a2 || b1 == b2)
		return false;

	/* doesn't count if they meet at an endpoint */
	if(a1 == b1 || a1 == b2 || a2 == b1 || a2 == b2)
		return false;

	/* get vertex positions */
	a1v = TRI_VERTEX_POS(&(this->tri), a1);
	a2v = TRI_VERTEX_POS(&(this->tri), a2);
	b1v = TRI_VERTEX_POS(&(this->tri), b1);
	b2v = TRI_VERTEX_POS(&(this->tri), b2);

	/* compute line intersection */
	return (geom_line_intersect(a1v, a2v, b1v, b2v, NULL) > 0);
}
	
bool tri_rep_t::star_intersection(int v, int a1, int a2, 
					set<int>& to_ignore)
{
	map<int, set<triple_t> >::iterator vit;
	set<triple_t>::iterator sit;
	vertex_t* a1v, *a2v, *vv, *wv;
	int i, w, r;

	/* get neighboring triangles */
	vit = this->vert_map.find(v);
	if(vit == this->vert_map.end())
		return false;
	
	/* get vertex positions */
	a1v = TRI_VERTEX_POS(&(this->tri), a1);
	a2v = TRI_VERTEX_POS(&(this->tri), a2);
	vv = TRI_VERTEX_POS(&(this->tri), v);
	
	/* iterate over triangles of v */
	for(sit = vit->second.begin(); sit != vit->second.end(); sit++) 
	{
		/* iterate over vertices of this triangle */
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		{
			/* get vertex index */
			w = sit->get(i);
			if(v == w || to_ignore.count(w))
				continue;

			/* get intersection */
			wv = TRI_VERTEX_POS(&(this->tri), w);
			r = geom_line_intersect(a1v, a2v, vv, wv, NULL);
			if(r > 0)
				return true;
		}
	}
	
	/* no intersection found */
	return false;
}
	
bool tri_rep_t::in_triangle(int v, int a, int b, int c)
{
	vertex_t* vv, *va, *vb, *vc;

	/* check edge cases */
	if(v == a || v == b || v == c)
		return false;

	/* get vertex positions */
	vv = TRI_VERTEX_POS(&(this->tri), v);
	va = TRI_VERTEX_POS(&(this->tri), a);
	vb = TRI_VERTEX_POS(&(this->tri), b);
	vc = TRI_VERTEX_POS(&(this->tri), c);

	/* check validity of triangle */
	if(geom_orient2D(va, vb, vc) <= 0)
		return false;

	/* check inside/outside test */
	return (geom_intriangle(va, vb, vc, vv) != 0); 
}
	
int tri_rep_t::collapse_edge(int a, int b)
{
	map<triple_t, tri_info_t>::iterator tit, tait, tbit, tnit, tnew_it;
	map<int, set<triple_t> >::iterator ait, bit, vit;
	set<triple_t>::iterator nit, sit;
	vector<triple_t> intersection;
	vector<triple_t>::iterator iit;
	triple_t t, ta, tb, tnew;
	bool ta_found, tb_found;
	double d;

	/* find these vertices in this structure */
	ait = this->vert_map.find(a);
	bit = this->vert_map.find(b);
	if(ait == this->vert_map.end() || bit == this->vert_map.end())
		return 1; /* unable to collapse non-existant verts */

	/* find the triangles they share */
	intersection.resize(ait->second.size());
	iit = set_intersection(ait->second.begin(), ait->second.end(),
				bit->second.begin(), bit->second.end(),
				intersection.begin());
	intersection.resize(iit - intersection.begin());

	/* If there are no triangles in common, then these vertices don't
	 * share an edge.  If there are multiple triangles in common, then
	 * the edge is not a boundary edge.  Either way, we can't collapse
	 * it. */
	if(intersection.size() != 1)
		return 2;
	t = *(intersection.begin()); /* this is the triangle to remove */
	tit = this->tris.find(t);
	if(tit == this->tris.end())
		return 3; /* this structure is inconsistant! */

	/* verify that t has at most two triangle neighbors */
	if(tit->second.neighs.size() > 2)
		return 4; /* t should have at least one boundary edge */

	/* t may have neighbors, whose neighbor pointers will need to 
	 * be reset once t is removed */
	ta_found = tb_found = false;
	for(nit = tit->second.neighs.begin();
			nit != tit->second.neighs.end(); nit++)
	{
		/* check edge case */
		if(*nit == t)
			continue;

		/* does the current neighbor contain a */
		if((*nit).i == a || (*nit).j == a || (*nit).k == a)
		{
			/* error check on topology */
			if(ta_found)
				return 5; /* shouldn't happen */

			/* record ta */
			ta_found = true;
			ta = *nit;
		}
		
		/* does the current neighbor contain b */
		if((*nit).i == b || (*nit).j == b || (*nit).k == b)
		{
			/* error check on topology */
			if(tb_found)
				return 6; /* shouldn't happen */

			/* record tb */
			tb_found = true;
			tb = *nit;
		}
	}

	/* if we collapse this edge, the vertex b will go away, and
	 * the triangles connected to be will be redefined.  The following
	 * does an initial check to make sure that there will be
	 * no collisions with existing triangles when this process occurs.
	 */
	for(sit = bit->second.begin(); sit != bit->second.end(); sit++)
	{
		/* ignore t, which is connected to b, but will soon
		 * be deleted */
		if(*sit == t)
			continue;

		/* first, what will we change this triangle into? */
		tnew.init(((*sit).i == b) ? a : (*sit).i,
			((*sit).j == b) ? a : (*sit).j,
			((*sit).k == b) ? a : (*sit).k);

		/* does this triangle already exist? */
		if(this->tris.count(tnew))
			return 7;

		/* check the orientation of this triangle.  If it is
		 * negative, that means that collapsing this edge will
		 * "fold" the triangulation, which is bad. */
		d = geom_orient2D(TRI_VERTEX_POS(&(this->tri), tnew.i),
				TRI_VERTEX_POS(&(this->tri), tnew.j),
				TRI_VERTEX_POS(&(this->tri), tnew.k));
		if(d <= 0)
			return 8;
	}

	/* We only need to worry about re-referencing neighbors of t if
	 * both ta and tb exist */
	if(ta_found && tb_found)
	{
		/* check degenerate case (which shouldn't exist) */
		if(ta == tb)
			return 9;

		/* get info about these triangles */
		tait = this->tris.find(ta);
		tbit = this->tris.find(tb);
		if(tait == this->tris.end() || tbit == this->tris.end())
			return 10;
	
		/* check the case that these triangles are already
		 * neighbors.  If so, this will cause issues when
		 * attempting to collapse the given edge */
		if(tait->second.neighs.count(tb) 
				|| tbit->second.neighs.count(ta))
			return 11;

		/* add these two triangles to each other's neighbors */
		tait->second.neighs.insert(tb);
		tbit->second.neighs.insert(ta);
	}

	/* remove t from this triangulation */
	this->remove(t);

	/* since we are also removing the vertex b from this triangulation,
	 * we should change all references from b to a in all fields */
	for(sit = bit->second.begin(); sit != bit->second.end(); )
	{
		/* the triangle *nit should contain b.  We want to change
		 * it so that it contains a instead of b.  This requires
		 * changing:
		 *
		 * 	- triangle-triangle neighbor references
		 * 	- vertex-triangle neighbor references
		 * 	- entry into this->tris
		 */

		/* first, what are we changing this triangle into? */
		t = *sit;
		tnew.init(((*sit).i == b) ? a : (*sit).i,
			((*sit).j == b) ? a : (*sit).j,
			((*sit).k == b) ? a : (*sit).k);

		/* update the iterator, so that we can modify the list
		 * without worrying about corrupting it. */
		sit++;

		/* does this triangle already exist? */
		if(this->tris.count(tnew))
		{
			/* such a triangle should never exist, so
			 * this causes issues.
			 *
			 * But since we've already modified this structure,
			 * we now have an error on our hands. */
			PRINT_ERROR("[tri_rep_t.collapse_edge]\t"
				"BAD DAY BAD DAY BAD DAY!");
			return -12;
		}

		/* add tnew to the structure */
		tnew_it = this->tris.insert(make_pair<triple_t, 
				tri_info_t>(tnew, tri_info_t())).first;

		/* get info about old triangle */
		tit = this->tris.find(t);
		if(tit == this->tris.end())
		{
			/* this shouldn't happen.  Getting here means
			 * that b references non-existant triangles
			 * in this mesh even though it is a valid vertex */
			PRINT_ERROR("[tri_rep_t.collapse_edge]\t"
				"inconsistant triangles found!");
			return -13;
		}

		/* copy over relevent information */
		tnew_it->second.root = tit->second.root;

		/* remove triangle t from all its neighboring triangles,
		 * and add triangle tn instead */
		for(nit = tit->second.neighs.begin();
				nit != tit->second.neighs.end(); nit++)
		{
			/* look up this neighbor in this structure */
			tnit = this->tris.find(*nit);
			if(tnit == this->tris.end())
			{
				/* invalid neighbors in the struct! */
				PRINT_ERROR("[tri_rep_t.collapse_edge]\t"
					"bad neighbors found!");
				return -14;
			}

			/* remove t from this neighbor's info */
			tnit->second.neighs.erase(t);

			/* add tn to this neighbor's info */
			tnit->second.neighs.insert(tnew);

			/* add this neighbor to tnew */
			tnew_it->second.neighs.insert(*nit);
		}

		/* remove triangle t from all its neighboring verts */
		vit = this->vert_map.find(t.i);
		if(vit == this->vert_map.end())
			return -15;
		vit->second.erase(t);
		vit = this->vert_map.find(t.j);
		if(vit == this->vert_map.end())
			return -16;
		vit->second.erase(t);
		vit = this->vert_map.find(t.k);
		if(vit == this->vert_map.end())
			return -17;
		vit->second.erase(t);

		/* add tnew to all its neighboring verts */
		vit = this->vert_map.find(tnew.i);
		if(vit == this->vert_map.end())
			return -18;
		vit->second.insert(tnew);
		vit = this->vert_map.find(tnew.j);
		if(vit == this->vert_map.end())
			return -19;
		vit->second.insert(tnew);
		vit = this->vert_map.find(tnew.k);
		if(vit == this->vert_map.end())
			return -20;
		vit->second.insert(tnew);

		/* remove triangle t from this structure */
		this->tris.erase(t);
	}

	/* b is no longer in this structure */
	this->vert_map.erase(b);
	
	/* success */
	return 0;
}
	
int tri_rep_t::remove_boundary_vertex(int b, set<int>& verts_removed)
{
	map<triple_t, tri_info_t>::iterator it;
	map<int, set<triple_t> >::iterator vmit;
	set<triple_t> tris_to_remove;
	set<triple_t> roots_of_removed;
	set<triple_t>::iterator sit;
	map<int, int> neigh_counter;
	map<int, int>::iterator cit;
	set<int> verts_to_remove;
	set<int> to_ignore;
	set<int>::iterator iit;
	queue<int> verts_to_check;
	vector<vector<int> > boundary_edges;
	vector<int>::iterator viit;
	int ret, i, a, c, n;
	bool forward, backward;
	triple_t t;

	/* first, get the other edge vertices that b is connected to:
	 *
	 * 	    (outside)
	 *	a <----- b <----- c
	 *	    (inside)
	 */
	vmit = this->vert_map.find(b);
	if(vmit == this->vert_map.end())
		return -1;

	/* search over neighboring triangles, looking for boundary edges */
	for(sit = vmit->second.begin(); sit != vmit->second.end(); sit++)
	{
		/* count each vertex of this neighboring triangle */
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			neigh_counter[sit->get(i)]++;
	}

	/* border vertices will be those who only have a count of 1,
	 * not including 'b' */
	a = c = -1;
	for(cit = neigh_counter.begin(); cit != neigh_counter.end(); cit++)
	{
		/* ignore b */
		if(cit->first == b)
			continue;

		/* check if this is a boundary vertex */
		if(cit->second != 1)
		{
			/* this is an interior vertex connected to b */
			verts_to_check.push(cit->first);
			continue;
		}

		/* check which boundary vertex this is.  We want to
		 * define edges so that b->a and c->b are inside
		 * oriented. */
		forward = (orient_edge(b, cit->first) > 0);
		backward = (orient_edge(cit->first, b) > 0);
		if(forward && !backward)
			a = cit->first;
		else if(!forward && backward)
			c = cit->first;
	}

	/* check that we have two neighboring boundary vertices */
	if(a < 0 || c < 0)
	{
		/* this may be because there are no neighbors 
		 * of b at all, in which case we don't need to simplify it,
		 * since it was already removed from the graph. */
		if(neigh_counter.empty())
			return 0;

		/* there were neighbors of b, but there were insufficient
		 * boundary neighbors.  If there are zero boundary
		 * neighbors, then b is interior and doesn't need to
		 * be simplified */
		if(a < 0 && c < 0)
			return 0;

		/* insufficient boundary neighbors */
		return -2;
	}

	/* when we are done, we want the edge c->a to exist.  We may
	 * need to remove some interior vertices that are on the wrong
	 * side of this edge */
	verts_to_check.push(b);
	to_ignore.clear();
	to_ignore.insert(a);
	to_ignore.insert(b);
	to_ignore.insert(c);
	while(!(verts_to_check.empty()))
	{
		/* get next vertex to check */
		n = verts_to_check.front();
		verts_to_check.pop();

		/* have we seen this one before? */
		if(verts_to_remove.count(n))
			continue;

		/* check against what we want to keep */
		if(n == c || n == a)
			continue;

		/* is this vertex has any edges that intersect
		 * the edge c->a */
		if(n != b 
			&& !(this->star_intersection(n,a,c,to_ignore))
			&& !(this->in_triangle(n, c, b, a)))
			continue; /* still interior, don't change */

		/* if so, then we must remove vertex and check its
		 * neighbors. */
		verts_to_remove.insert(n);
		vmit = this->vert_map.find(n);
		if(vmit == this->vert_map.end())
			return -5;

		/* by removing this vertex, we will need to remove
		 * all adjoining triangles */
		tris_to_remove.insert(vmit->second.begin(), 
					vmit->second.end());

		/* add neighbors to checklist */
		for(sit = vmit->second.begin(); 
					sit != vmit->second.end(); sit++)
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				verts_to_check.push(sit->get(i));
	}

	/* check the rooms of the triangles we are about to remove */
	roots_of_removed.clear();
	for(sit = tris_to_remove.begin(); sit != tris_to_remove.end(); 
								sit++)
	{
		/* get info about this triangle */
		it = this->tris.find(*sit);
		if(it == this->tris.end())
			return -6;

		/* add root */
		roots_of_removed.insert(it->second.root);
	}

	/* check that only one room is to be removed */
	if(roots_of_removed.size() != 1)
		return PROPEGATE_ERROR(-7, -1*roots_of_removed.size());

	/* now that we know what vertices we should remove, we
	 * can remove any triangles associated with these vertices.
	 * These triangles, along with the edge c->a, will define 
	 * the polygonal hole left in the mesh.  First we must determine
	 * the edges of this polygon, in counter-clockwise order. */
	ret = compute_boundary_edges(boundary_edges, tris_to_remove);
	if(ret)
		return PROPEGATE_ERROR(-8, ret);

	/* there should be only one border for this polygon */
	if(boundary_edges.size() != 1)
		return PROPEGATE_ERROR(-9, -1*boundary_edges.size());
	
	/* check for vertices that didn't make the cut */
	n = boundary_edges[0].size();
	for(i = 0; i < n; i++)
		verts_to_remove.erase(boundary_edges[0][i]);
	verts_removed.insert(verts_to_remove.begin(), 
				verts_to_remove.end());

	/* remove the excess vertices and corresponding triangles */
	for(sit = tris_to_remove.begin(); 
				sit != tris_to_remove.end(); sit++)
		this->remove(*sit);

	/* modify the remaining polygonal hole so that b is not present
	 * when we repopulate it with triangles */
	viit = std::remove(boundary_edges[0].begin(), 
				boundary_edges[0].end(), b);
	boundary_edges[0].resize(viit - boundary_edges[0].begin());
	
	/* add triangles to fill the polygonal hole */
	ret = this->fill_polygonal_hole(boundary_edges[0], 
				*(roots_of_removed.begin()));
	if(ret)
		return PROPEGATE_ERROR(-10, ret);

	/* success */
	return 0;
}
	
int tri_rep_t::remove_interroom_columns(double thresh)
{
	map<triple_t, tri_info_t>::iterator tit, tit2;
	map<int, set<triple_t> >::iterator vit;
	set<triple_t> all_tris;
	set<triple_t>::iterator sit;
	vector<vector<int> > boundary_edges;
	vector<vector<int> >::iterator bit;
	map<triple_t, int> room_counts;
	map<triple_t, int>::iterator rit;
	triple_t best_room;
	double len;
	int ret, i, n, brc;

	/* consolidate all triangles in this representation */
	for(tit = this->tris.begin(); tit != this->tris.end(); tit++)
		all_tris.insert(tit->first);

	/* find all walls of this representation, grouped into
	 * connected components. */
	ret = tri_rep_t::compute_boundary_edges(boundary_edges, all_tris);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* iterate over connected components, looking for columns
	 * that are below the specified threshold */
	for(bit = boundary_edges.begin(); bit!=boundary_edges.end(); bit++)
	{
		/* determine the perimeter length of this boundary edge */
		len = 0;
		n = bit->size();
		room_counts.clear();
		for(i = 0; i < n; i++)
		{
			/* get vertex information */
			vit = this->vert_map.find((*bit)[i]);
			if(vit == this->vert_map.end())
				return -2;

			/* add rooms to our counts */
			for(sit = vit->second.begin(); 
					sit != vit->second.end(); sit++)
			{
				/* get triangle info */
				tit2 = this->tris.find(*sit);
				if(tit2 == this->tris.end())
					return -3;

				/* update room count */
				room_counts[tit2->second.root]++;
			}

			/* include this length */
			len += this->dist((*bit)[i], (*bit)[(i+1)%n]);
		}

		/* count number of rooms */
		if(room_counts.size() < 2)
			continue;

		/* check perimeter length against threshold */
		if(len >= thresh)
			continue;

		/* find the most represented room */
		rit = room_counts.begin();
		best_room = rit->first;
		brc = rit->second;
		rit++;
		for( ; rit != room_counts.end(); rit++)
			if(brc < rit->second)
			{
				/* update best room */
				best_room = rit->first;
				brc = rit->second;
			}

		/* reverse the order of the vertices to signify the
		 * interior of the polygon, whereas it currently
		 * signifies the outside of it */
		reverse(bit->begin(), bit->end());

		/* fill this polygon */
		ret = fill_polygonal_hole(*bit, best_room);
		if(ret)
		{
			/* if bug occurred, then continue processing */
		}
	}

	/* success */
	return 0;
}

void tri_rep_t::find_local_max()
{
	set<triple_t> nonextrema;
	set<triple_t> checked;
	queue<triple_t> circum_neighs;
	map<triple_t, tri_info_t>::iterator it, oit;
	set<triple_t>::iterator nit;
	triple_t n;
	double d;
	bool any_neighbors_larger;

	/* initially, all triangles are labeled with local_max = false.
	 * so we just need to iterate over the ones that are possibly
	 * local max. */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* initially assume that this triangle is not a 
		 * local max */
		(*it).second.is_local_max = false;

		/* check to make sure we haven't already ruled this
		 * triangle out as an extrema */
		if(nonextrema.count((*it).first))
			continue;

		/* verify that this triangle meets threshold criteria
		 * for being a room seed. */
		if((*it).second.rcc < MIN_LOCAL_MAX_CIRCUMRADIUS)
			continue;

		/* search the local triangulation to this triangle,
		 * checking if any local triangles have a larger
		 * circumradius. */
		
		/* make sure temporary structures are empty */
		checked.clear();
		while(!circum_neighs.empty())
			circum_neighs.pop();
		
		/* add neighbors to queue */
		for(nit = (*it).second.neighs.begin();
				nit != (*it).second.neighs.end(); nit++)
			circum_neighs.push(*nit);

		/* do a breadth-first search of neighbors */
		any_neighbors_larger = false;
		while(!circum_neighs.empty())
		{
			/* get the next circum neighbor */
			n = circum_neighs.front();
			circum_neighs.pop();
			
			/* make sure we haven't already checked this
			 * neighbor */
			if(checked.count(n) || n == (*it).first)
				continue;
			checked.insert(n);

			/* get info for this neighbor */
			oit = this->tris.find(n);
			if(oit == this->tris.end())
				continue; /* invalid neighbor */

			/* verify that the circumcircles of n and *it
			 * actually intersect */
			d = sqrt(geom_dist_sq(&((*it).second.cc), 
						&((*oit).second.cc)));
			if((*it).second.rcc + (*oit).second.rcc < d)
			{
				/* too far away */
				continue;
			}
		
			/* check if this neighbor's circumcircle is
			 * larger than *it's */
			if((*it).second.rcc < (*oit).second.rcc)
			{
				/* current triangle is not a maximum!
				 * stop searching! */
				any_neighbors_larger = true;
				break;
			}

			/* we know that n is not a local max, since
			 * its a neighbor ot *it and its smaller */
			nonextrema.insert(n);

			/* continue searching neighbors */
			for(nit = (*oit).second.neighs.begin();
				nit != (*oit).second.neighs.end(); nit++)
				circum_neighs.push(*nit);
		}

		/* is *it the biggest? */
		if(!any_neighbors_larger)
			(*it).second.is_local_max = true;
	}
 }
	
void tri_rep_t::flood_rooms()
{
	priority_queue<tri_edge_t> pq;
	map<triple_t, tri_info_t>::iterator it, sit, oit;
	tri_edge_t e;

	/* initialize the flooding with the edges of
	 * each local max triangle. */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* check if current triangle is local max */
		if(!((*it).second.is_local_max))
			continue;

		/* add edges to priority queue */
		pq.push(tri_edge_t((*it).first.i, (*it).first.j,
						this->tri));
		pq.push(tri_edge_t((*it).first.j, (*it).first.k,
						this->tri));
		pq.push(tri_edge_t((*it).first.k, (*it).first.i,
						this->tri));
	}

	/* flood until no more edges.  We want to flood the larger
	 * edges first, so that the boundaries between rooms will be
	 * the smallest gaps */
	while(!(pq.empty()))
	{
		/* get next edge */
		e = pq.top();
		pq.pop();

		/* verify that the end triangle is:
		 *
		 * 	- in this triangulation
		 * 	- still unclaimed
		 */
		oit = this->tris.find(e.end);
		if(oit == this->tris.end())
			continue; /* not interior edge */
		if((*oit).second.root != (*oit).first 
				|| (*oit).second.is_local_max)
			continue; /* already claimed by another room */
	
		/* also double-check the validity of the starting
		 * triangle of this edge */
		sit = this->tris.find(e.start);
		if(sit == this->tris.end())
			continue; /* not valid triangle */
		if((*sit).second.root == (*sit).first
				&& !((*sit).second.is_local_max))
			continue; /* start triangle unclaimed */

		/* expand sit's room into oit */
		(*oit).second.root = (*sit).second.root;

		/* now we want to add the edges of oit to the queue */
		pq.push(tri_edge_t((*oit).first.i, (*oit).first.j, 
							this->tri));
		pq.push(tri_edge_t((*oit).first.j, (*oit).first.k,
							this->tri));
		pq.push(tri_edge_t((*oit).first.k, (*oit).first.i,
							this->tri));
	}
}
	
void tri_rep_t::reset_roots()
{
	map<triple_t, tri_info_t>::iterator it;

	/* iterate over the triangles */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
		/* reset the root of the current triangle */
		(*it).second.root = (*it).first;
}


/* helper function:
 *
 * Will add the perimeter edge specified to the specified room,
 * checking that this is a valid boundary edge.
 */
void tri_rep_t::add_edge_to_room(tri_edge_t& e, room_info_t& r)
{
	map<triple_t, tri_info_t>::iterator sit, eit;
	map<triple_t, double>::iterator bit;

	/* first, check that the start of this edge
	 * is a triangle has the same root as the room */
	sit = this->tris.find(e.start);
	if(sit == this->tris.end())
		return;
	if((*sit).second.root != r.root)
		return;

	/* next, check that the ending triangle is in a
	 * different room */
	eit = this->tris.find(e.end);
	if(eit == this->tris.end())
		return;
	if((*eit).second.root == r.root)
		return;

	/* add the length of the edge to the perimeter boundary
	 * between these two rooms */
	bit = r.border_lengths.find((*eit).second.root);
	if(bit == r.border_lengths.end())
	{
		/* need to add a new entry for this neighboring room */
		bit = r.border_lengths.insert(make_pair<triple_t,
				double>((*eit).second.root, 0)).first;
	}
	(*bit).second += sqrt(e.len_sq);
}

/* helper function:
 *
 * This is used to sort room_info_t pointers based on the area
 * of the rooms.
 */
bool compare_room_info_for_sorting(room_info_t* a, room_info_t* b)
{
	if(a != NULL && b != NULL)
		return (a->area < b->area);
	if(a != NULL)
		return true;
	return false;
}

int tri_rep_t::unlabel_extra_rooms()
{
	vector<room_info_t*> room_list;
	map<triple_t, room_info_t*> room_map;
	map<triple_t, room_info_t*>::iterator rit;
	vector<room_info_t*>::iterator vit;
	map<triple_t, tri_info_t>::iterator tit;
	map<triple_t, double>::iterator bit;
	triple_t root;
	tri_edge_t e;
	int num_rooms_unlabeled;
	bool unlabel;

	/* determine surface area and perimeter of each room,
	 * by iterating over the triangles and adding their
	 * geometry to the rooms */
	for(tit = this->tris.begin(); tit != this->tris.end(); tit++)
	{
		/* get the root of the current triangle's room */
		root = (*tit).second.root;

		/* get this room's info */
		rit = room_map.find(root);
		if(rit == room_map.end())
		{
			/* need to make a new room info for this root */
			room_list.push_back(new room_info_t(root));
			rit = room_map.insert(make_pair<triple_t,
				room_info_t*>(root, 
				room_list.back())).first;
		}

		/* add this triangle's area to total area of the room */
		(*rit).second->add_triangle((*tit).first, this->tri);

		/* incorporate each edge of this triangle
		 * into the boundary */
		e = tri_edge_t((*tit).first.i, (*tit).first.j, this->tri);
		this->add_edge_to_room(e, *((*rit).second));
		e = tri_edge_t((*tit).first.j, (*tit).first.k, this->tri);
		this->add_edge_to_room(e, *((*rit).second));
		e = tri_edge_t((*tit).first.k, (*tit).first.i, this->tri);
		this->add_edge_to_room(e, *((*rit).second));
	}

	/* sort rooms based on area */
	sort(room_list.begin(), room_list.end(),
			compare_room_info_for_sorting);

	/* check if each room is unnecesary by comparing
	 * its border with other rooms. */
	num_rooms_unlabeled = 0;
	for(vit = room_list.begin(); vit != room_list.end(); vit++)
	{
		/* iterate over boundary, checking if
		 *
		 * 	- the boundary is too big
		 * 	- the room on the other side is still labeled
		 */
		unlabel = false;
		for(bit = (*vit)->border_lengths.begin();
				bit != (*vit)->border_lengths.end();
					bit++)
		{
			/* check the length */
			if((*bit).second <= MAX_DOOR_WIDTH)
				continue; /* too small to merge */

			/* check that the room on the other side still
			 * has its label */
			tit = this->tris.find((*bit).first);
			if(tit == this->tris.end())
				continue;
			tit = this->tris.find((*tit).second.root);
			if(tit == this->tris.end())
				continue;
			if(!((*tit).second.is_local_max))
				continue;

			/* unlabel this room */
			unlabel = true;
			break;
		}

		/* check if we should unlabel this room */
		if(unlabel)
		{
			/* get the root of room */
			tit = this->tris.find((*vit)->root);
			if(tit == this->tris.end())
				continue;

			/* set root label to false */
			(*tit).second.is_local_max = false;
			num_rooms_unlabeled++;
		}
	}


	/* clean up allocated list */
	for(vit = room_list.begin(); vit != room_list.end(); vit++)
		delete (*vit);
	room_list.clear();

	/* return the number of rooms that were changed */
	return num_rooms_unlabeled;
}

int tri_rep_t::remove_unvisited_rooms(set<triple_t>& visited)
{
	set<triple_t> visited_rooms; /* stores the root of the room */
	vector<triple_t> to_remove;
	set<triple_t>::iterator it;
	map<triple_t, tri_info_t>::iterator mit;
	vector<triple_t>::iterator vit;
	triple_t r;

	/* iterate through the visited triangles */
	for(it = visited.begin(); it != visited.end(); it++)
	{
		/* check which room this triangle is in */
		mit = this->tris.find(*it);
		if(mit == this->tris.end())
			continue;

		/* add this room to the list of visited rooms */
		visited_rooms.insert(mit->second.root);
	}

	/* get a list of triangles to remove */
	for(mit = this->tris.begin(); mit != this->tris.end(); mit++)
	{
		/* check if root is visited */
		if(!(visited_rooms.count(mit->second.root)))
			to_remove.push_back(mit->first);
	}

	/* remove these triangles */
	for(vit = to_remove.begin(); vit != to_remove.end(); vit++)
		this->remove(*vit);

	/* return number of rooms removed */
	return to_remove.size();
}

int tri_rep_t::add_room_labels_to_graph()
{
	map<triple_t, int> room_ids;
	map<triple_t, int>::iterator rit;
	map<triple_t, tri_info_t>::iterator it;
	int i, num_rooms;

	/* go through the triangles defined in this graph, and 
	 * find each unique room */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* add this triangle's room's root to the map */
		room_ids.insert(make_pair<triple_t, int>(
					(*it).second.root, 0));
	}

	/* for each room, make a unique id */
	i = 0;
	for(rit = room_ids.begin(); rit != room_ids.end(); rit++)
	{
		/* make a new id for this room */
		(*rit).second = i++;
	}

	/* store the number of rooms */
	num_rooms = i;

	/* iterate over triangles */
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* get the room id for this triangle */
		rit = room_ids.find((*it).second.root);
		if(rit == room_ids.end())
			return -1;
		i = (*rit).second;

		/* for each triangle, iterate over vertices, and add
		 * this triangle's room to each of those vertices */
		((cell_t*) (TRI_VERTEX_POS(&(this->tri), (*it).first.i
				)->orig_data))->room_ids.insert(i);
		((cell_t*) (TRI_VERTEX_POS(&(this->tri), (*it).first.j
				)->orig_data))->room_ids.insert(i);
		((cell_t*) (TRI_VERTEX_POS(&(this->tri), (*it).first.k
				)->orig_data))->room_ids.insert(i);
	}

	/* success */
	return num_rooms;
}
	
void tri_rep_t::populate_room_heights()
{
	map<triple_t, vector<double> > room_min_z_vals;
	map<triple_t, vector<double> > room_max_z_vals;
	map<triple_t, vector<double> >::iterator fit, cit;
	map<triple_t, room_height_t>::iterator rit;
	map<triple_t, tri_info_t>::iterator tit;
	cell_t* c;

	/* clear fields */
	this->room_heights.clear();

	/* iterate over triangles */
	for(tit = this->tris.begin(); tit != this->tris.end(); tit++)
	{
		/* get this triangle's root's iterators */
		fit = room_min_z_vals.find(tit->second.root);
		if(fit == room_min_z_vals.end())
			fit = room_min_z_vals.insert(
				make_pair<triple_t, vector<double> >(
				tit->second.root, vector<double>())).first;
		cit = room_max_z_vals.find(tit->second.root);
		if(cit == room_max_z_vals.end())
			cit = room_max_z_vals.insert(
				make_pair<triple_t, vector<double> >(
				tit->second.root, vector<double>())).first;

		/* add the heights of each vertex to this room */
		c = (cell_t*) (TRI_VERTEX_POS(&(this->tri), 
					tit->first.i)->orig_data);	
		fit->second.push_back(c->min_z);
		cit->second.push_back(c->max_z);
		c = (cell_t*) (TRI_VERTEX_POS(&(this->tri), 
					tit->first.j)->orig_data);	
		fit->second.push_back(c->min_z);
		cit->second.push_back(c->max_z);
		c = (cell_t*) (TRI_VERTEX_POS(&(this->tri), 
					tit->first.k)->orig_data);	
		fit->second.push_back(c->min_z);
		cit->second.push_back(c->max_z);	
	}

	/* get the median range for each room */
	for(fit = room_min_z_vals.begin(); fit != room_min_z_vals.end(); 
								fit++)
	{
		/* get the iterator to the room height map */
		rit = this->room_heights.find(fit->first);
		if(rit == this->room_heights.end())
		{
			/* make a new entry */
			rit = this->room_heights.insert(
				make_pair<triple_t, room_height_t>(
				fit->first, room_height_t())).first;
		}

		/* get the median floor height */
		if(!(fit->second.empty()))
		{
			sort(fit->second.begin(), fit->second.end());
			rit->second.min_z = fit->second[
						fit->second.size()/2];
		}
		
		/* get the median ceiling height */
		cit = room_max_z_vals.find(fit->first);
		if(!(cit->second.empty()))
		{
			sort(cit->second.begin(), cit->second.end());
			rit->second.max_z = cit->second[
						cit->second.size()/2];
		}
	}
}

int tri_rep_t::verify()
{
	map<triple_t, tri_info_t>::iterator tit, tnit;
	map<int, set<triple_t> >::iterator vit;
	set<triple_t>::iterator nit;

	/* iterate over triangles */
	for(tit = this->tris.begin(); tit != this->tris.end(); tit++)
	{
		/* check that this triangle's vertices exist, and recognize
		 * this triangle as neighbors */
		vit = this->vert_map.find(tit->first.i);
		if(vit == this->vert_map.end())
			return -1;
		if(vit->second.find(tit->first) == vit->second.end())
			return -2;
		vit = this->vert_map.find(tit->first.j);
		if(vit == this->vert_map.end())
			return -3;
		if(vit->second.find(tit->first) == vit->second.end())
			return -4;
		vit = this->vert_map.find(tit->first.k);
		if(vit == this->vert_map.end())
			return -5;
		if(vit->second.find(tit->first) == vit->second.end())
			return -6;

		/* iterate over neighboring triangles.  verify that
		 * they agree on neighboring edge */
		for(nit = tit->second.neighs.begin();
				nit != tit->second.neighs.end(); nit++)
		{
			/* get neighbor info */
			tnit = this->tris.find(*nit);
			if(tnit == this->tris.end())
				return -7;
			if(tnit->second.neighs.find(tit->first)
					== tnit->second.neighs.end())
				return -8;

			/* verify that these two triangles share common
			 * vertices. */
			if(!(tnit->first.neighbors_with(tit->first)))
			{
				PRINT_ERROR("verification failed");
				cout << "[tri_rep_t.verify]\tTriangle ("
				     << tnit->first.i << ", "
				     << tnit->first.j << ", "
				     << tnit->first.k << ") thinks it's"
				     << " neighbors with triangle ("
				     << tit->first.i << ", "
				     << tit->first.j << ", "
				     << tit->first.k << ")" << endl;
				return -9;
			}
		}
	}

	/* iterate over vertices */
	for(vit = this->vert_map.begin(); 
				vit != this->vert_map.end(); vit++)
	{
		/* iterate over neighboring triangles of this vertex */
		for(nit = vit->second.begin(); 
				nit != vit->second.end(); nit++)
		{
			/* check that this neighboring triangle
			 * actually contains this vertex */
			if(!(nit->contains(vit->first)))
				return -10;
		}
	}

	/* no errors found */
	return 0;
}

void tri_rep_t::color_by_room(const triple_t& t, int& r, int& g, int& b)
{
	map<triple_t, tri_info_t>::iterator it;

	/* some default color */
	r = 255;
	g = 255;
	b = 255;

	/* get the root */
	it = this->tris.find(t);
	if(it == this->tris.end())
		return;

	/* randomly seed based on root */
	srand((3011*(it->second.root.i) + it->second.root.j)*3109 
					+ it->second.root.k);
	r = 64 + rand() % 128;
	g = 64 + rand() % 128;
	b = 64 + rand() % 128;
}

void tri_rep_t::color_by_room(int v, int& r, int& g, int& b)
{
	map<triple_t, tri_info_t>::iterator it;
	map<int, set<triple_t> >::iterator vit;
	set<triple_t> roots;
	set<triple_t>::iterator nit;

	/* some default color */
	r = 255;
	g = 255;
	b = 255;

	/* get neighboring triangles */
	vit = this->vert_map.find(v);
	if(vit == this->vert_map.end())
		return;

	/* compute the room roots of all triangles about this vertex */
	for(nit = vit->second.begin(); nit != vit->second.end(); nit++)
	{
		/* get triangle info */
		it = this->tris.find(*nit);
		if(it == this->tris.end())
			continue;

		/* add root to list */
		roots.insert(it->second.root);
	}

	/* color based on number of roots */
	switch(roots.size())
	{
		case 0:
			/* no rooms, color it white */
			return;
		case 1:
			/* randomly seed based on root */
			nit = roots.begin();
			srand((3011*(nit->i) + nit->j)*3109 + nit->k);
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

int tri_rep_t::print(char* filename)
{
	ofstream outfile;
	vertex_t* p;
	int num_verts_written;
	int r, g, b, p1, p2;

	map<triple_t, tri_info_t>::iterator it;

	/* open file for writing */
	outfile.open(filename);
	if(!(outfile.is_open()))
		return -1;

	/* print triangles in this trirep */
	num_verts_written = 0;
	p1 = 3011; p2 = 3109; /* large primes */	
	for(it = this->tris.begin(); it != this->tris.end(); it++)
	{
		/* compute random color based on root */
		srand((p1*(*it).second.root.i + (*it).second.root.j)*p2 
						+ (*it).second.root.k);
		r = rand() % 256;
		g = rand() % 256;
		b = rand() % 256;

		/* plot vertices */
		p = TRI_VERTEX_POS(&(this->tri), (*it).first.i);
		outfile << "v " << p->pos[0] 
		        << " " << p->pos[1] << " 0 "
			<< r << " " << g << " " << b << endl;
		p = TRI_VERTEX_POS(&(this->tri), (*it).first.j);
		outfile << "v " << p->pos[0] 
		        << " " << p->pos[1] << " 0 "
			<< r << " " << g << " " << b << endl;
		p = TRI_VERTEX_POS(&(this->tri), (*it).first.k);
		outfile << "v " << p->pos[0] 
		        << " " << p->pos[1] << " 0 "
			<< r << " " << g << " " << b << endl;
		
		/* plot triangle */
		outfile << "f " << (1+num_verts_written) 
			<<  " " << (2+num_verts_written)
			<<  " " << (3+num_verts_written)
			<< endl;
		num_verts_written += 3;
	
		/* plot local max centers */
		if((*it).second.is_local_max)
		{
			outfile << "v " << (*it).second.cc.pos[0]
				<<  " " << (*it).second.cc.pos[1]
				<< " 0 255 0 0"
				<< endl;
			num_verts_written++;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}

/**************** TRI_EDGE_T FUNCTIONS *******************/

tri_edge_t::tri_edge_t()
{
	this->len_sq = -1;
}

tri_edge_t::tri_edge_t(unsigned int i, unsigned int j, 
						triangulation_t& tri)
{
	vertex_t* pi, *pj;

	/* determine incident triangles */
	this->start.init(i, j, tri_get_apex(&tri, i, j));
	this->end.init(j, i, tri_get_apex(&tri, j, i));

	/* determine length of edge */
	pi = TRI_VERTEX_POS(&(tri), i);
	pj = TRI_VERTEX_POS(&(tri), j);
	this->len_sq = geom_dist_sq(pi, pj);
}

/**************** ROOM_INFO_T FUNCTIONS ******************/

room_info_t::room_info_t()
{
	this->area = 0;
}

room_info_t::room_info_t(triple_t& t)
{
	this->root = t;
	this->area = 0;
}

room_info_t::~room_info_t() {}

void room_info_t::add_triangle(const triple_t& t, triangulation_t& tri)
{
	vertex_t* pi, *pj, *pk;

	/* get the vertices of the triangle */
	pi = TRI_VERTEX_POS(&tri, t.i);
	pj = TRI_VERTEX_POS(&tri, t.j);
	pk = TRI_VERTEX_POS(&tri, t.k);

	/* add area of this triangle to total */
	this->area += geom_triangle_area(pi, pj, pk);
}
