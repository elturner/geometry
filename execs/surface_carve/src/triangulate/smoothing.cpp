#include "smoothing.h"
#include <map>
#include <set>
#include <vector>
#include "../structs/triangulation.h"
#include "../util/parameters.h"

using namespace std;

void smoothing_laplace(triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	vertex_t* v;
	double x_new, y_new, z_new, n;
	int i;

	/* iterate over all vertices */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* initialize counters (weight current pos x2,
		 * because neighbors will be double-counted) */
		x_new = 2*(*vit).second->x;
		y_new = 2*(*vit).second->y;
		z_new = 2*(*vit).second->z;
		n = 2;

		/* iterate over triangles of current vertex */
		for(tit = (*vit).second->mytris.begin(); 
				tit != (*vit).second->mytris.end(); tit++)
		{
			/* for all points not the current vertex,
			 * add to count */
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			{
				v = (*tit)->v[i];
				if(v != (*vit).second)
				{
					x_new += v->x;
					y_new += v->y;
					z_new += v->z;
					n++;
				}
			}
		}

		/* set position of current vertex to be average */
		(*vit).second->x = x_new / n;
		(*vit).second->y = y_new / n;
		(*vit).second->z = z_new / n;
	}
}

void smoothing_laplace_in_region(triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	vertex_t* v;
	double x_new, y_new, z_new, n;
	int i, r;
	bool is_good;

	/* iterate over all vertices */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* check that all triangles that contain this vertex
		 * belong to the same region. */
		r = -1;
		is_good = true;
		for(tit = (*vit).second->mytris.begin(); 
				tit != (*vit).second->mytris.end(); tit++)
		{
			if(r == -1)
				r = (*tit)->index;
			else if( (*tit)->index != r )
			{
				is_good = false;
				break;
			}
		}
		if(!is_good)
			continue;

		/* initialize counters (weight current pos x2,
		 * because neighbors will be double-counted) */
		x_new = 2*(*vit).second->x;
		y_new = 2*(*vit).second->y;
		z_new = 2*(*vit).second->z;
		n = 2;

		/* iterate over triangles of current vertex */
		for(tit = (*vit).second->mytris.begin(); 
				tit != (*vit).second->mytris.end(); tit++)
		{
			/* for all points not the current vertex,
			 * add to count */
			for(i = 0; i < NUM_VERTS_PER_TRI; i++)
			{
				v = (*tit)->v[i];
				if(v != (*vit).second)
				{
					x_new += v->x;
					y_new += v->y;
					z_new += v->z;
					n++;
				}
			}
		}

		/* set position of current vertex to be average */
		(*vit).second->x = x_new / n;
		(*vit).second->y = y_new / n;
		(*vit).second->z = z_new / n;
	}
}

void smoothing_laplace_region_edges(triangulation_t& tri)
{
	map<voxelface_t, vertex_t*>::iterator vit;
	vector<triangle_t*>::iterator tit;
	set<int>::iterator sit;
	set<int> vrs;
	vertex_t* v, *w;
	int num_regs, i;
	double x_new, y_new, z_new, n;

	/* iterate over each vertex in the triangulation */
	for(vit = tri.vertices.begin(); vit != tri.vertices.end(); vit++)
	{
		/* get the vertex in question */
		v = (*vit).second;
		if(!v)
			continue;

		/* iterate over all triangles that are incident to this
		 * vertex, determining which regions each triangle belongs
		 * to */
		vrs.clear();
		for(tit = v->mytris.begin(); tit != v->mytris.end(); tit++)
			vrs.insert((*tit)->region_id);

		/* check what subspace this vertex is restricted to */
		num_regs = vrs.size();
		if(num_regs > 1)
		{
			/* initialize counters (weight current pos x2,
			 * because neighbors will be double-counted) */
			x_new = 2*v->x;
			y_new = 2*v->y;
			z_new = 2*v->z;
			n = 2;

			/* iterate over triangles of current vertex */
			for(tit = v->mytris.begin(); tit != v->mytris.end();
								tit++)
			{
				/* error check */
				if(!(*tit))
					continue;

				/* for all points not the current vertex,
				 * add to count */
				for(i = 0; i < NUM_VERTS_PER_TRI; i++)
				{
					w = (*tit)->v[i];
					if(v != w && w)
					{
						x_new += w->x;
						y_new += w->y;
						z_new += w->z;
						n++;
					}
				}
			}

			/* set position of current vertex to be average */
			v->x = x_new / n;
			v->y = y_new / n;
			v->z = z_new / n;
		}
	}
}
