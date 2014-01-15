#include "reordering.h"
#include "triangulation/triangulation.h"
#include "triangulation/geometry.h"
#include "../util/error_codes.h"
#include <stdlib.h>
#include <math.h>

#define BRIO_MIN_ROUND_SIZE 1000

int reorder_BRIO(triangulation_t* tri)
{
	int ret, len, rs, ri;

	/* check argument */
	if(!tri)
		return -1;

	/* randomize the order of the points */
	ret = reorder_randomize(tri);
	if(ret)
		return PROPEGATE_ERROR(-2,ret);

	/* get the number of vertices in this
	 * triangulation, and use this to determine
	 * the size of each round */
	len = tri->num_verts;
	ri = len;
	for(rs = len/2 ; rs > BRIO_MIN_ROUND_SIZE ; rs /= 2)
	{
		/* ri is the first index of the current round
		 * rs is the number of indices in the current round */
		ri = ri - rs;

		/* z-order sort the vertices in this round */
		ret = reorder_z_order_sort(tri->vertices + ri, rs);
		if(ret)
			return PROPEGATE_ERROR(-3,ret);
	}

	/* get final round [0,ri) */
	ret = reorder_z_order_sort(tri->vertices, ri);
	if(ret)
		return PROPEGATE_ERROR(-4,ret);

	/* make sure the first triangle is not colinear */
	ret = reorder_maximize_first_area(tri);
	if(ret)
		return PROPEGATE_ERROR(-5,ret);

	/* success */
	return 0;
}

int reorder_randomize(triangulation_t* tri)
{
	int N, j, k, num_swaps;
	vertex_t dummy;
	vertex_t* jp, *kp;

	/* check arguments */
	if(!tri)
		return -1;

	/* make 2N swaps */
	N = tri->num_verts;
	for(num_swaps = 2*N ; num_swaps > 0 ; num_swaps--)
	{
		/* pick two indices to switch */
		j = rand() % N;
		k = rand() % N;
		
		jp = tri->vertices + j;
		kp = tri->vertices + k;

		/* switch j'th and k'th vertices */
		if(vertex_copy(&dummy, jp))
			return -3;
		if(vertex_copy(jp, kp))
			return -4;
		if(vertex_copy(kp, &dummy))
			return -5;
	}

	/* success */
	return 0;
}

int reorder_maximize_first_area(triangulation_t* tri)
{
	unsigned int i, n;
	vertex_t* p, *q, *r;
	vertex_t dummy;
	double a_max, a;

	/* check argument */
	if(!tri)
		return -1;

	/* check trivial edge case */
	n = tri->num_verts;
	if(n <= 3)
		return 0;

	/* get positions of first two vertices */
	p = tri->vertices;
	q = tri->vertices + 1;
	if(!p || !q)
		return -2;

	/* iterate through remaining vertices, checking
	 * potential first-triangle areas */
	a_max = fabs(geom_orient2D(p, q, tri->vertices + 2));
	for(i = 3; i < n; i++)
	{
		/* get i'th vertex */
		r = tri->vertices + i;

		/* check area of p,q,r */
		a = fabs(geom_orient2D(p, q, r));
		if(a > a_max)
		{
			a_max = a;
		
			/* switch 2nd and i'th vertices */
			if(vertex_copy(&dummy, r))
				return -3;
			if(vertex_copy(r, tri->vertices + 2))
				return -4;
			if(vertex_copy(tri->vertices + 2, &dummy))
				return -5;
		}
	}
	
	/* success */
	return 0;
}

int reorder_z_order_sort(vertex_t* list, int len)
{
	int i;
	double z_order_min_x, z_order_min_y, z_order_max_x, z_order_max_y;
	double z_order_precision_x, z_order_precision_y;
	double x,y;
	unsigned int j, x_ind, y_ind, f_ind;

	/* check arguments */
	if(!list || len <= 0)
		return -1;

	/* determine the extent of the point-cloud being
	 * sorted, and how many significant figures to use
	 * when performing a z-order sort */
	z_order_min_x = list[0].pos[VERTEX_X_IND];
	z_order_min_y = list[0].pos[VERTEX_Y_IND];
	z_order_max_x = list[0].pos[VERTEX_X_IND];
	z_order_max_y = list[0].pos[VERTEX_Y_IND];
	for(i = 1; i < len; i++)
	{
		x = list[i].pos[VERTEX_X_IND];
		y = list[i].pos[VERTEX_Y_IND];

		if(z_order_min_x > x)
			z_order_min_x = x;
		if(z_order_max_x < x)
			z_order_max_x = x;
		if(z_order_min_y > y)
			z_order_min_y = y;
		if(z_order_max_y < y)
			z_order_max_y = y;
	}
	z_order_precision_x = ((double) len) 
				/ (z_order_max_x - z_order_min_x);
	z_order_precision_y = ((double) len) 
				/ (z_order_max_y - z_order_min_y);

	/* iterate through vertices, and record index for z-ordering */
	for(i = 0; i < len; i++)
	{
		x = list[i].pos[VERTEX_X_IND];
		y = list[i].pos[VERTEX_Y_IND];

		/* get indices in each dimension */
		x_ind = (unsigned int) ((x - z_order_min_x) 
						* z_order_precision_x);
		y_ind = (unsigned int) ((y - z_order_min_y) 
						* z_order_precision_y);
	
		/* record final index by interweiving x_ind and y_ind */
		f_ind = 0;
		for(j = 0; j < sizeof(unsigned int); j += 2)
		{
			f_ind |= ((x_ind >>  j   ) & 1);
			f_ind |= ((y_ind >> (j+1)) & 1);
		}
		list[i].z_order_index = f_ind;
	}

	/* sort this list based on the calculated z-ordering */
	qsort(list, len, sizeof(vertex_t), reorder_z_order_comp);
	return 0;
}

int reorder_z_order_comp(const void* pp, const void* qp)
{
	if(!pp || !qp)
		return 0;
	
	/* get the z-order indices for each vertex */
	return ( ((vertex_t*) pp)->z_order_index 
				- ((vertex_t*) qp)->z_order_index);
}
