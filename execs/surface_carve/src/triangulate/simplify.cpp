#include "simplify.h"
#include <algorithm>
#include <vector>
#include <set>
#include <stdlib.h>
#include "region_growing.h"
#include "smoothing.h"
#include "../structs/triangulation.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

/*** DEBUGGING FUNCTIONS ***/

void print_vertex(vertex_t* v)
{
	vector<triangle_t*>::iterator it;

	LOGI("vertex: %p\n\tmytris:", (void*) v);
	for(it = v->mytris.begin(); it != v->mytris.end(); it++)
		LOGI(" %p", (void*) (*it));
	LOG("\n");
}

void print_triangle(triangle_t* t)
{
	int i;

	LOGI("triangle: %p\n\tmy vertices:", (void*) t);
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		LOGI(" %p", (void*) (t->v[i]));
	LOG("\n\tmy neighbors:");
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		LOGI(" %p", (void*) (t->t[i]));
	LOG("\n");
}

/**** IMPLEMENTED FUNCTIONS ****/

void simplify_triangulation(triangulation_t& tri, 
		vector<planar_region_t>& rl)
{
	unsigned int i, n;
	
	/* to ease triangle removal, sort triangles in tri */
	sort(tri.triangles.begin(), tri.triangles.end());

	/* iterate over all regions, simplifying */
	n = rl.size();
	for(i = 0; i < n; i++)
	{
		/* only simplify regions that are complex enough */
		if(rl[i].tris.size() < MIN_SNAP_REGION_SIZE)
			continue;

		/* simplify! */
		simplify_region(tri, rl[i]);
	}
	
	/* Some triangles may be incorrectly oriented, which
	 * can be mitigated by smoothing */
	for(i = 0; i < SIMPLIFICATION_SMOOTHING_ROUNDS; i++)
		smoothing_laplace_in_region(tri);
}

