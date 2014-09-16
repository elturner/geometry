#include "union_find_faces.h"
#include <map>
#include <vector>
#include <algorithm>
#include "../structs/mesher.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

int remove_small_unions_faces(mesher_t& mesh, int tus)
{
	vector<vector<face_t> > unions;
	int i, j, n, m, ret;

	/* compute union-find on this triangulation */
	ret = union_find_faces(unions, mesh);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* iterate over unions, looking for small ones */
	n = unions.size();
	for(i = 0; i < n; i++)
	{
		/* check if the i'th union is small enough */
		m = unions[i].size();
		if(m >= tus)
			continue;
		
		/* Remove all faces in this union. We don't need
		 * to worry about neighbor pointers, since any
		 * face that has this face as a neighbor will also
		 * be removed */
		for(j = 0; j < m; j++)
			mesh.graph.erase(unions[i][j]);
	}

	/* success */
	return 0;
}

int union_find_faces(vector<vector<face_t> >& unions, mesher_t& mesh)
{
	map<face_t, face_state_t>::iterator git;
	map<face_t, int>::iterator iit, nit;
	map<face_t, int> faces;
	vector<int> forest;
	vector<int> roots;
	vector<int>::iterator it;
	int i, j, k, M, r, rj;

	/* store all the faces and their indices */
	i = 0;
	for(git = mesh.graph.begin(); git != mesh.graph.end(); git++)
	{
		faces.insert(pair<face_t, int>((*git).first, i));
		i++;
	}
	M = i;

	/* initialize forest such that each element represents one face_t */
	forest.resize(M);
	for(i = 0; i < M; i++)
		forest[i] = i;

	/* iterate over each face in this list, and connect
	 * all faces with their neighbors */
	for(iit = faces.begin(); iit != faces.end(); iit++)
	{
		/* get info about this face */
		i = iit->second;
		r = get_root_faces(forest, i);
		git = mesh.graph.find(iit->first);
		if(git == mesh.graph.end())
			return -1;
		
		/* find the smallest root among this face's neighbors */
		for(k = 0; k < NUM_EDGES_PER_SQUARE; k++)
		{
			/* get the neighbor's index */
			nit = faces.find(git->second.neighbors[k]);
			if(nit == faces.end())
				return -2;

			/* check neighbor's root */
			j = nit->second;
			rj = get_root_faces(forest, j);
			if(r < 0 || (rj >= 0 && rj < r))
				r = rj;
		}

		/* check that root is valid */
		if(r < 0)
			return -3;

		/* set the root of all of these faces to be r */
		forest[i] = r;
		for(k = 0; k < NUM_EDGES_PER_SQUARE; k++)
		{
			/* get the neighbor's index */
			nit = faces.find(git->second.neighbors[k]);
			if(nit == faces.end())
				return -2;

			/* update neighbor's root */
			j = nit->second;
			rj = get_root_faces(forest, j);
			if(rj >= 0 && r >= 0)
				forest[rj] = r;
		}
	}

	/* fully simplify tree, and count roots */
	roots.clear();
	for(i = 0; i < M; i++)
	{
		r = get_root_faces(forest, i);
		if(r == i)
			roots.push_back(i);
	}
	
	/* initialize union list */
	unions.clear();
	unions.resize(roots.size());

	/* fill unions list */
	for(iit = faces.begin(); iit != faces.end(); iit++)
	{
		i = (*iit).second;
		r = get_root_faces(forest, i);
		it = lower_bound(roots.begin(), roots.end(), r);
		unions[(int) (it - roots.begin())].push_back((*iit).first);
	}

	/* success */
	return 0;
}

int get_root_faces(vector<int>& forest, int i)
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
	r = get_root_faces(forest, p);

	/* simplify tree */
	forest[i] = r;

	/* return */
	return r;
}
