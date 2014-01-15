#ifndef LINK_RING_H
#define LINK_RING_H

/* linkring.h
 *
 * This file defines the structure
 * used to represent link-rings within
 * the triangulation data structure.
 *
 * The following implementation uses
 * a dynamically allocated integer array,
 * which is assumed to be circularly indexed.
 */

typedef struct linkring
{
	/* len is the number of vertices
	 * represented in this link-ring */
	unsigned int len;
	
	/* cap is the size of memory of
	 * the vertices array */
	unsigned int cap;

	/* a dynamically-allocated array of
	 * vertex indices */
	unsigned int* vertices;
} linkring_t;

/* The following macros allow for circular
 * access of a link-ring */
#define LINKRING_GET_VAL(lrt,i) ((lrt)->vertices[ (i) % ((lrt)->len) ])
#define LINKRING_NEXT_IND(lrt,i) ( ((i)+1) % ((lrt)->len) )
#define LINKRING_PREV_IND(lrt,i) ( ((i)-1+(lrt)->len) % ((lrt)->len) )
#define LINKRING_NEXT_VAL(lrt,i) ((lrt)->vertices[LINKRING_NEXT_IND(lrt,i)])
#define LINKRING_PREV_VAL(lrt,i) ((lrt)->vertices[LINKRING_PREV_IND(lrt,i)])

/* linkring_init:
 *
 * 	Will initialize memory to be
 *	an empty link-ring.
 *
 * arguments:
 *
 * 	lrt -	Pointer to memory to init
 */
void linkring_init(linkring_t* lrt);

/* linkring_cleanup:
 *
 * 	Will free any allocated resources used
 * 	by the given link-ring.  The result will
 * 	still be a valid link-ring of length 0.
 *
 * arguments:
 *
 * 	lrt -	The linkring to clean-up
 */
void linkring_cleanup(linkring_t* lrt);

/* linkring_clear:
 *
 * 	Sets length of link-ring to zero.
 *
 * arguments:
 *
 * 	lrt -	Link-ring to modify
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int linkring_clear(linkring_t* lrt);

/* linkring_add:
 *
 * 	Will add vertex v into the linkring lrt
 * 	at index i.
 *
 * arguments:
 *
 * 	lrt -	The linkring to modify
 *	v -	The vertex index to insert
 *	i -	The location within the link-ring to insert v
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int linkring_add(linkring_t* lrt, unsigned int v, unsigned int i);

/* linkring_remove:
 *
 * 	Will remove the i'th vertex within the
 * 	given link-ring.
 *
 * arguments:
 *
 * 	lrt -	The link-ring to modify
 * 	i -	The index within the link-ring of
 * 		the element to remove.
 *
 * return value:
 *
 * 	Returns the value of the element removed
 * 	on success, negative value on failure.
 */
int linkring_remove(linkring_t* lrt, unsigned int i);

/* linkring_find:
 *
 * 	Will attempt to find the given vertex index within
 * 	the specified link-ring.
 *
 * arguments:
 *
 * 	lrt -	The link-ring to analyze
 * 	v -	The vertex index to find
 *
 * return value:
 *
 * 	Returns the index of v if found.  Returns -1 if
 * 	not found.
 */
int linkring_find(linkring_t* lrt, unsigned int v);

/* linkring_move:
 *
 *	Will copy the data stored in src to dest, and
 *	will reset src to an empty link-ring.
 *
 * arguments:
 *
 * 	dest -	Where to store the data that is in src
 * 	src -	The link-ring to move
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int linkring_move(linkring_t* dest, linkring_t* src);

/* linkring_replace_range:
 *
 * 	Given a link-ring that contains the values v0 and vf, will
 * 	delete all elements between v0 and vf (moving counter-clockwise)
 * 	and add the vertex index w between v0.
 *
 * 	The operation is:
 *
 * 		[ ... v0, v1, v2, ..., vf, ... ] => [ ..., v0, w, vf, ... ]
 *
 * arguments:
 *
 *	lrt -	The link-ring to modify
 *	v0 -	The beginning of the sequence to replace (exclusive)
 *	vf -	The end of the sequence to replace (exclusive)
 *	w -	The value to place between v0 and vf
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int linkring_replace_range(linkring_t* lrt, unsigned int v0,
				unsigned int vf, unsigned int w);

#endif
