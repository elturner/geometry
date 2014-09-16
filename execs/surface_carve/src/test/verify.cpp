#include "verify.h"
#include <algorithm>
#include <map>
#include <vector>
#include <stdlib.h>
#include "../structs/triangulation.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

bool verify_triangulation(triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;

	/* iterate through all members of this triangulation,
	 * check each one */

	/* check triangles */
	for(tit = tri.triangles.begin(); tit != tri.triangles.end(); tit++)
		if(!verify_triangle(tri, *tit))
			return false;

	/* check vertices */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
		if(!verify_vertex(tri, (*vit).second))
			return false;

	/* all checks succeeded */
	return true;
}

bool verify_triangle(triangulation_t& tri, triangle_t* t)
{
	vector<triangle_t*> edge;
	vector<triangle_t*>::iterator it;
	int i, j, a, b;
	bool mutual;

	tri = tri;

	/* first, make sure that no NULL pointers exist
	 * in this triangle */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		if(t->v[i] == NULL)
		{
			LOGI("\n[verify]\tAt triangle %p\n", (void*) t);
			LOGI("\t\tt->v[%d] = NULL\n", i);
			return false;
		}

	/* check neighbor pointers as well */
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		if(t->t[i] == NULL)
		{
			LOGI("\n[verify]\tAt triangle %p\n", (void*) t);
			LOGI("\t\tt->t[%d] = NULL\n", i);
			return false;
		}

	/* check for unique vertices */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
		for(j = i+1; j < NUM_VERTS_PER_TRI; j++)
			if(t->v[i] == t->v[j])
			{
				LOGI("\n[verify]\tAt triangle %p\n",
							(void*) t);
				LOG("\t\tDuplicate vertices!\n");
			
				LOG("\t\tvertices:");
				for(j = 0; j < NUM_VERTS_PER_TRI; j++)
					LOGI("  %p", (void*) (t->v[j]));
				LOG("\n");
				return false;
			}

	/* check for unique neighbors */
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
		for(j = i+1; j < NUM_EDGES_PER_TRI; j++)
			if(t->t[i] == t->t[j])
			{
				LOGI("\n[verify]\tAt triangle %p\n",
							(void*) t);
				LOG("\t\tDuplicate neighbors!\n");
			
				LOG("\t\tneighbors:");
				for(j = 0; j < NUM_EDGES_PER_TRI; j++)
					LOGI("  %p", (void*) (t->v[j]));
				LOG("\n");
				return false;
			}

	/* check that each vertex knows that it is a part of t */
	for(i = 0; i < NUM_VERTS_PER_TRI; i++)
	{
		mutual = false;

		/* iterate over mytris of i'th vertex */
		for(it = t->v[i]->mytris.begin();
				it != t->v[i]->mytris.end(); it++)
		{
			/* look for t */
			if(t == *it)
			{
				mutual = true;
				break;
			}
		}

		/* check if mutual pointers exist */
		if(!mutual)
		{
			LOGI("\n[verify]\tAt triangle %p\n", (void*) t);
			LOGI("\t\tt->v[%d]", i);
			LOGI("\t(%p)\tdoes not point back.\n", 
						(void*) (t->v[i]));
			
			LOG("\t\tvertices:");
			for(j = 0; j < NUM_VERTS_PER_TRI; j++)
				LOGI("  %p", (void*) (t->v[j]));
			LOG("\n\t\tmytris:");
			for(it = t->v[i]->mytris.begin();
				it != t->v[i]->mytris.end(); it++)
				LOGI("  %p", (void*) *it);
			LOG("\n");

			return false;
		}
	}

	/* next, make sure that all neighboring triangles
	 * also have t as a neighbor */
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
	{
		/* iterate over the neighbors of t->t[i],
		 * looking for t */
		mutual = false;
		for(j = 0; j < NUM_EDGES_PER_TRI; j++)
			if(t->t[i]->t[j] == t)
			{
				mutual = true;
				break;
			}
		
		/* did we find a mutual pointer? */
		if(!mutual)
		{
			LOGI("\n[verify]\tAt triangle %p\n", (void*) t);
			LOGI("\t\tt->t[%d]", i);
			LOGI("\t(%p)\tdoes not point back.\n", 
						(void*) (t->t[i]));
			return false;
		}	
	}

	/* check for unique edges */
	for(i = 0; i < NUM_EDGES_PER_TRI; i++)
	{
		/* get the triangles that share both of
		 * the vertices that make up the edge
		 * between t and t->t[i].
		 *
		 * Hint: there should only be t and t->t[i] */
		a = (i+1) % NUM_EDGES_PER_TRI;
		b = (i+2) % NUM_EDGES_PER_TRI;

		edge.resize(t->v[a]->mytris.size()
				+ t->v[b]->mytris.size());

		/* get intersection */
		it = set_intersection(t->v[a]->mytris.begin(),
					t->v[a]->mytris.end(),
					t->v[b]->mytris.begin(),
					t->v[b]->mytris.end(),
					edge.begin());
		edge.resize(it - edge.begin());

		/* check intersection */
		if(edge.size() > 2)
		{
			LOGI("\n[verify]\tAt triangle %p\n", (void*) t);
			LOGI("\t\tEdge shared with %p is also shared by:"
					"\n",(void*) (t->t[i]));
			for(it = edge.begin(); it != edge.end(); it++)
			{
				LOGI("\t\t\t%p, verts:", (void*) *it);
				for(j = 0; j < NUM_VERTS_PER_TRI; j++)
					LOGI(" %p", (void*) (*it)->v[j]);
				LOG("\n");
			}
			LOGI("\t\tva = %p\n", (void*) t->v[a]);
			LOG("\t\tva->mytris:");
			for(it = t->v[a]->mytris.begin(); 
					it != t->v[a]->mytris.end(); it++)
				LOGI("  %p", (void*) (*it));
			LOGI("\n\t\tvb = %p\n", (void*) t->v[b]);
			LOG("\t\tvb->mytris:");
			for(it = t->v[b]->mytris.begin(); 
					it != t->v[b]->mytris.end(); it++)
				LOGI("  %p", (void*) (*it));
			LOG("\n\t\tvertices:");
			for(j = 0; j < NUM_VERTS_PER_TRI; j++)
				LOGI("  %p", (void*) (t->v[j]));
			LOG("\n\t\tneighbors:");
			for(j = 0; j < NUM_VERTS_PER_TRI; j++)
				LOGI("  %p", (void*) (t->t[j]));
			LOG("\n");
			return false;
		}
	}

	/* success */
	return true;
}

bool verify_vertex(triangulation_t& tri, vertex_t* v)
{
	vector<triangle_t*>::iterator it;
	triangle_t* t;
	int i, n;
	bool mutual;

	tri = tri;

	/* check that all triangles referenced by this vertex
	 * also have a reference back */
	for(it = v->mytris.begin(); it != v->mytris.end(); it++)
	{
		/* get current triangle */
		t = *it;

		/* check current triangle to make sure
		 * that v is a vertex of it */
		mutual = false;
		for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			if(t->v[i] == v)
			{
				mutual = true;
				break;
			}

		/* check if mutual */
		if(!mutual)
		{
			LOGI("\n[verify]\tAt vertex %p\n", (void*) v);
			LOGI("\t\ttriangle (%p) is referenced but "
				"doesn't contain this vertex.\n", 
							(void*) t);
			LOG("\t\tmytris:");
			n = v->mytris.size();
			for(i = 0; i < n; i++)
				LOGI(" %p", (void*) (v->mytris[i]));
			LOG("\n");
			return false;
		}	
	}

	/* all checks pass */
	return true;
}
