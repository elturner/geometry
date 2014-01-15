#include "vertex.h"
#include <stdio.h>

int vertex_set(vertex_t* v, double x, double y)
{
	/* check arguments */
	if(!v)
		return -1;

	/* set position */
	v->pos[VERTEX_X_IND] = x;
	v->pos[VERTEX_Y_IND] = y;

	/* success */
	return 0;
}

int vertex_copy(vertex_t* dest, vertex_t* src)
{
	/* check arguments */
	if(!dest || !src)
		return -1;

	/* copy values */
	dest->pos[VERTEX_X_IND] = src->pos[VERTEX_X_IND];
	dest->pos[VERTEX_Y_IND] = src->pos[VERTEX_Y_IND];
	dest->orig_data = src->orig_data;
	dest->z_order_index = src->z_order_index;

	/* success */
	return 0;
}

void vertex_print(vertex_t* p)
{
	if(!p)
		printf("< Null vertex >\n");
	else
		printf("< %f, %f > (%p)\n", p->pos[VERTEX_X_IND], 
				p->pos[VERTEX_Y_IND], p->orig_data);
}
