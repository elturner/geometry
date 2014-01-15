#include "triangulation.h"
#include "geometry.h"
#include "../../util/error_codes.h"
#include <stdlib.h>
#include <stdio.h>

/* the following used for dynamic resizing */
#define INIT_CAP     10
#define RESIZE_RATIO  2

void tri_init(triangulation_t* tri)
{
	/* check arguments */
	if(!tri)
		return;

	/* set all values to zero */
	tri->num_verts = 0;
	tri->vert_cap = 0;
	tri->vertices = NULL;
	tri->links = NULL;
	tri->starting_index = 0;
	tri->last_tri[0] = GHOST_VERTEX;
	tri->last_tri[1] = GHOST_VERTEX;
	tri->last_tri[2] = GHOST_VERTEX;
}

void tri_cleanup(triangulation_t* tri)
{
	int i, n;

	/* check arguments */
	if(!tri)
		return;

	/* check if already freed */
	if(!(tri->vertices))
		return;

	/* free vertex position array */
	free(tri->vertices);
	tri->vertices = NULL;

	/* free each link-ring */
	n = tri->num_verts;
	for(i = 0; i <= n; i++)
		linkring_cleanup((tri->links) + i);
		
	/* free link-ring array */
	free(tri->links);
	tri->links = NULL;

	/* set cap and len to zero */
	tri->num_verts = 0;
	tri->vert_cap = 0;
}

void tri_change_cap(triangulation_t* tri, unsigned int nc)
{
	tri->vertices = (vertex_t*) realloc(tri->vertices, 
					sizeof(vertex_t) * nc);
	tri->links = (linkring_t*) realloc(tri->links, 
					sizeof(linkring_t) * (nc+1));
	tri->vert_cap = nc;
}

int tri_add_vertex(triangulation_t* tri, vertex_t* v)
{
	unsigned int i, nc;

	/* check arguments */
	if(!tri || !v)
		return -1;

	/* resize arrays if necessary */
	while(tri->vert_cap <= tri->num_verts)
	{
		nc = (tri->vert_cap == 0) ? INIT_CAP 
					: (tri->vert_cap * RESIZE_RATIO);
		tri_change_cap(tri, nc);
	}

	/* add vertex position */
	i = tri->num_verts;
	if(vertex_copy(tri->vertices + i, v))
		return -2;

	/* add link-ring for ghost vertex, if it doesn't
	 * already exist */
	if(i == 0)
		linkring_init(tri->links);

	/* add new link-ring for this vertex */
	linkring_init(tri->links + i + 1);

	/* update count and return vertex number */
	tri->num_verts++;
	return IND_2_VERT_NUM(i);
}

int tri_set_neighbors(triangulation_t* tri, unsigned int v,
						linkring_t* neighs)
{
	linkring_t* lrt;
	linkring_t* nrt;
	int i, j, s;
	unsigned int w;

	/* check arguments */
	if(!tri || !neighs)
		return -1;

	/* get current link for v */
	lrt = TRI_GET_LINKRING(tri, v);
	if(!lrt)
		return -2;

	#ifdef SUPER_VERBOSE
		LOG("[tri_set_neighbors]\t( ");
		for(w = 0; w < neighs->len; w++)
		{
			LOGI("%d ", neighs->vertices[w]);
		}
		LOG(")\n");
	#endif

	/* remove triangles represented in current link.
	 * Assume this triangle is internal (which is fine as long
	 * as the ghost vertex is used), so just remove edges */
	s = lrt->len;
	for(i = 0; i < s; i++)
	{
		/* get the i'th neighbor of v */
		w = LINKRING_GET_VAL(lrt, i);
		nrt = tri->links + w;

		/* remove v from w's link-ring */
		j = linkring_find(nrt, v);
		if(j < 0)
			return PROPEGATE_ERROR(-3, j);
		j = linkring_remove(nrt, (unsigned int) j);
		if(j < 0)
			return PROPEGATE_ERROR(-4, j);
	}
	linkring_cleanup(lrt);

	/* set neighs as the link for v */
	j = linkring_move(lrt, neighs);
	if(j < 0)
		return PROPEGATE_ERROR(-5, j);

	/* go through the new link, and verify neighbors' links */
	s = lrt->len;
	for(i = 0; i < s; i++)
	{
		/* v is replacing some of the neighbors in each of its
		 * link-neighbors' link-rings.  We need to remove these
		 * neighbors.  Typically this is a symmetric operation,
		 * and we would need to check the soon-to-be-removed
		 * vertex's link-ring as well, but it is assumed that
		 * any neighbors removed are also neighbors of v, so this
		 * operation will be performed anyway.
		 *
		 * That is, if v's linkring has the subsequence
		 * [...w0,w1,w2,...] , then the edges (w1,w0) and (w1,w2)
		 * should already exist.  However, other edges may also
		 * exist between these two when counting counter-clockwise
		 * around w1.  It is assumed that these edges are (w1,x), 
		 * where x is in the given neighs.
		 */
		
		w = LINKRING_GET_VAL(lrt, i);
		nrt = tri->links + w;
		j = linkring_replace_range(nrt, LINKRING_NEXT_VAL(lrt, i),
				LINKRING_PREV_VAL(lrt, i), v);
		if(j)
			return PROPEGATE_ERROR(-6, j);
	}

	/* success */
	return 0;
}

