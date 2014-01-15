#include "linkring.h"
#include "../../util/error_codes.h"
#include <stdlib.h>

/* the following defines are constants used by
 * this data structure */
#define INIT_CAP 10
#define RESIZE_RATIO 2

void linkring_init(linkring_t* lrt)
{
	/* check arguments */
	if(!lrt)
		return;

	/* set array to length zero */
	lrt->len = 0;
	lrt->cap = 0;
	lrt->vertices = NULL;
}

void linkring_cleanup(linkring_t* lrt)
{
	/* check arguments */
	if(!lrt)
		return;

	/* free the vertex array */
	free(lrt->vertices);
	lrt->vertices = NULL;
	lrt->cap = 0;
	lrt->len = 0;
}

int linkring_clear(linkring_t* lrt)
{
	/* check arguments */
	if(!lrt)
		return -1;

	lrt->len = 0;
	return 0;
}

int linkring_add(linkring_t* lrt, unsigned int v, unsigned int i)
{
	unsigned int nc;
	int j;

	/* check arguments */
	if(!lrt)
		return -1;
	if(i > lrt->len)
		return -2;

	/* resize if necessary */
	while(lrt->len >= lrt->cap)
	{
		nc = (lrt->cap == 0) ? INIT_CAP : (RESIZE_RATIO * lrt->cap);
		lrt->vertices = (unsigned int*) realloc(lrt->vertices,
					sizeof(unsigned int) * nc);
		lrt->cap = nc;
	}

	/* iterate backwards from end of array, shifting values */
	for(j = ((int) lrt->len)-1; j >= (int) i; j--)
		lrt->vertices[j+1] = lrt->vertices[j];
	
	/* add v at i */
	lrt->vertices[i] = v;
	lrt->len++;

	/* success */
	return 0;
}

int linkring_remove(linkring_t* lrt, unsigned int i)
{
	unsigned int j;

	/* check arguments */
	if(!lrt)
		return -1;
	if(i >= lrt->len)
		return -2;

	/* for each element at index > i, shift back */
	for(j = i+1; j < lrt->len; j++)
		lrt->vertices[j-1] = lrt->vertices[j];

	/* resize and report success */
	lrt->len--;
	return 0;
}

int linkring_find(linkring_t* lrt, unsigned int v)
{
	unsigned int i, n;
	
	/* check arguments */
	if(!lrt)
		return -1;
	
	n = lrt->len;
	if(!n)
		return -1;

	/* iterate over array */
	for(i = 0; i < n; i++)
		if(lrt->vertices[i] == v)
			return i;
	
	/* didn't find squat */
	return -1;
}

int linkring_move(linkring_t* dest, linkring_t* src)
{
	/* check arguments */
	if(!dest || !src)
		return -1;

	/* make sure dest doesn't have anything stored */
	if(dest->vertices != NULL)
		return -2;

	/* copy info */
	dest->vertices = src->vertices;
	dest->len = src->len;
	dest->cap = src->cap;

	/* clear src */
	src->vertices = NULL;
	src->len = 0;
	src->cap = 0;

	/* success */
	return 0;
}

int linkring_replace_range(linkring_t* lrt, unsigned int v0,
				unsigned int vf, unsigned int w)
{
	int v0i, vfi, wi, i, s, d;

	/* check arguments */
	if(!lrt)
		return -1;

	/* find positions of v0 and vf in lrt */
	v0i = linkring_find(lrt, v0);
	vfi = linkring_find(lrt, vf);
	wi = linkring_find(lrt, w);

	/* make sure w is not in lrt */
	if(wi >= 0)
	{
		/* we'll be adding wi in soon anyway, so
		 * remove this extra reference for now */
		if(linkring_remove(lrt, wi))
			return -2;
	}

	/* check what's in the linkring */
	if(lrt->len == 0)
	{
		/* insert both v0 and vf, then return */
		if(linkring_add(lrt, v0, 0))
			return -3;
		if(linkring_add(lrt, w, 1))
			return -4;
		if(v0 != vf)
			if(linkring_add(lrt, vf, 2))
				return -5;
		return 0;
	}
	else if(v0 == vf)
		return -3;
	else if(v0i < 0 && vfi < 0)
		return -5;
	else if(v0i < 0)
	{
		/* insert v0 before vf, and return */
		if(linkring_add(lrt, w, vfi))
			return -6;
		if(linkring_add(lrt, v0, vfi))
			return -7;
		return 0;
	}
	else if(vfi < 0)
	{
		/* insert vf after v0 and return */
		if(linkring_add(lrt, vf, v0i+1))
			return -7;
		if(linkring_add(lrt, w, v0i+1))
			return -8;
		return 0;
	}

	/* delete elements between v0 and vf, and add w */
	if(v0i + 1 == vfi)
	{
		/* don't need to remove anything, just
		 * add w between these values */
		if(linkring_add(lrt, w, vfi))
			return -9;
	}
	else if(v0i == ((int)(lrt->len)) - 1 && vfi == 0)
	{
		/* don't need to remove anything, just
		 * tack w onto end */
		if(linkring_add(lrt, w, lrt->len))
			return -10;
	}
	else if(v0i < vfi)
	{
		/* set voi + 1 to be w */
		lrt->vertices[v0i+1] = w;

		/* shift indices vfi and above
		 * down to remove all others */
		d = vfi - v0i - 2;
		s = lrt->len;
		for(i = vfi; i < s; i++)
			lrt->vertices[i-d] = lrt->vertices[i];
		
		/* update length of link-ring */
		lrt->len -= d;
	}
	else
	{
		/* Need to remove all values before vfi and after
		 * v0i. We know there are a non-zero number of these
		 * elements */
		d = vfi;
		if(d == 1)
		{
			/* rather than shifting all the kept elements
			 * down by one, just put w in at index 0 */
			lrt->vertices[0] = w;
		}
		else
		{
			if(d > 0)
			{
				/* shift kept elements down by d */
				for(i = vfi; i <= v0i; i++)
					lrt->vertices[i-d]
						= lrt->vertices[i];
			}

			/* add w to end of link-ring */
			lrt->vertices[v0i-d+1] = w;
		}

		/* update length of link-ring */
		lrt->len = v0i - vfi + 2;
	}

	/* success */
	return 0;
}
