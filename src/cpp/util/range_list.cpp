#include "range_list.h"
#include <string>
#include <set>
#include <sstream>
#include <cmath>
#include <stdio.h>

/**
 * @file range_list.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the range list class, which represents
 * a subset of the real line with a sequence of disjoint closed 
 * intervals.
 */

using namespace std;

/*** range_list_t class function defintions ***/

range_list_t::range_list_t()
{
	/* clear list to denote empty set */
	this->list.clear();
}

range_list_t::~range_list_t()
{
	/* clear list to remove values */
	this->list.clear();
}

void range_list_t::clear()
{	
	/* clear list to remove values */
	this->list.clear();
}

int range_list_t::parse(std::string& s)
{
	stringstream ss;
	string r;
	range_t range;

	/* populate stream with string */
	ss.str(s);

	/* iterate through elements of stream */
	while(!(ss.eof()))
	{
		/* get the next range string */
		std::getline(ss, r, ';');

		/* parse as range */
		range = range_t(r);

		/* check if valid */
		if(range.length() < 0)
			return -1;

		/* add to list */
		this->add(range);
	}

	/* success */
	return 0;
}

void range_list_t::add(const range_t& r)
{
	pair<set<range_t>::iterator, bool> ret;
	range_t range;

	/* check the argument */
	range = r;
	if(range.length() < 0)
		return; /* do nothing */

	/* add to list while checking for
	 * any intersecting intervals that
	 * are already present */
	while(true)
	{
		/* attempt to add to set */
		ret = this->list.insert(range);
		if(ret.second)
			break; /* we're done */

		/* if got to here, means this range
		 * intersects with another in the set,
		 * so remove that range, merge, and repeat */
		range += *(ret.first);
		this->list.erase(ret.first);
	}
}
		
void range_list_t::add_int(int i)
{
	range_t r;

	/* create the range */
	r.min = i - 0.51;
	r.max = i + 0.51;

	/* add to list */
	this->add(r);
}

void range_list_t::add(double a, double b)
{
	range_t range(a, b);

	/* add this range to the list */
	this->add(range);
}

bool range_list_t::contains(double v) const
{
	range_t r(v,v);

	/* check if r clashes with any value in list */
	return (this->list.find(r) != this->list.end());
}
		
bool range_list_t::intersects(const range_t& r) const
{
	/* if this range intersects any existing ones, then
	 * it will compare equal to those ranges in the set
	 * and count will return a positive value */
	return (this->list.count(r) > 0);
}
		
void range_list_t::get_ints(vector<int>& is) const
{
	set<range_t>::iterator it;
	int i;

	/* first, clear the input list */
	is.clear();

	/* iterate through the ranges in this list */
	for(it = this->list.begin(); it != this->list.end(); it++)
		/* find the integers within this range */
		for(i = ((int) ceil(it->min)); i < it->max; i++)
			is.push_back(i); /* i is within range, add it */
}

void range_list_t::get_ranges(vector<pair<double, double> >& rs) const
{
	set<range_t>::const_iterator it;

	/* clear the vector */
	rs.clear();

	/* add the ranges */
	for(it = this->list.begin(); it != this->list.end(); it++)
		rs.push_back(pair<double,double>(it->min, it->max));
}

/*** range_t class function definitions ***/

range_t::range_t()
{
	this->min = 0;
	this->max = 1;
}

range_t::range_t(double a, double b)
{
	if(a > b)
	{
		this->min = b;
		this->max = a;
	}
	else
	{
		this->min = a;
		this->max = b;
	}
}

range_t::range_t(std::string& s)
{
	int ret;

	/* parse the string */
	ret = sscanf(s.c_str(), "[%lf,%lf]",
		         &(this->min), &(this->max));
	if(ret != 2)
	{
		/* unable to parse range */

		/* try to parse as singleton value */
		ret = sscanf(s.c_str(), "%lf", &(this->min));
		if(ret != 1)
		{
			/* unable to parse, set as invalid interval */
			this->min = 1;
			this->max = 0;
		}

		/* set max to same as min */
		this->max = this->min;
	}
}

range_t::~range_t()
{
	/* do nothing here */
}

double range_t::length() const
{
	return (this->max - this->min);
}

bool range_t::contains(double v) const
{
	/* check if this range contains v */
	return (v >= this->min && v <= this->max);
}

range_t range_t::operator = (const range_t& other)
{
	/* set this range to the values of the other range */
	this->min = other.min;
	this->max = other.max;

	/* return the newly changed range */
	return (*this);
}

bool range_t::operator == (const range_t& other) const
{
	/* check if these two ranges overlap */
	return (this->min <= other.min && other.min <= this->max)
	       || (other.min <= this->min && this->min <= other.max);
}

bool range_t::operator != (const range_t& other) const
{
	return !((*this) == other);
}

bool range_t::operator < (const range_t& other) const
{
	return (this->max < other.min);
}

bool range_t::operator > (const range_t& other) const
{
	return (this->min > other.max);
}

bool range_t::operator <= (const range_t& other) const
{
	return ((*this) < other) || ((*this) == other);
}

bool range_t::operator >= (const range_t& other) const
{
	return ((*this) > other) || ((*this) == other);
}

range_t range_t::operator + (const range_t& other) const
{
	/* create convex hull */
	range_t s( 
	           (this->min < other.min) ? this->min : other.min,
	           (this->max > other.max) ? this->max : other.max 
	         );

	/* return generated range */
	return s;
}

range_t range_t::operator += (const range_t& other)
{
	/* set this range to the sum */
	(*this) = (*this) + other;

	/* return this range */
	return (*this);
}