int tri_get_apex(triangulation_t* tri, unsigned int v0, unsigned int v1)
{
	linkring_t* lrt;
	int i;

	/* check arguments */
	if(!tri)
		return -1;
	if(tri->num_verts < v0)
		return -2;
	if(tri->num_verts < v1)
		return -3;

	/* get link-ring for v0 */
	lrt = tri->links + v0;

	/* make sure there is more than one value
	 * in this link-ring.  Otherwise no triangles
	 * are defined */
	if(lrt->len < 2)
		return -4;

	/* find v1 in this ring */
	i = linkring_find(lrt, v1);
	if(i < 0)
		return -5;

	/* get the next value in the linkring */
	return LINKRING_NEXT_VAL(lrt, i);
}

int tri_get_directions(triangulation_t* tri, vertex_t* start, 
		vertex_t* pos, unsigned int v0, unsigned int v1,
					unsigned int v2)
{
	linkring_t* lr0;
	int i;
	vertex_t* v0p, *v1p, *v2p;
	double o0, o1, o2;

	/* check arguments */
	if(!tri || !pos || !start)
		return -1;
	if(tri->num_verts < v0)
		return -2;
	if(tri->num_verts < v1)
		return -3;
	if(tri->num_verts < v2)
		return -4;

	/* make sure that triangle (v0,v1,v2) exists in tri */
	lr0 = TRI_GET_LINKRING(tri, v0);
	i = linkring_find(lr0, v1);
	if(i < 0)
		return PROPEGATE_ERROR(-5,i);

	if(LINKRING_NEXT_VAL(lr0, i) != v2)
		return -6;

	/* get positions of v0, v1, and v2 */
	v0p = TRI_VERTEX_POS(tri, v0);
	v1p = TRI_VERTEX_POS(tri, v1);
	v2p = TRI_VERTEX_POS(tri, v2);

	/* Check if pos is on the edge of the triangle.
	 * This check must be done to see if triangle on
	 * other side of edge is exterior (has ghost vertex).
	 * If so, we want to prefer the exterior triangle.  Otherwise
	 * we want to be inclusive with defining interior of triangle. */
	i = geom_ontriangleedge(v0p,v1p,v2p,pos);
	if(i < 0)
		return PROPEGATE_ERROR(-7,i);
	switch(i)
	{
		/* if on edge, check if other side of
		 * edge is ghost vertex (i.e. this is a
		 * boundary edge).  If so, then traverse
		 * edge.  If not, then pos is inside
		 * triangle. */
		case 0:
			if(tri_get_apex(tri, v2, v1) == GHOST_VERTEX)
				return i;
			return 3;
		case 1:
			if(tri_get_apex(tri, v0, v2) == GHOST_VERTEX)
				return i;
			return 3;
		case 2:
			if(tri_get_apex(tri, v1, v0) == GHOST_VERTEX)
				return i;
			return 3;

			/* not on edge */
		default:
			break;
	}

	/* check if pos is inside triangle */
	if(geom_intriangle(v0p, v1p, v2p, pos))
		return 3;

	/* get outgoing edge */
	o0 = geom_orient2D(start, pos, v0p);
	o1 = geom_orient2D(start, pos, v1p);
	o2 = geom_orient2D(start, pos, v2p);

	if(o0 > 0 && o2 < 0)
	{
		/* path intersects edge (2,0) */
		return 1;
	}
	else if(o1 > 0 && o0 < 0)
	{
		/* path intersects edge (0,1) */
		return 2;
	}
	else if(o2 > 0 && o1 < 0)
	{
		/* path intersects edge (1,2) */
		return 0;
	}
	else
	{
		/* edge case: if traversal line is completely disjoint
		 * from triangle, just return the "best" edge, which points
		 * in the general direction of pos */
		o0 = geom_orient2D(v2p, v1p, pos);
		o1 = geom_orient2D(v0p, v2p, pos);
		o2 = geom_orient2D(v1p, v0p, pos);

		if(o0 >= o1 && o0 >= o2)
			return 0;
		else if(o1 >= o0 && o1 >= o2)
			return 1;
		else
			return 2;
	}

}