void simplify_region(triangulation_t& tri, planar_region_t& reg)
{
	set<triangle_t*>::iterator it;
	vector<triangle_t*>::iterator mit, oit;
	triangle_t* ta, *tb, *tc, *td, *te, *tf;
	vertex_t* vc, *vd, *va2b, *vb2a, *v;
	int i, j, a2b, b2a, c, d, e, f, c2a, d2a, e2b, f2b;
	bool is_good;
	
	/* iterate over triangles of region reg */
	it = reg.tris.begin();
	while(it != reg.tris.end())
	{
		/* get current triangle */
		ta = (*it);
		it++;

		/* check for triangles with all-region neighbors */
		if(ta->region_neigh_count != NUM_EDGES_PER_TRI)
			continue;
	
		/* all neighbors should also have full count */
		if(ta->t[0]->region_neigh_count != NUM_EDGES_PER_TRI)
			continue;
		if(ta->t[1]->region_neigh_count != NUM_EDGES_PER_TRI)
			continue;
		if(ta->t[2]->region_neigh_count != NUM_EDGES_PER_TRI)
			continue;

		/* find a neighboring triangle to collapse with */
		is_good = false;
		for(a2b = 0; a2b < NUM_EDGES_PER_TRI; a2b++)
		{
			tb = ta->t[a2b];

			/* find tb's pointer to ta */
			b2a = -1;
			for(i = 0; i < NUM_EDGES_PER_TRI; i++)
				if(ta->t[a2b]->t[i] == ta)
				{
					b2a = i;
					break;
				}
			if(b2a < 0)
			{
				LOG("[simplify]\tbad mesh (0)\n");
				return;
			}

			/* verify that all neighbors of tb 
			 * have full count */
			if(tb->t[0]->region_neigh_count 
						!= NUM_EDGES_PER_TRI)
				continue;
			if(tb->t[1]->region_neigh_count 
						!= NUM_EDGES_PER_TRI)
				continue;
			if(tb->t[2]->region_neigh_count 
						!= NUM_EDGES_PER_TRI)
				continue;
		
			/* tb passed all checks, let's use it */
			is_good = true;
			break;
		}
		if(!is_good)
			continue;

		/* Now we want to remove ta and tb from the
		 * triangulation.  First, get the 
		 * neighbor pointers for the other triangles */
		c = (a2b+1) % NUM_EDGES_PER_TRI;
		d = (a2b+2) % NUM_EDGES_PER_TRI;
		tc = ta->t[c];
		td = ta->t[d];

		/* make tc and td point to each other.  To do
		 * that, we need to know what neighbor index 
		 * ta is of them */
		c2a = -1;
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			if(tc->t[i] == ta)
			{
				c2a = i;
				break;
			}
		d2a = -1;
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			if(td->t[i] == ta)
			{
				d2a = i;
				break;
			}
		if(c2a < 0 || d2a < 0)
		{
			LOG("[simplify_region]\tbad mesh! (1)\n");
			return;
		}

		/* do the same thing for the neighbors of tb */
		e = (b2a+1) % NUM_EDGES_PER_TRI;
		f = (b2a+2) % NUM_EDGES_PER_TRI;
		te = tb->t[e];
		tf = tb->t[f];

		/* make te and tf point to each other.  To do
		 * that, we need to know what neighbor index 
		 * ta is of them */
		e2b = -1;
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			if(te->t[i] == tb)
			{
				e2b = i;
				break;
			}
		f2b = -1;
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
			if(tf->t[i] == tb)
			{
				f2b = i;
				break;
			}
		if(e2b < 0 || f2b < 0)
		{
			LOG("[simplify_region]\tbad mesh! (2)\n");
			return;
		}
		
		/* get pointers to relevant vertices */
		vc = ta->v[c];
		vd = ta->v[d];
		va2b = ta->v[a2b];
		vb2a = tb->v[b2a];

		/* verify that ta and tb share the vertices we think
		 * they share */
		if(tb->v[f] != vc || tb->v[e] != vd)
		{
			LOG("[simplify_region]\tbad mesh! (3)\n");
			continue;
		}
	
		/* To prevent degenerate triangles, make sure that
		 * tc, td, te, tf don't share any edges and are not
		 * pointing to the same triangles */
		if(tc == td || tc == te || tc == tf 
				|| td == te || td == tf || te == tf)
			continue;
		if(vc == vd || vc == va2b || vc == vb2a 
				|| vd == va2b || vd == vb2a 
				|| va2b == vb2a)
			continue;
		is_good = true;
		for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		{
			if(tc->t[i] == td || tc->t[i] == te 
				|| tc->t[i] == tf || td->t[i] == tc 
				|| td->t[i] == te || td->t[i] == tf
				|| te->t[i] == tc || te->t[i] == td 
				|| te->t[i] == tf || tf->t[i] == tc 
				|| tf->t[i] == td || tf->t[i] == te)
			{
				is_good = false;
				break;
			}
		}
		if(!is_good)
			continue;

		/* to prevent degenerate triangles, there can not
		 * be any point p such that the edge vc-p exists and
		 * vd-p exists (other than the points va2b and vb2a).
		 * Thus we must check for this by iterating over
		 * all neighboring triangles for vc and vd. */
		is_good = true;
		for(mit = vc->mytris.begin(); is_good 
					&& mit != vc->mytris.end(); mit++)
		{
		for(oit = vd->mytris.begin(); is_good &&
					oit != vd->mytris.end(); oit++)
		{
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		{
		for(j = 0; j < NUM_VERTS_PER_TRI; j++)
		{
		if((*mit)->v[i] == (*oit)->v[j])
		{
			v = (*mit)->v[i];
			if(v != va2b && v != vb2a && v != vc && v != vd)
			{
				is_good = false;
				break;
			}
		}
		}
		}
		}
		}
		if(!is_good)
			continue;
	
		/*** PAST THIS POINT IS WHERE THE MESH IS MODIFIED ***/
	
		/* next, move a vertex to the midpoint of the
		 * edge to be collapsed */
		simplify_set_edge_center(vc, vd);
		
		/* now that we have the values of
		 * c2a and d2a, make tc and td point
		 * to each other directly */
		tc->t[c2a] = td;
		td->t[d2a] = tc;
		ta->t[c] = NULL;
		ta->t[d] = NULL;

		/* now that we have the values of
		 * e2b and f2b, make tc and td point
		 * to each other directly */
		te->t[e2b] = tf;
		tf->t[f2b] = te;
		tb->t[e] = NULL;
		tb->t[f] = NULL;

		/* remove ta and tb from relevant mytris lists */
		
		/* find ta in vertex a2b */
		for(mit = va2b->mytris.begin();
			mit != va2b->mytris.end(); mit++)
			if((*mit) == ta)
			{
				va2b->mytris.erase(mit);
				ta->v[a2b] = NULL;
				break;
			}

		/* find tb in vertex b2a */
		for(mit = vb2a->mytris.begin();
			mit != vb2a->mytris.end(); mit++)
			if((*mit) == tb)
			{
				vb2a->mytris.erase(mit);
				tb->v[b2a] = NULL;
				break;
			}

		/* find ta and tb in vertex c */
		for(mit = vc->mytris.begin();
			mit != vc->mytris.end(); mit++)
			if((*mit) == ta)
			{
				vc->mytris.erase(mit);
				ta->v[c] = NULL;
				break;
			}

		for(mit = vc->mytris.begin();
			mit != vc->mytris.end(); mit++)
			if((*mit) == tb)
			{
				vc->mytris.erase(mit);
				tb->v[f] = NULL;
				break;
			}
	
		/* find ta and tb in vertex d */
		for(mit = vd->mytris.begin();
			mit != vd->mytris.end(); mit++)
			if((*mit) == ta)
			{
				vd->mytris.erase(mit);
				ta->v[d] = NULL;
				break;
			}

		for(mit = vd->mytris.begin();
			mit != vd->mytris.end(); mit++)
			if((*mit) == tb)
			{
				vd->mytris.erase(mit);
				tb->v[e] = NULL;
				break;
			}
			
		/* make all references to vd instead be
		 * to the vertex vc */
		for(mit = vd->mytris.begin(); 
				mit != vd->mytris.end(); mit++)
		{
			/* debug, make sure this triangle is
			 * neither td nor te */
			if(*mit == td || *mit == te)
			{
				LOG("\n[simplify]\tgot labels switched!\n");
				LOG("mit\t"); print_triangle(*mit);
			}

			/* since vc is taking the place of vd,
			 * any triangles that are pointed to by
			 * vd should now be pointed to by vc 
			 * (except for ta,tb, which will be deleted) */
			if(*mit == ta || *mit == tb)
				continue;

			/* find the reference in this triangle
			 * to vd, and switch it to vc */
			is_good = false;
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				if((*mit)->v[i] == vd)
				{
					(*mit)->v[i] = vc;
					vc->mytris.push_back(*mit);
					is_good = true;
					break;
				}
			if(!is_good)
			{
				LOG("[simplify]\tnon-mutual"
					" pointers found!\n");
				return;
			}
		}
		vd->mytris.clear();

		/* clean up vc->mytris */
		sort(vc->mytris.begin(), vc->mytris.end());
		mit = unique(vc->mytris.begin(), vc->mytris.end());
		vc->mytris.resize(mit - vc->mytris.begin());

		/* debug, check that tc, tf see vc */
		is_good = false;
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			if(tc->v[i] == vc)
			{
				is_good = true;
				break;
			}
		if(!is_good)
		{
			LOG("[simplify]\ttc does not see vc!\n");
			return;
		}
		is_good = false;
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			if(tf->v[i] == vc)
			{
				is_good = true;
				break;
			}
		if(!is_good)
		{
			LOG("[simplify]\ttf does not see vc!\n");
			return;
		}

		/* remove current triangles from triangulation */
		mit = lower_bound(tri.triangles.begin(),
				tri.triangles.end(), ta);
		if((*mit) != ta)
		{
			LOG("[simplify]\tbad triangulation, "
					"can't find ta.\n");
			return;
		}
		tri.triangles.erase(mit);
		mit = lower_bound(tri.triangles.begin(),
				tri.triangles.end(), tb);
		if((*mit) != tb)
		{
			LOG("[simplify]\tbad triangulation, "
					"can't find tb.\n");
			return;
		}
		tri.triangles.erase(mit);
		
		/* remove redundant vertex */
		tri.vertices.erase(vd->hash);

		/* remove from region */
		reg.tris.erase(ta);
		reg.tris.erase(tb);
	
		/* free memory */
		delete ta;
		delete tb;
		delete vd;
		ta = tb = NULL;
		vd = NULL;

		/* must reset iterator since 
		 * elements were removed */
		it = reg.tris.begin();	
	}
}

void simplify_set_edge_center(vertex_t* vc, vertex_t* vd)
{
	vector<triangle_t*>::iterator it;
	double x, y, z;
	int i, n;

	/* initialize counter */
	n = 0;
	x = y = z = 0;

	/* we want to compute the average position of the link-ring
	 * of the edge vc-vd.  So iterate over all of the neighbors
	 * of both of these vertices. */
	for(it = vc->mytris.begin(); it != vc->mytris.end(); it++)
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			if((*it)->v[i] != vc && (*it)->v[i] != vd)
			{
				x += (*it)->v[i]->x;
				y += (*it)->v[i]->y;
				z += (*it)->v[i]->z;
				n++;
			}

	/* do the same thing for the neighbors of vd */
	for(it = vd->mytris.begin(); it != vd->mytris.end(); it++)
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			if((*it)->v[i] != vc && (*it)->v[i] != vd)
			{
				x += (*it)->v[i]->x;
				y += (*it)->v[i]->y;
				z += (*it)->v[i]->z;
				n++;
			}

	/* get average */
	x /= n;
	y /= n;
	z /= n;

	/* set position */
	vc->x = x;
	vc->y = y;
	vc->z = z;
}
