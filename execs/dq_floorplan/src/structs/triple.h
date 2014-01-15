#ifndef TRIPLE_H
#define TRIPLE_H

/* triple.h:
 *
 * 	This file contains classes used to mark the topology
 * 	of triangulations, where vertices are referenced by
 * 	integer indices.  The triple_t class is used to
 * 	represent triangles, and the edge_t class is used to
 * 	represent edges.
 */

#include <set>

using namespace std;

/* these classes are defined in this file */
class triple_t;
class edge_t;

/* The following will store three comparable values, in sorted order */
class triple_t
{
	/*** parameters ***/
	public:

	/* the values stored in this struct in sorted order */
	int a, b, c;

	/* the values in their original ordering */
	int i, j, k;

	/*** functions ***/
	public:

	/* constructors */
	
	triple_t();
	triple_t(int ii, int jj, int kk);
	~triple_t();

	void init(int ii, int jj, int kk);

	/* accessors */

	/* get:
	 *
	 * 	Get's the specified element of this triple.
	 */
	inline int get(int ind) const
	{
		switch(ind)
		{
			case 0:
				return this->i;
			case 1:
				return this->j;
			case 2:
				return this->k;
			default:
				return -1;
		}
	};

	/* analysis */

	/* unique:
	 *
	 * 	Returns true if all elements in this struct
	 * 	are unique (i.e. no duplicates appear)
	 */
	inline bool unique() const
	{
		/* a,b,c are sorted elements */
		return (this->a != this->b) && (this->b != this->c);
	};

	/* neighbors_with
	 *
	 * 	Returns true iff the two given triples share at
	 * 	least two elements in common.
	 */
	bool neighbors_with(const triple_t& other) const;

	/* contains:
	 *
	 * 	Returns true iff this triple contains the argument.
	 */
	bool contains(int x) const;

	/* get_edges:
	 *
	 * 	Adds the edges of this triple to the specified
	 * 	container.
	 *
	 * arguments:
	 *
	 * 	loc -	Where to insert the edges of this triple
	 */
	void get_edges(set<edge_t>& loc) const;

	/* operators */

	inline bool operator < (const triple_t& rhs) const
	{
		if(this->a < rhs.a)
			return true;
		if(this->a > rhs.a)
			return false;
		if(this->b < rhs.b)
			return true;
		if(this->b > rhs.b)
			return false;
		return (this->c < rhs.c);
	};
	inline bool operator == (const triple_t& rhs) const
	{
		return (this->a == rhs.a && this->b == rhs.b 
					&& this->c == rhs.c);
	};
	inline bool operator != (const triple_t& rhs) const
	{
		return (this->a != rhs.a || this->b != rhs.b 
					|| this->c != rhs.c);
	};
	inline triple_t operator = (const triple_t& rhs)
	{
		this->a = rhs.a;
		this->b = rhs.b;
		this->c = rhs.c;
		this->i = rhs.i;
		this->j = rhs.j;
		this->k = rhs.k;
		return (*this);
	};
};

/* this class is used to represent edges in a triangulation by 
 * pairs of integers. */
class edge_t
{
	/*** parameters ***/
	public:

	/* values in original order */
	int i;
	int j;

	/*** functions ***/
	public:

	/* constructors */
	edge_t();
	edge_t(int ii, int jj);
	~edge_t();

	/* init:
	 *
	 * 	Will reinitialize this object
	 * 	with the given values.
	 */
	void init(int ii, int jj);
	
	/* get:
	 *
	 * 	Get's the specified element of this triple.
	 */
	inline int get(int ind) const
	{
		switch(ind)
		{
			case 0:
				return i;
			case 1:
				return j;
			default:
				return -1;
		}
	};

	/* flip:
	 *
	 * 	Returns the reverse of this edge.
	 */
	inline edge_t flip() const
	{
		edge_t e;

		/* flip this edge */
		e.i = this->j;
		e.j = this->i;

		/* return it */
		return e;
	};

	/* operators */

	inline bool operator < (const edge_t& rhs) const
	{
		if(this->i < rhs.i)
			return true;
		if(this->i > rhs.i)
			return false;
		return (this->j < rhs.j);
	};
	inline bool operator == (const edge_t& rhs) const
	{
		return (this->i == rhs.i && this->j == rhs.j);
	};
	inline bool operator != (const edge_t& rhs) const
	{
		return (this->i != rhs.i || this->j != rhs.j);
	};
	inline edge_t operator = (const edge_t& rhs)
	{
		this->i = rhs.i;
		this->j = rhs.j;
		return (*this);
	};
};

#endif