int tri_locate(triangulation_t* tri, vertex_t* v,
		unsigned int s0, unsigned int s1, unsigned int s2,
		unsigned int* v0, unsigned int* v1, unsigned int* v2)
{
	int valid_start, i;
	linkring_t* lrt;
	vertex_t start;
	vertex_t* s0p, *s1p, *s2p;
	unsigned int stemp, sa_old, sb_old;
	
	/* check arguments */
	if(!tri || !v || !v0 || !v1 || !v2)
		return -1;

	/* determine if given valid starting triangle */
	valid_start = 0;
	if(s0 <= tri->num_verts)
	{
		lrt = tri->links + s0;
		i = linkring_find(lrt, s1);
		if(i >= 0 && LINKRING_NEXT_VAL(lrt, i) == s2)
			valid_start = 1;
	}

	/* if not given a valid starting triangle, pick arbitrary
	 * triangle */
	while(!valid_start)
	{
		s0 = 1 + (rand() % (tri->num_verts));
		lrt = tri->links + s0;

		if(lrt->len < 2)
			continue;

		s1 = LINKRING_GET_VAL(lrt,0);
		s2 = LINKRING_GET_VAL(lrt,1);
		valid_start = 1;
	}
	
	/* get starting location as center of starting triangle */
	s0p = TRI_VERTEX_POS(tri, s0);
	s1p = TRI_VERTEX_POS(tri, s1);
	s2p = TRI_VERTEX_POS(tri, s2);

	/* find center of triangle */
	if(geom_center(s0p, s1p, s2p, &start))
		return -2;

	/* Next, traverse the graph from triangle to triangle to
	 * find v */
	sa_old = sb_old = 0;
	while(1)
	{
		/* get directions from current triangle to destination */
		i = tri_get_directions(tri, &start, v, s0, s1, s2);
	
		if(i < 0)
		{
			/* error occurred */
			return PROPEGATE_ERROR(-3,i);
		}
		else if(i == 3)
		{
			/* current triangle contains v */
			break;
		}
		else
		{
			/* i will tell which edge of triangle to cross
			 * in order to move closer to vertex v.  For
			 * a given value of i, keep the other two
			 * corners of the triangle, and use the triangle
			 * whose third corner is the apex of the kept
			 * vertices */
			switch(i)
			{
				case 0:
					i = tri_get_apex(tri,s2,s1);
					if(i < 0)
						return -4;

					/* check if we're looping */
					if(sa_old == s1 && sb_old == s2)
					{
						LOG("LOOPING!\n");
						*v0 = s0;
						*v1 = s1;
						*v2 = s2;
						return 0;
					}
					sa_old = s2;
					sb_old = s1;

					/* cross this edge */
					s0 = i;
					stemp = s1;
					s1 = s2;
					s2 = stemp;
					break;
				case 1:
					i = tri_get_apex(tri,s0,s2);
					if(i < 0)
						return -5;

					/* check if we're looping */
					if(sa_old == s2 && sb_old == s0)
					{
						LOG("LOOPING!\n");
						*v0 = s0;
						*v1 = s1;
						*v2 = s2;
						return 0;
					}
					sa_old = s0;
					sb_old = s2;

					/* cross this edge */
					s1 = i;
					stemp = s0;
					s0 = s2;
					s2 = stemp;
					break;
				case 2:
					i = tri_get_apex(tri,s1,s0);
					if(i < 0)
						return -6;

					/* check if we're looping */
					if(sa_old == s0 && sb_old == s1)
					{
						LOG("LOOPING!\n");
						*v0 = s0;
						*v1 = s1;
						*v2 = s2;
						return 0;
					}
					sa_old = s1;
					sb_old = s0;

					/* cross this edge */
					s2 = i;
					stemp = s1;
					s1 = s0;
					s0 = stemp;
					break;
			}
		}
	}

	/* save final triangle to given pointers */
	*v0 = s0;
	*v1 = s1;
	*v2 = s2;
	return 0;
}

