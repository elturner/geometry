#include "triple.h"
#include <stdlib.h>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

/********* TRIPLE_T FUNCTIONS ***********/

triple_t::triple_t()
{
	this->i = this->j = this->k = this->a = this->b = this->c = 0;
}

triple_t::triple_t(int ii, int jj, int kk)
{
	this->init(ii, jj, kk);
}

triple_t::~triple_t()
{
	/* no work required */
}

void triple_t::init(int ii, int jj, int kk)
{
	/* store original ordering */
	this->i = ii;
	this->j = jj;
	this->k = kk;

	/* sort these three numbers */
	if(ii < jj)
	{
		if(ii < kk)
		{
			this->a = ii;
			if(jj < kk)
			{
				this->b = jj;
				this->c = kk;
			}
			else
			{
				this->b = kk;
				this->c = jj;
			}
		}
		else
		{
			this->a = kk;
			this->b = ii;
			this->c = jj;
		}
	}
	else
	{
		if(jj < kk)
		{
			this->a = jj;
			if(ii < kk)
			{
				this->b = ii;
				this->c = kk;
			}
			else
			{
				this->b = kk;
				this->c = ii;
			}
		}
		else
		{
			this->a = kk;
			this->b = jj;
			this->c = ii;
		}
	}
}

bool triple_t::neighbors_with(const triple_t& other) const
{
	vector<int> my_list;
	vector<int> other_list;
	vector<int> intersect;
	vector<int>::iterator it;

	/* store elements in lists */
	my_list.push_back(this->a);
	my_list.push_back(this->b);
	my_list.push_back(this->c);

	other_list.push_back(other.a);
	other_list.push_back(other.b);
	other_list.push_back(other.c);

	/* get intersection */
	intersect.resize(3);
	it = set_intersection(my_list.begin(), my_list.end(),
				other_list.begin(), other_list.end(),
				intersect.begin());

	/* check count */
	return (2 <= (it - intersect.begin()));
}
	
bool triple_t::contains(int x) const
{
	return (this->i == x || this->j == x || this->k == x);
}

void triple_t::get_edges(set<edge_t>& loc) const
{
	edge_t e;

	/* add three edges to this set */
	e.init(this->i, this->j);
	loc.insert(e);
	e.init(this->j, this->k);
	loc.insert(e);
	e.init(this->k, this->i);
	loc.insert(e);
}

/************ EDGE_T FUNCTIONS **************/

edge_t::edge_t()
{
	this->i = this->j = 0;
}

edge_t::edge_t(int ii, int jj)
{
	init(ii, jj);
}

edge_t::~edge_t() {}

void edge_t::init(int ii, int jj)
{
	this->i = ii;
	this->j = jj;
}
