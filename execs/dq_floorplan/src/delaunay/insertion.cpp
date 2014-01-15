#include "insertion.h"
#include <stdlib.h>
#include "triangulation/triangulation.h"
#include "triangulation/vertex.h"
#include "triangulation/linkring.h"
#include "triangulation/geometry.h"
#include "../util/error_codes.h"

int begin_triangulation(triangulation_t* tri)
{
	unsigned int v1, v2;
	linkring_t lrt;
	int ret;

	/* check argument */
	if(!tri)
		return -1;
	if(tri->num_verts < 2)
		return -2; /* can't triangulate */

	/* pick two vertices to create the first edge */
	v1 = 1;
	v2 = 2;

	/* add triangles including the ghost vertex */
	linkring_init(&lrt);
	ret = linkring_add(&lrt, GHOST_VERTEX, 0);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	ret = linkring_add(&lrt, v2, 1);
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	ret = tri_set_neighbors(tri, v1, &lrt);
	if(ret)
		return PROPEGATE_ERROR(-5, ret);

	/* record most recent triangle */
	tri->last_tri[0] = GHOST_VERTEX;
	tri->last_tri[1] = v1;
	tri->last_tri[2] = v2;

	/* clean up */
	linkring_cleanup(&lrt);
	return 0;
}

int insert_vertex(triangulation_t* tri, unsigned int v)
{
	vertex_t* vp;
	unsigned int s0, s1, s2, f0, f1, f2;
	int ret;
	linkring_t lrt;

	/* check arguments */
	if(!tri)
		return -1;
	
	if(v > tri->num_verts)
		return -2;

	/********************************/
	/* get triangle that contains v */
	/********************************/
	
	vp = TRI_VERTEX_POS(tri,v);
	
	/* store starting triangle in (s0,s1,s2) */
	s0 = tri->last_tri[0];
	s1 = tri->last_tri[1];
	s2 = tri->last_tri[2];

	/* after the following call, (f0,f1,f2) will be the
	 * triangle that contains v */
	ret = tri_locate(tri, vp, s0, s1, s2, &f0, &f1, &f2);
	if(ret < 0)
		return PROPEGATE_ERROR(-3, ret);

	/**************************************************************/
	/* remove all triangles that contain v in their circumcircles */
	/**************************************************************/

	linkring_init(&lrt);

	/* check if (f0,f1,f2) contains the ghost vertex. If so, we need
	 * to traverse the outer edge to find all edges that face v */
	if(f0 == GHOST_VERTEX)
	{
		/* fill lrt with (f1,f2) */
		if(linkring_add(&lrt, f1, 0))
			return -4;
		if(linkring_add(&lrt, f2, 1))
			return -5;

		/* search outer edge */
		ret = search_outer_edge(tri, vp, &lrt);
		if(ret)
			return PROPEGATE_ERROR(-6, ret);
	}
	else if(f1 == GHOST_VERTEX)
	{
		/* fill lrt with (f2,f0) */
		if(linkring_add(&lrt, f2, 0))
			return -7;
		if(linkring_add(&lrt, f0, 1))
			return -8;

		/* search outer edge */
		ret = search_outer_edge(tri, vp, &lrt);
		if(ret)
			return PROPEGATE_ERROR(-9, ret);
	}
	else if(f2 == GHOST_VERTEX)
	{
		/* fill lrt with (f0,f1) */
		if(linkring_add(&lrt, f0, 0))
			return -10;
		if(linkring_add(&lrt, f1, 1))
			return -11;

		/* search outer edge */
		ret = search_outer_edge(tri, vp, &lrt);
		if(ret)
			return PROPEGATE_ERROR(-12, ret);
	}
	else
	{
		/* fill lrt with (f0,f1,f2) */
		if(linkring_add(&lrt, f0, 0))
			return -13;
		if(linkring_add(&lrt, f1, 1))
			return -14;
		if(linkring_add(&lrt, f2, 2))
			return -15;
		
		/* (f0,f1,f2) is a fully-enclosed triangle.  Thus we
		 * need to do a graph search of neighbors to determine
		 * which have v in their circumcircle */
	}

	ret = search_circumcircles(tri, vp, &lrt);
	if(ret)
		return PROPEGATE_ERROR(-16, ret);

	/**************************************************************/
	/* add v to the triangulation using the constructed link-ring */
	/**************************************************************/

	if(lrt.len < 3)
		return -17;

	ret = tri_set_neighbors(tri, v, &lrt);
	if(ret)
		return PROPEGATE_ERROR(-18, ret);

	/* record most recent triangle */
	tri->last_tri[0] = v;
	tri->last_tri[1] = tri->links[v].vertices[0];
	tri->last_tri[2] = tri->links[v].vertices[1];

	/* success */
	linkring_cleanup(&lrt);
	return 0;
}