int tri_verify_delaunay(triangulation_t* tri)
{
	int r;
	unsigned int i, j, nv, nl, w, s;
	linkring_t* lrt;
	vertex_t* ip, *wp, *sp, *rp;
	double area;

	/* check arguments */
	if(!tri)
		return 0;

	/* iterate through triangulation's link-rings, ignoring
	 * the link-ring of the ghost vertex */
	nv = tri->num_verts;
	for(i = 1; i <= nv; i++)
	{
		lrt = tri->links + i;
		nl = lrt->len;

		/* if there are two vertex nums in the linkset
		 * then we only have one triangle. If there are
		 * N > 2 nums, there are N triangles. */
		nl = (nl < 2) ? 0 : ( (nl == 2) ? 1 : nl );

		for(j = 0; j < nl; j++)
		{
			w = LINKRING_GET_VAL(lrt,j);
			s = LINKRING_NEXT_VAL(lrt,j);
			
			/* (i, w, s) is a triangle
			 * in this triangulation.  However, since
			 * each triangle is represented multiple times,
			 * only print if i is the minimum vertex num */
			if(i < w && i < s)
			{
				/* get vertex positions of this triangle */
				ip = TRI_VERTEX_POS(tri,i);
				wp = TRI_VERTEX_POS(tri,w);
				sp = TRI_VERTEX_POS(tri,s);

				/* verify that this triangle is listed
				 * in counter-clockwise order */
				area = geom_orient2D(ip,wp,sp);
				if(area < 0)
					return 0;
				if(area == 0 && ip && wp && sp)
				{
					LOG("ZERO-AREA TRIANGLE\n");
					return 0;
				}

				/* for each side of the triangle, check
				 * if the apex is inside the circumcircle */
				r = tri_get_apex(tri, w, i);
				if(r < 0)
					return 0;
				rp = TRI_VERTEX_POS(tri,r);
				if(geom_incircle(ip, wp, sp, rp) > 0)
					return 0;

				r = tri_get_apex(tri, s, w);
				if(r < 0)
					return 0;
				rp = TRI_VERTEX_POS(tri,r);
				if(geom_incircle(ip, wp, sp, rp) > 0)
					return 0;

				r = tri_get_apex(tri, i, s);
				if(r < 0)
					return 0;
				rp = TRI_VERTEX_POS(tri,r);
				if(geom_incircle(ip, wp, sp, rp) > 0)
					return 0;
			}
		}
	}

	/* this triangulation is delaunay */
	return 1;
}

void tri_print(triangulation_t* t)
{
	unsigned int i, j;

	printf("\n-------Triangulation---------\n\n");

	if(!t)
	{
		printf("NULL\n");
		return;
	}

	printf("num_verts : %d\t\t(cap %d)\n\n", t->num_verts, t->vert_cap);
	
	for(i = 0; i < t->num_verts; i++)
	{
		printf("vert[%d] = (%f, %f)\t%p\n", i+1, 
			t->vertices[i].pos[0],
			t->vertices[i].pos[1], t->vertices[i].orig_data);
	}

	printf("\n");

	for(i = 0; i <= t->num_verts; i++)
	{
		printf("links[%d] = ( ", i);
		for(j = 0; j < t->links[i].len; j++)
			printf("%d ", t->links[i].vertices[j]);
		printf(")\n");
	}

	printf("\n-----------------------------\n\n");
}
