#include "union_find.h"
#include <set>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include "../structs/triangulation.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

void remove_small_unions(triangulation_t& tri, int tus)
{
	vector<vector<int> > unions;
	set<vertex_t*> vertices_to_delete;
	set<vertex_t*>::iterator vit;
	set<int> tris_to_delete;
	set<int>::reverse_iterator tit;
	triangle_t* t;
	int i, j, k, r, n, m;

	/* compute union-find on this triangulation */
	union_find(unions, tri);

	/* initialize lists of objects to delete */
	vertices_to_delete.clear();
	tris_to_delete.clear();

	/* iterate over unions, looking for small ones */
	n = unions.size();
	for(i = 0; i < n; i++)
	{
		/* check if the i'th union is small enough */
		m = unions[i].size();
		if(m >= tus)
			continue;

		/* Remove all triangles in this union. */
		for(j = 0; j < m; j++)
		{
			/* get triangle to delete */
			k = unions[i][j];

			/* record the vertices of this triangle */
			for(r = 0; r < NUM_VERTS_PER_TRI; r++)
				vertices_to_delete.insert(
					tri.triangles[k]->v[r]);

			/* remove this triangle.  Since we
			 * are removing entire unions, don't worry
			 * about resetting neighbor pointers. */
			tris_to_delete.insert(k);
		}
	}
	
	/* remove all marked vertices */
	for(vit = vertices_to_delete.begin(); 
				vit != vertices_to_delete.end(); vit++)
	{
		/* delete this vertex and free memory */
		tri.vertices.erase((*vit)->hash);
		delete (*vit);
	}
	
	/* remove all marked triangles.  Since
	 * these indices are sorted, going backwards
	 * preserves the indices of anything yet to
	 * be deleted */
	for(tit = tris_to_delete.rbegin(); tit != tris_to_delete.rend();
								tit++)
	{
		/* delete this triangle and free memory */
		k = *tit;
		t = tri.triangles[k];
		tri.triangles.erase(tri.triangles.begin() + k);
		delete t;
	}
}

void color_by_union(triangulation_t& tri)
{
	vector<vector<int> > unions;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	int i, j, k, n, m;

	/* compute union-find on triangulation */
	union_find(unions, tri);

	/* iterate over unions */
	n = unions.size();
	for(i = 0; i < n; i++)
	{
		/* create a random color for each union */
		red   = (unsigned char) ((rand() % 156) + 100);
		green = (unsigned char) ((rand() % 156) + 100);
		blue  = (unsigned char) ((rand() % 156) + 100);

		/* iterate over all triangles in i'th union */
		m = unions[i].size();
		for(j = 0; j < m; j++)
		{
			/* iterate over the vertices of this triangle */
			for(k = 0; k < NUM_VERTS_PER_TRI; k++)
			{
				tri.triangles[unions[i][j]]->v[k]->red 
								= red;
				tri.triangles[unions[i][j]]->v[k]->green 
								= green;
				tri.triangles[unions[i][j]]->v[k]->blue 
								= blue;
			}
		}
	}
}

void union_find(vector<vector<int> >& unions, triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	vector<int> forest;
	vector<int> roots;
	vector<int>::iterator it;
	int i, M, r, ri;

	/* initialize forest such that each element represents one
	 * triangle */
	M = tri.triangles.size();
	forest.resize(M);
	for(i = 0; i < M; i++)
		forest[i] = i;

	/* force triangles to have proper indices */
	tri.index_triangles();

	/* iterate over each vertex in tri, and connect
	 * all triangles that contain this vertex */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* find the smallest root among this vertex's triangles */
		r = -1;
		for(tit = (*vit).second->mytris.begin(); 
				tit != (*vit).second->mytris.end(); tit++)
		{
			i = (*tit)->index;
			ri = get_root(forest, i);
			if(r < 0 || (ri >= 0 && ri < r))
				r = ri;
		}

		/* set the root of all of these triangles to be r */
		for(tit = (*vit).second->mytris.begin(); 
				tit != (*vit).second->mytris.end(); tit++)
		{
			i = (*tit)->index;
			ri = get_root(forest, i);
			if(ri >= 0 && r >= 0)
				forest[ri] = r;
		}
	}

	/* fully simplify tree, and count roots */
	roots.clear();
	for(i = 0; i < M; i++)
	{
		ri = get_root(forest, i);
		if(ri == i)
			roots.push_back(i);
	}
	
	/* initialize union list */
	unions.clear();
	unions.resize(roots.size());

	/* fill unions list */
	for(i = 0; i < M; i++)
	{
		ri = get_root(forest, i);
		it = lower_bound(roots.begin(), roots.end(), ri);
		unions[(int) (it - roots.begin())].push_back(i);
	}
}

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

	/* simplify tree */
	forest[i] = r;

	/* return */
	return r;
}