int search_circumcircles(triangulation_t* tri, vertex_t* vp, 
				linkring_t* lrt)
{
	unsigned int p, q, i;
	int r, ret;
	vertex_t* pp, *qp, *rp;

	/* check arguments */
	if(!tri || !vp || !lrt)
		return -1; /* bad pointers */
	if(lrt->len < 3)
		return -2; /* need to start with containing triangle */
	
	/* search all edges (Depth-First Search) by iterating over
	 * cavity edges */
	i = 0;
	while(i < lrt->len)
	{
		#ifdef SUPER_VERBOSE
			LOG("[search_circumcircles]\tsearch so far: ( ");
			for(q = 0; q < lrt->len; q++)
				if(q == i)
					LOGI("%d_", lrt->vertices[q]);
				else
					LOGI("%d ", lrt->vertices[q]);
			LOG(")\n");
		#endif

		/* get edge (i,i+1) from lrt */
		q = LINKRING_GET_VAL(lrt,i);
		p = LINKRING_NEXT_VAL(lrt,i);
		r = tri_get_apex(tri, p, q);

		/* verify that (p,q,r) is a valid triangle in tri */
		if(r < 0)
			return PROPEGATE_ERROR(-4, r);

		/* get vertex positions for (p,q,r) */
		pp = TRI_VERTEX_POS(tri,p);
		qp = TRI_VERTEX_POS(tri,q);
		rp = TRI_VERTEX_POS(tri,r);

		/* check if vp is inside this triangle */
		if(geom_incircle(pp,qp,rp,vp) > 0)
		{
			/* since vp is inside the circumcircle of this
			 * triangle, we need to add this triangle's border
			 * to lrt. This is done by just adding r at position
			 * i+1 */
			ret = linkring_add(lrt,r,i+1);
			if(ret)
				return PROPEGATE_ERROR(-5, ret);
		}
		else
		{
			/* since vp is outside the circumcircle, 
			 * increment i, do not check this edge again,
			 * and move to the next edge in the cavity 
			 *
			 * This branch will automatically occur if p, q,
			 * or r are the ghost vertex */
			i++;
		}
	}

	/* success */
	return 0;
}

int search_outer_edge(triangulation_t* tri, vertex_t* vp, linkring_t* lrt)
{
	int i, j, ret;
	linkring_t* gvlr;
	unsigned int p, q;
	vertex_t* pp, *qp;

	/* check arguments */
	if(!tri || !vp || !lrt)
		return -1;
	
	if(lrt->len != 2)
		return -2; /* must specify boundary edge */

	/* get ghost vertex link-ring */
	gvlr = tri->links; 
	if(!gvlr)
		return -3;

	/* get index of the first vertex of lrt in gvlr */
	i = linkring_find(gvlr, lrt->vertices[0]);
	if(i < 0)
		return PROPEGATE_ERROR(-4, i);

	/* verify that the next vertex is the second element of lrt */
	if(LINKRING_NEXT_VAL(gvlr, i) != lrt->vertices[1])
		return -5;

	/* check edges, traversing counter-clockwise */
	j = i; /* j represents the index of the second vertex 
	        * in the current edge */
	while(j != i+1)
	{
		/* get vertex numbers of current edge */
		p = LINKRING_PREV_VAL(gvlr, j);
		q = LINKRING_GET_VAL(gvlr, j);

		/* get positions of the vertices in the current edge */
		pp = TRI_VERTEX_POS(tri, p);
		qp = TRI_VERTEX_POS(tri, q);

		if(!pp || !qp)
			return -6;

		/* check their orientation with vp */
		if(geom_orient2D(pp, qp, vp) > 0)
		{
			/* if oriented, then add to lrt (at the front) */
			ret = linkring_add(lrt, p, 0);
			if(ret)
				return PROPEGATE_ERROR(-7, ret);
		}
		else
		{
			break; /* stop searching in this direction */
		}

		/* search next boundary edge in counter-clockwise 
		 * direction */
		j = LINKRING_PREV_IND(gvlr,j);
	}

	/* check edges, traversing clockwise */
	j = i+1; /* j represents the index of the first vertex 
	          * in the current edge */
	while(j != i)
	{
		/* get vertex numbers of current edge */
		p = LINKRING_GET_VAL(gvlr, j);
		q = LINKRING_NEXT_VAL(gvlr, j);

		/* get positions of the vertices in the current edge */
		pp = TRI_VERTEX_POS(tri, p);
		qp = TRI_VERTEX_POS(tri, q);

		if(!pp || !qp)
			return -8;

		/* check their orientation with vp */
		if(geom_orient2D(pp, qp, vp) > 0)
		{
			/* if oriented, then add to lrt (at the back) */
			ret = linkring_add(lrt, q, lrt->len);
			if(ret)
				return PROPEGATE_ERROR(-9, ret);
		}
		else
		{
			break; /* stop searching in this direction */
		}

		/* search next boundary edge in clockwise direction */
		j = LINKRING_NEXT_IND(gvlr,j);
	}
	
	/* add the ghost vertex at the end */
	if(linkring_add(lrt, GHOST_VERTEX, lrt->len))
		return -10;

	/* success */
	return 0;
}
